/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/vbus/vdevice-ops.h>

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <l4/cxx/exceptions>
#include <l4/io/pciids.h>
#include <l4/sys/err.h>

#include "debug.h"
#include "pci.h"
#include "vpci.h"
#include "vbus_factory.h"

namespace Vi {

// -----------------------------------------------------------------------
// Pci_virtual_dev
// -----------------------------------------------------------------------

Pci_virtual_dev::Pci_virtual_dev()
{
  memset(&_h, 0, sizeof(_h));
}

int
Pci_virtual_dev::cfg_read(int reg, l4_uint32_t *v, Cfg_width order)
{
  reg >>= order;
  if ((unsigned)reg >= (_h_len >> order))
    return -L4_ERANGE;

#define RC(x) *v = *((Hw::Pci::Cfg_type<x>::Type const *)_h + reg); break
  *v = 0;
  switch (order)
    {
    case Hw::Pci::Cfg_byte: RC(Hw::Pci::Cfg_byte);
    case Hw::Pci::Cfg_short: RC(Hw::Pci::Cfg_short);
    case Hw::Pci::Cfg_long: RC(Hw::Pci::Cfg_long);
    }

  return 0;
#undef RC
}

int
Pci_virtual_dev::cfg_write(int reg, l4_uint32_t v, Cfg_width order)
{
  switch (reg & ~3)
    {
    case 0x4: // status is RO
      v &= 0x0000ffff << (reg & (3 >> order));
      break;

    default:
      break;
    }

  reg >>= order;
  if ((unsigned)reg >= (_h_len >> order))
    return -L4_ERANGE;

#define RC(x) *((Hw::Pci::Cfg_type<x>::Type *)_h + reg) = v; break
  switch (order)
    {
    case Hw::Pci::Cfg_byte: RC(Hw::Pci::Cfg_byte);
    case Hw::Pci::Cfg_short: RC(Hw::Pci::Cfg_short);
    case Hw::Pci::Cfg_long: RC(Hw::Pci::Cfg_long);
    }

  return 0;
#undef RC
}



// -----------------------------------------------------------------------
// Pci_proxy_dev
// -----------------------------------------------------------------------

Pci_proxy_dev::Pci_proxy_dev(Hw::Pci::If *hwf)
: _hwf(hwf), _rom(0)
{
  assert (hwf);
  for (int i = 0; i < 6; ++i)
    {
      Adr_resource *r = _hwf->bar(i);

      if (!r)
	{
	  _vbars[i] = 0;
	  continue;
	}

      if (_hwf->is_64bit_high_bar(i))
	{
	  _vbars[i] = l4_uint64_t(r->start()) >> 32;
	}
      else
	{
	  _vbars[i] = r->start();
	  if (r->type() == Adr_resource::Io_res)
	    _vbars[i] |= 1;

	  if (r->is_64bit())
	    _vbars[i] |= 4;

	  if (r->prefetchable())
	    _vbars[i] |= 8;

	}

      //printf("  bar: %d = %08x\n", i, _vbars[i]);
    }

  if (_hwf->rom())
    _rom = _hwf->rom()->start();
}

int
Pci_proxy_dev::irq_enable(Irq_info *irq)
{
  for (Resource_list::iterator i = host()->resources()->begin();
      i != host()->resources()->end(); ++i)
    {
      Adr_resource *res = dynamic_cast<Adr_resource*>(*i);
      if (!res)
	continue;

      if (res->type() == Adr_resource::Irq_res)
	{
	  irq->irq = res->start();
	  irq->trigger = !(res->flags() & Resource::Irq_info_base);
	  irq->polarity = !!(res->flags() & (Resource::Irq_info_base * 2));
	  d_printf(DBG_DEBUG, "Enable IRQ: %d %x %x\n", irq->irq, irq->trigger, irq->polarity);
	  if (dlevel(DBG_DEBUG2))
	    dump();
	  return 0;
	}
    }
  return -L4_EINVAL;
}



l4_uint32_t
Pci_proxy_dev::read_bar(int bar)
{
  // d_printf(DBG_ALL, "   read bar[%x]: %08x\n", bar, _vbars[bar]);
  return _vbars[bar];
}

void
Pci_proxy_dev::write_bar(int bar, l4_uint32_t v)
{
  Hw::Pci::If *p = _hwf;

  Adr_resource *r = p->bar(bar);
  if (!r)
    return;

  // printf("  write bar[%x]: %llx-%llx...\n", bar, r->abs_start(), r->abs_end());
  l4_uint64_t size_mask = r->alignment();

  if (r->type() == Adr_resource::Io_res)
    size_mask |= 0xffff0000;

  if (p->is_64bit_high_bar(bar))
    size_mask >>= 32;

  _vbars[bar] = (_vbars[bar] & size_mask) | (v & ~size_mask);

  // printf("    bar=%lx\n", _vbars[bar]);
}

void
Pci_proxy_dev::write_rom(l4_uint32_t v)
{
  Hw::Pci::If *p = _hwf;

  // printf("write rom bar %x %p\n", v, _dev->rom());
  Adr_resource *r = p->rom();
  if (!r)
    return;

  l4_uint64_t size_mask = r->alignment();

  _rom = (_rom & size_mask) | (v & (~size_mask | 1));

  p->cfg_write(0x30, (r->start() & ~1U) | (v & 1), Hw::Pci::Cfg_long);
}

int
Pci_proxy_dev::cfg_read(int reg, l4_uint32_t *v, Cfg_width order)
{
  Hw::Pci::If *p = _hwf;

  l4_uint32_t buf;
  l4_uint32_t const *r = &buf;
  reg &= ~0U << order;
  int dw = reg >> 2;
  switch (dw)
    {
    case 0: case 2: case 11:
      r = p->cfg_word(dw); break;
    case 1: return p->cfg_read(reg, v, order);
    case 3: //buf = l4_uint32_t(_dev->hdr_type) << 16; break;
	    p->cfg_read(dw * 4, &buf, Hw::Pci::Cfg_long);
	    buf |= 0x00800000;
	    break;
    case 4: case 5: case 6: case 7: case 8: case 9:
      buf = read_bar(dw-4); break;
    case 12: buf = read_rom(); break;
    default: return p->cfg_read(reg, v, order); //buf = 0; break;
    }

  unsigned mask = ~0U >> (32 - (1U << (order + 3)));
  *v = (*r >> ((reg & 3) *8)) & mask;
  return L4_EOK;
}

int
Pci_proxy_dev::cfg_write(int reg, l4_uint32_t v, Cfg_width order)
{
  Hw::Pci::If *p = _hwf;

  reg &= ~0U << order;
  int dw = reg >> 2;
  switch (dw)
    {
    case 4: case 5: case 6: case 7: case 8: case 9: case 12: break;
    default: return p->cfg_write(reg, v, order);
    }

  l4_uint32_t old;
  if (order < 2)
    cfg_read(reg, &old, Hw::Pci::Cfg_long);

  l4_uint32_t mask = ~0U >> (32 - (1U << (order + 3)));
  l4_uint32_t sh = (reg & 3) * 8;
  old &= ~(mask << sh);
  old |= (v & mask) << sh;

  switch (dw)
    {
    case 4: case 5: case 6: case 7: case 8: case 9:
	  write_bar(dw-4, old); break;
    case 12: write_rom(old); break;
    default: break;
    }
  return L4_EOK;
}

void
Pci_proxy_dev::dump() const
{
  Hw::Pci::If *p = _hwf;

  printf("       %04x:%02x:%02x.%x:\n",
         0, p->bus_nr(), _hwf->host()->adr() >> 16, _hwf->host()->adr() & 0xffff);
#if 0
  char buf[130];
  libpciids_name_device(buf, sizeof(buf), _dev->vendor(), _dev->device());
  printf("              %s\n", buf);
#endif
}


// -----------------------------------------------------------------------
// Virtual PCI dummy device
// -----------------------------------------------------------------------

class Pci_dummy : public Pci_virtual_dev, public Device
{
private:
  unsigned char _cfg_space[4*4];

public:
  int irq_enable(Irq_info *irq)
  {
    irq->irq = -1;
    return -1;
  }

