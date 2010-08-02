/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "vpci.h"
#include "vpci_pci_bridge.h"
#include "vbus_factory.h"

namespace Vi {

/**
 * \brief A virtual Host-to-PCI bridge.
 */
class Pci_vroot : public Pci_bridge, public Dev_feature
{
public:

  explicit Pci_vroot();

  int dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios);

  ~Pci_vroot() {}

  char const *hid() const
  { return "PNP0A03"; }

public:
  int cfg_read(L4::Ipc_iostream &ios);
  int cfg_write(L4::Ipc_iostream &ios);
  int irq_enable(L4::Ipc_iostream &ios);
  bool match_hw_feature(const Hw::Dev_feature*) const
  { return false; }
};

// -----------------------------------------------------------------------
// Virtual PCI root bridge with identity mapping of phys devices
// -----------------------------------------------------------------------


class Pci_vroot_id : public Pci_vroot
{
public:
  void add_child(Device *d);

};



// -------------------------------------------------------------------
// Virtual PCI Root bridge
// -------------------------------------------------------------------

Pci_vroot::Pci_vroot() : Pci_bridge()
{
  add_feature(this);
  _subordinate = 100;
}

int
Pci_vroot::cfg_read(L4::Ipc_iostream &ios)
{
  l4_uint32_t bus;
  l4_uint32_t devfn;
  l4_uint32_t reg;
  l4_uint32_t value = 0;
  l4_uint32_t width;

  ios >> bus >> devfn >> reg >> width;
  value = ~0U >> (32 - width);

  //printf("cfg read: %02x:%02x.%1x: reg=%x w=%d\n", bus, devfn >> 16, devfn & 0xffff, reg, width);

  if ((devfn >> 16) >= 32 || (devfn & 0xffff) >= 8)
    {
      ios << value;
      return L4_EOK;
    }

  Pci_dev *d = 0;
  child_dev(bus, (devfn >> 16), (devfn & 0xffff), &d);

  if (!d)
    {
      ios << value;
      return L4_EOK;
    }

  int res = d->cfg_read(reg, &value, Hw::Pci::cfg_w_to_o(width));

  if (res < 0)
    return res;

  //printf("  value=%x\n", value);
  ios << value;
  return L4_EOK;
}
int
Pci_vroot::irq_enable(L4::Ipc_iostream &ios)
{
  l4_uint32_t bus;
  l4_uint32_t devfn;

  ios >> bus >> devfn;

  // printf("irq enable: %02x:%02x.%1x: pin=%d\n", bus, devfn >> 16, devfn & 0xffff, pin);

  if ((devfn >> 16) >= 32 || (devfn & 0xffff) >= 8)
    {
      ios << (int)-1;
      return L4_EOK;
    }

  Pci_dev *d = 0;
  child_dev(bus, (devfn >> 16), (devfn & 0xffff), &d);
  // printf("dev = %p\n", d);
  if (!d)
    {
      ios << (int)-1;
      return L4_EOK;
    }

  Pci_dev::Irq_info info;
  int res = d->irq_enable(&info);
  if (res < 0)
    {
      ios << (int)-1;
      return res;
    }

  // printf("  irq = %d\n", info.irq);
  ios << info.irq << info.trigger << info.polarity;
  return L4_EOK;
}

int
Pci_vroot::cfg_write(L4::Ipc_iostream &ios)
{
  l4_uint32_t bus;
  l4_uint32_t devfn;
  l4_uint32_t reg;
  l4_uint32_t value;
  l4_uint32_t width;

  ios >> bus >> devfn >> reg >> value >> width;
  // printf("cfg write: %02x:%02x.%1x: reg=%x w=%x v=%08x\n", bus, devfn >> 16, devfn & 0xffff, reg, width, value);

  if ((devfn >> 16) >= 32 || (devfn & 0xffff) >= 8)
    return L4_EOK;

  Pci_dev *d = 0;
  child_dev(bus, (devfn >> 16), (devfn & 0xffff), &d);

  if (!d)
    return L4_EOK;

  return d->cfg_write(reg, value, Hw::Pci::cfg_w_to_o(width));
}

int
Pci_vroot::dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios)
{
  switch (func)
    {
    case 0: return cfg_read(ios);
    case 1: return cfg_write(ios);
    case 2: return irq_enable(ios);
    default: return -L4_ENOSYS;
    }
}


void
Pci_vroot_id::add_child(Device *d)
{
  Pci_dev *vp = d->find_feature<Pci_dev>();

  // hm, also here, drom non PCI devices
  if (!vp)
    return;

  if (Pci_proxy_dev *proxy = dynamic_cast<Pci_proxy_dev*>(vp))
    {
      Pci_pci_bridge_basic const *hw_br
	= dynamic_cast<Pci_pci_bridge_basic const *>(proxy->hwf()->bus());

      l4_uint32_t hwadr = proxy->hwf()->host()->adr();
      // printf("VPCI: add proxy PCI device %p (hwbr=%p)\n", proxy, hw_br);
      unsigned dn = (hwadr >> 16) & (Bus::Devs-1);
      unsigned fn = hwadr & (Dev::Fns-1);

      if (!hw_br)
	{
	  // MUST be a device on the root PCI bus
	  _bus.dev(dn)->fn(fn, vp);
	  Device::add_child(d);
	  return;
	}

      Pci_bridge *sw_br = find_bridge(hw_br->num);
      if (!sw_br)
	{
	  Pci_to_pci_bridge *b = new Pci_to_pci_bridge();
	  sw_br = b;
	  sw_br->primary(hw_br->pri);
	  sw_br->secondary(hw_br->num);
	  sw_br->subordinate(hw_br->subordinate);

	  unsigned dn = (hw_br->host()->adr() >> 16) & (Bus::Devs-1);
	  unsigned fn = hw_br->host()->adr() & (Dev::Fns-1);
	  _bus.dev(dn)->fn(fn, b);
	  Device::add_child(b);
	}

      sw_br->add_child_fixed(d, vp, dn, fn);
      return;
    }
  else
    {
      _bus.add_fn(vp);
      Device::add_child(d);
    }
}



static Dev_factory_t<Pci_vroot> __pci_root_factory("PCI_bus");
static Dev_factory_t<Pci_vroot_id> __pci_root_id_factory("PCI_bus_ident");

}
