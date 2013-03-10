/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */


#include "gpio"
#include "hw_device.h"
#include "vdevice.h"
#include "vbus_factory.h"
#include "vbus.h"
#include "vicu.h"

#include <l4/vbus/vbus_gpio-ops.h>

#include <cerrno>


namespace Vi {
namespace {

class Gpio : public Device, public Dev_feature
{
public:
  int dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc::Iostream &ios);

  explicit Gpio(Hw::Device *d)
    : _hwd(dynamic_cast<Hw::Gpio_chip*>(d)), _mask(0)
  {
    add_feature(this);

    for (unsigned i = 0; i < Max_pins; ++i)
      _irqs[i] = -1;
  }

  int add_filter(cxx::String const &tag, cxx::String const &)
  {
    if (tag != "pins")
      return -ENODEV;
    return -EINVAL;
  }

  int add_filter(cxx::String const &tag, unsigned long long val)
  {
    if (tag != "pins")
      return -ENODEV;
    _mask |= (1UL << val);
    return 0;
  }

  int add_filter(cxx::String const &tag, unsigned long long s, unsigned long long e)
  {
    if (tag != "pins")
      return -ENODEV;
    _mask |= ~(~0UL << (e - s + 1)) << s;
    return 0;
  }

  char const *hid() const { return "GPIO"; }
  void set_host(Device *d) { _host = d; }
  Device *host() const { return _host; }

  bool match_hw_feature(const Hw::Dev_feature*) const
  { return false; }

private:
  enum { Max_pins = 32 };
  Device *_host;
  Hw::Gpio_chip *_hwd;
  l4_uint32_t _mask;

  int _irqs[Max_pins];

  int setup(L4::Ipc::Iostream &ios);
  int config_pad(L4::Ipc::Iostream &ios);
  int config_get(L4::Ipc::Iostream &ios);
  int get(L4::Ipc::Iostream &ios);
  int set(L4::Ipc::Iostream &ios);
  int multi_setup(L4::Ipc::Iostream &ios);
  int multi_config_pad(L4::Ipc::Iostream &ios);
  int multi_get(L4::Ipc::Iostream &ios);
  int multi_set(L4::Ipc::Iostream &ios);
  int to_irq(L4::Ipc::Iostream &ios);

  void check(unsigned pin)
  {
    if (pin > 31)
      throw -ERANGE;

    if (!((1UL << pin) & _mask))
      throw -ERANGE;
  }

  void check_mask(unsigned mask)
  {
    if (mask & ~_mask)
      throw -ERANGE;
  }
};



int
Gpio::setup(L4::Ipc::Iostream &ios)
{
  unsigned pin, mode;
  int value;
  ios >> pin >> mode >> value;
  check(pin);
  _hwd->setup(pin, mode, value);
  return 0;
}

int
Gpio::config_pad(L4::Ipc::Iostream &ios)
{
  unsigned pin, func, value;
  ios >> pin >> func >> value;
  check(pin);
  _hwd->config_pad(pin, _mask, func, value);
  return 0;
}

int
Gpio::config_get(L4::Ipc::Iostream &ios)
{
  unsigned pin, func, value;
  ios >> pin >> func;
  check(pin);
  _hwd->config_get(pin, _mask, func, &value);
  ios << value;
  return 0;
}

int
Gpio::get(L4::Ipc::Iostream &ios)
{
  unsigned pin;
  ios >> pin;
  check(pin);
  return _hwd->get(pin);
}

int
Gpio::set(L4::Ipc::Iostream &ios)
{
  unsigned pin;
  int value;
  ios >> pin >> value;
  check(pin);
  _hwd->set(pin, value);
  return 0;
}

int
Gpio::multi_setup(L4::Ipc::Iostream &ios)
{
  unsigned mask, mode, outvalue;
  ios >> mask >> mode >> outvalue;
  check_mask(mask);
  _hwd->multi_setup(mask, mode, outvalue);
  return 0;
}

int
Gpio::multi_config_pad(L4::Ipc::Iostream &ios)
{
  unsigned mask, func, value;
  ios >> mask >> func >> value;
  check_mask(mask);
  _hwd->multi_config_pad(mask, func, value);
  return 0;
}

int
Gpio::multi_get(L4::Ipc::Iostream &ios)
{
  unsigned data = _hwd->multi_get();
  ios << (unsigned)(data & _mask);
  return 0;
}

int
Gpio::multi_set(L4::Ipc::Iostream &ios)
{
  unsigned mask, data;
  ios >> mask >> data;
  check_mask(mask);
  _hwd->multi_set(mask, data);
  return 0;
}

int
Gpio::to_irq(L4::Ipc::Iostream &ios)
{
  unsigned pin;
  ios >> pin;
  check(pin);

  if (_irqs[pin] == -1)
    {
      // we have to allocate the IRQ...
      // if it fails we mark the IRQ as unavailable (-L4_ENODEV)
      _irqs[pin] = -L4_ENODEV;

      int irqnum = _hwd->get_irq(pin);
      if (irqnum < 0)
        return irqnum;

      if (System_bus *sb = dynamic_cast<System_bus *>(get_root()))
        {
          int virq = sb->sw_icu()->alloc_irq(Sw_icu::S_allow_set_mode,
                                             Sw_icu::real_irq(irqnum));
	  if (virq < 0)
	    return virq;

	  _irqs[pin] = virq;
	  return virq;
	}
      return -L4_ENODEV;
    }
  return _irqs[pin];
}

int
Gpio::dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc::Iostream &ios)
{
  try
    {
      switch (func)
	{
	case L4VBUS_GPIO_OP_SETUP: return setup(ios);
	case L4VBUS_GPIO_OP_CONFIG_PAD: return config_pad(ios);
	case L4VBUS_GPIO_OP_CONFIG_GET: return config_get(ios);
	case L4VBUS_GPIO_OP_GET: return get(ios);
	case L4VBUS_GPIO_OP_SET: return set(ios);
	case L4VBUS_GPIO_OP_MULTI_SETUP: return multi_setup(ios);
	case L4VBUS_GPIO_OP_MULTI_CONFIG_PAD: return multi_config_pad(ios);
	case L4VBUS_GPIO_OP_MULTI_GET: return multi_get(ios);
	case L4VBUS_GPIO_OP_MULTI_SET: return multi_set(ios);
	case L4VBUS_GPIO_OP_TO_IRQ: return to_irq(ios);
	default: return -L4_ENOSYS;
	}
    }
  catch (int err)
    {
      return err;
    }
}


static Dev_factory_t<Gpio, Hw::Gpio_device> __gpio_factory;


}
}