  Pci_dummy()
  {
    add_feature(this);
    _h = &_cfg_space[0];
    _h_len = sizeof(_cfg_space);
    cfg_hdr()->hdr_type = 0x80;
    cfg_hdr()->vendor_device = 0x02000400;
    cfg_hdr()->status = 0;
    cfg_hdr()->class_rev = 0x36440000;
    cfg_hdr()->cmd = 0x0;
  }

  bool match_hw_feature(const Hw::Dev_feature*) const { return false; }
  int dispatch(l4_umword_t, l4_uint32_t, L4::Ipc::Iostream&)
  { return -L4_ENOSYS; }
  void set_host(Device *d) { _host = d; }
  Device *host() const { return _host; }

private:
  Device *_host;
};


// ----------------------------------------------------------------------
// Basic virtual PCI bridge functionality
// ----------------------------------------------------------------------

Pci_bridge::Dev::Dev()
{
  memset(_fns, 0, sizeof(_fns));
}

void
Pci_bridge::Dev::add_fn(Pci_dev *f)
{
  for (unsigned i = 0; i < sizeof(_fns)/sizeof(_fns[0]); ++i)
    {
      if (!_fns[i])
	{
	  _fns[i] = f;
	  return;
	}
    }
}

void
Pci_bridge::Dev::sort_fns()
{
  // disabled sorting because the relation of two functions is questionable
#if 0
  unsigned n;
  for (n = 0; n < sizeof(_fns)/sizeof(_fns[0]) && _fns[n]; ++n)
    ;

  if (n < 2)
    return;

  bool exchg = false;

  do
    {
      exchg = false;
      for (unsigned i = 0; i < n - 1; ++i)
	{
	  if (_fns[i]->dev()->function_nr() > _fns[i+1]->dev()->function_nr())
	    {
	      Pci_dev *t = _fns[i];
	      _fns[i] = _fns[i+1];
	      _fns[i+1] = t;
	      exchg = true;
	    }
	}
      n -= 1;
    }
  while (exchg && n >= 1);
#endif
}

void
Pci_bridge::Bus::add_fn(Pci_dev *pd, int slot)
{
  if (slot >= 0)
    {
      _devs[slot].add_fn(pd);
      _devs[slot].sort_fns();
      return;
    }

  for (unsigned d = 0; d < Devs && !_devs[d].empty(); ++d)
    if (_devs[d].cmp(pd))
      {
	_devs[d].add_fn(pd);
	_devs[d].sort_fns();
	return;
      }

  for (unsigned d = 0; d < Devs; ++d)
    if (_devs[d].empty())
      {
	_devs[d].add_fn(pd);
	return;
      }
}

void
Pci_bridge::add_child(Device *d)
{
  Pci_dev *vp = d->find_feature<Pci_dev>();

  // hm, we do currently not add non PCI devices.
  if (!vp)
    return;

  _bus.add_fn(vp);
  Device::add_child(d);
}


void
Pci_bridge::add_child_fixed(Device *d, Pci_dev *vp, unsigned dn, unsigned fn)
{
  _bus.dev(dn)->fn(fn, vp);
  Device::add_child(d);
}


Pci_bridge *
Pci_bridge::find_bridge(unsigned bus)
{
  // printf("PCI[%p]: look for bridge for bus %x %02x %02x\n", this, bus, _subordinate, _secondary);
  if (bus == _secondary)
    return this;

  if (bus < _secondary || bus > _subordinate)
    return 0;

  for (unsigned d = 0; d < Bus::Devs; ++d)
    for (unsigned f = 0; f < Dev::Fns; ++f)
      {
	Pci_dev *p = _bus.dev(d)->fn(f);
	if (!p)
	  break;

	Pci_bridge *b = dynamic_cast<Pci_bridge*>(p);
	if (b && (b = b->find_bridge(bus)))
	  return b;
      }

  return 0;
}


bool
Pci_bridge::child_dev(unsigned bus, unsigned char dev, unsigned char fn,
                       Pci_dev **rd)
{
  Pci_bridge *b = find_bridge(bus);
  if (!b)
    {
      *rd = 0;
      return true;
    }

  if (dev >= Bus::Devs || fn >= Dev::Fns)
    {
      *rd = 0;
      return true;
    }

  *rd = b->_bus.dev(dev)->fn(fn);
  return true;
}

void
Pci_bridge::setup_bus()
{
  for (unsigned d = 0; d < Bus::Devs; ++d)
    for (unsigned f = 0; f < Dev::Fns; ++f)
      {
	Pci_dev *p = _bus.dev(d)->fn(f);
	if (!p)
	  break;

	Pci_bridge *b = dynamic_cast<Pci_bridge*>(p);
	if (b)
	  {
	    b->_primary = _secondary;
	    if (b->_secondary <= _secondary)
	      {
	        b->_secondary = ++_subordinate;
		b->_subordinate = b->_secondary;
	      }

	    b->setup_bus();

	    if (_subordinate < b->_subordinate)
	      _subordinate = b->_subordinate;
	  }
      }
}

void
Pci_bridge::finalize_setup()
{
  for (unsigned dn = 0; dn < Bus::Devs; ++dn)
    {
      if (!_bus.dev(dn)->empty())
	continue;

      for (unsigned fn = 0; fn < Dev::Fns; ++fn)
	if (_bus.dev(dn)->fn(fn))
	  {
	    Pci_dummy *dummy = new Pci_dummy();
	    _bus.dev(dn)->fn(0, dummy);
	    Device::add_child(dummy);
	    break;
	  }
    }
#if 0
  for (VDevice *c = dynamic_cast<VDevice*>(children()); c; c = c->next())
    c->finalize_setup();
#endif
}

namespace {
  static Feature_factory_t<Pci_proxy_dev, ::Pci_dev> __pci_f_factory1;
  static Dev_factory_t<Pci_dummy> __pci_dummy_factory("PCI_dummy_device");
}

}

