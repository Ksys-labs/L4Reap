/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "debug.h"
#include "hw_device.h"
#include "pci.h"
#include "main.h"

namespace {

inline
l4_uint32_t
pci_conf_addr0(l4_uint32_t bus, l4_uint32_t dev,
               l4_uint32_t fn, l4_uint32_t reg)
{ return (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3); }

class Pci_iomem_root_bridge : public Pci_root_bridge, public Hw::Device
{
public:
  typedef Hw::Pci::Cfg_width Cfg_width;

  explicit Pci_iomem_root_bridge(unsigned bus_nr = 0)
  : Pci_root_bridge(bus_nr, this), _iobase_virt(0), _iobase_phys(~0UL),
    _dev_start(~0UL), _dev_end(~0UL), _iosize(0)
  {
    set_discover_bus_if(this);
  }

  int cfg_read(Cfg_addr addr, l4_uint32_t *value, Cfg_width);
  int cfg_write(Cfg_addr addr, l4_uint32_t value, Cfg_width);

  void scan_bus();

  void init();

  int set_property(cxx::String const &prop, Prop_val const &val);
  template< typename T>
  int getval(const char *s, cxx::String const &prop,
             Prop_val const &val, T *rval);
  int int_map(int i) const { return _int_map[i]; }

private:
  l4_addr_t _iobase_virt, _iobase_phys, _dev_start, _dev_end;
  int _int_map[4];
  l4_size_t _iosize;
};

// Irq router that maps INTA-D to GSI
class Irq_router_rs : public Resource_space
{
public:
  bool request(Resource *parent, Device *, Resource *child, Device *cdev);
  bool alloc(Resource *, Device *, Resource *, Device *, bool)
  { return false; }
};

void
Pci_iomem_root_bridge::init()
{
  if (_iobase_phys == ~0UL)
    {
      d_printf(DBG_ERR, "ERROR: Pci_root_bridge: 'iobase' not set.\n");
      return;
    }

  if (_iosize == 0U)
    {
      d_printf(DBG_ERR, "ERROR: Pci_root_bridge: 'iosize' not set.\n");
      return;
    }

  if (_dev_start == ~0UL || _dev_end == ~0UL)
    {
      d_printf(DBG_ERR, "ERROR: Pci_root_bridge: 'dev_start' and/or 'dev_end' not set.\n");
      return;
    }

  _iobase_virt = res_map_iomem(_iobase_phys, _iosize);
  if (!_iobase_virt)
    return;

  add_resource(new Resource(Resource::Mmio_res
                            | Resource::F_fixed_size
                            | Resource::F_fixed_addr,
                            _iobase_phys,
                            _iobase_phys + _iosize - 1));

  Resource *r = new Resource_provider(Resource::Mmio_res
                                      | Resource::F_fixed_size
                                      | Resource::F_fixed_addr);
  r->alignment(0xfffff);
  r->start_end(_dev_start, _dev_end);
  add_resource(r);

  r = new Resource_provider(Resource::Io_res
                            | Resource::F_fixed_size
                            | Resource::F_fixed_addr);
  r->start_end(0, 0xffff);
  add_resource(r);

  add_resource(new Pci_irq_router_res<Irq_router_rs>());

  Hw::Device::init();
}

void
Pci_iomem_root_bridge::scan_bus()
{
  if (!_iobase_virt)
    return;
  Pci_root_bridge::scan_bus();
}

int
Pci_iomem_root_bridge::cfg_read(Cfg_addr addr, l4_uint32_t *value, Cfg_width w)
{
  using namespace Hw;

  if (!_iobase_virt)
    return -1;

  l4_uint32_t a = _iobase_virt + addr.to_compat_addr();
  switch (w)
    {
    case Pci::Cfg_byte: *value = *(volatile l4_uint8_t *)a; break;
    case Pci::Cfg_short: *value = *(volatile l4_uint16_t *)a; break;
    case Pci::Cfg_long: *value = *(volatile l4_uint32_t *)a; break;
    }

  d_printf(DBG_ALL, "Pci_iomem_root_bridge::cfg_read(%x, %x, %x, %x, %x, %d)\n",
           addr.bus(), addr.dev(), addr.fn(), addr.reg(), *value, w);

  return 0;
}

int
Pci_iomem_root_bridge::cfg_write(Cfg_addr addr, l4_uint32_t value, Cfg_width w)
{
  using namespace Hw;

  if (!_iobase_virt)
    return -1;

  d_printf(DBG_ALL, "Pci_iomem_root_bridge::cfg_write(%x, %x, %x, %x, %x, %d)\n",
           addr.bus(), addr.dev(), addr.fn(), addr.reg(), value, w);

  l4_uint32_t a = _iobase_virt + addr.to_compat_addr();
  switch (w)
    {
    case Pci::Cfg_byte: *(volatile l4_uint8_t *)a = value; break;
    case Pci::Cfg_short: *(volatile l4_uint16_t *)a = value; break;
    case Pci::Cfg_long: *(volatile l4_uint32_t *)a = value; break;
    }
  return 0;
}

template< typename T>
int
Pci_iomem_root_bridge::getval(const char *s, cxx::String const &prop,
                              Prop_val const &val, T *rval)
{
  if (prop == s)
    {
      if (val.type != Prop_val::Int)
        return -E_inval;

      *rval = val.val.integer;
      return E_ok;
    }
  return -E_no_prop;
}

int
Pci_iomem_root_bridge::set_property(cxx::String const &prop, Prop_val const &val)
{
  int r;

  if ((r = getval("iobase", prop, val, &_iobase_phys)) != -E_no_prop)
    return r;
  else if ((r = getval("iosize", prop, val, &_iosize)) != -E_no_prop)
    return r;
  else if ((r = getval("dev_start", prop, val, &_dev_start)) != -E_no_prop)
    return r;
  else if ((r = getval("dev_end", prop, val, &_dev_end)) != -E_no_prop)
    return r;
  else if ((r = getval("int_a", prop, val, &_int_map[0])) != -E_no_prop)
    return r;
  else if ((r = getval("int_b", prop, val, &_int_map[1])) != -E_no_prop)
    return r;
  else if ((r = getval("int_c", prop, val, &_int_map[2])) != -E_no_prop)
    return r;
  else if ((r = getval("int_d", prop, val, &_int_map[3])) != -E_no_prop)
    return r;

  return Hw::Device::set_property(prop, val);
}

static Hw::Device_factory_t<Pci_iomem_root_bridge>
              __hw_pci_root_bridge_factory("Pci_iomem_root_bridge");

bool Irq_router_rs::request(Resource *parent, Device *pdev,
                            Resource *child, Device *cdev)
{
  Hw::Device *cd = dynamic_cast<Hw::Device*>(cdev);
  if (!cd)
    return false;

  if (child->start() > 3)
    return false;

  int i = (child->start() + (cd->adr() >> 16)) & 3;

  Pci_iomem_root_bridge *pd = dynamic_cast<Pci_iomem_root_bridge *>(pdev);
  if (!pd)
    return false;


  child->del_flags(Resource::F_relative);
  child->start(pd->int_map(i));
  child->del_flags(Resource::Irq_info_base * 3);
  child->add_flags(Resource::Irq_level | Resource::Irq_low);

  child->parent(parent);

  return true;
}
}
