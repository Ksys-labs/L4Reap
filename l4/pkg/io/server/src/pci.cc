/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/types.h>
#include <l4/cxx/string>
#include <l4/cxx/minmax>
#include <l4/io/pciids.h>

#include <cstdio>
#include <typeinfo>

#include "main.h"
#include "pci.h"
#include "phys_space.h"
#include "cfg.h"

static Pci_root_bridge *__pci_root_bridge;
static unsigned _last_msi;

class Pci_cardbus_bridge : public Pci_pci_bridge_basic
{
public:
  Pci_cardbus_bridge(Hw::Device *host, Pci_bridge *bus)
  : Pci_pci_bridge_basic(host, bus)
  {}

  void discover_resources(Hw::Device *host);
};

Pci_root_bridge *pci_root_bridge(int segment)
{
  if (segment != 0)
    return 0;
  return __pci_root_bridge;
}

#if defined(ARCH_x86) || defined(ARCH_amd64)

#include <l4/util/port_io.h>

int
pci_register_root_bridge(int segment, Pci_root_bridge *b)
{
  if (segment != 0)
    return -1;

  __pci_root_bridge = b;
  return 0;
}

inline
l4_uint32_t
pci_conf_addr(l4_uint32_t bus, l4_uint32_t dev, l4_uint32_t fn, l4_uint32_t reg)
{ return 0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3); }

int
Pci_port_root_bridge::cfg_read(unsigned bus, l4_uint32_t devfn,
                              l4_uint32_t reg,
                              l4_uint32_t *value, Cfg_width order)
{
  l4util_out32(pci_conf_addr(bus, devfn >> 16, devfn & 0xffff, reg), 0xcf8);
  using namespace Hw::Pci;

  switch (order)
    {
    case Cfg_byte:  *value = l4util_in8(0xcfc + (reg & 3)); break;
    case Cfg_short: *value = l4util_in16((0xcfc + (reg & 3)) & ~1UL); break;
    case Cfg_long:  *value = l4util_in32(0xcfc); break;
    }
  return 0;
}

int
Pci_port_root_bridge::cfg_write(unsigned bus, l4_uint32_t devfn,
                               l4_uint32_t reg,
                               l4_uint32_t value, Cfg_width order)
{
  l4util_out32(pci_conf_addr(bus, devfn >> 16, devfn & 0xffff, reg), 0xcf8);
  using namespace Hw::Pci;

  switch (order)
    {
    case Cfg_byte:  l4util_out8(value, 0xcfc + (reg & 3)); break;
    case Cfg_short: l4util_out16(value, (0xcfc + (reg & 3)) & ~1UL); break;
    case Cfg_long:  l4util_out32(value, 0xcfc); break;
    }
  return 0;
}

#endif

namespace {
static inline l4_uint32_t
devfn(unsigned dev, unsigned fn)
{ return (dev << 16) | fn; }

}

void
Pci_bridge::scan_bus()
{
  using namespace Hw::Pci;

  int device = 0;
  for (device = 0; device < 32; ++device)
    {
      l4_uint32_t hdr_type;
      cfg_read(num, devfn(device, 0), Pci_dev::C_header_type, &hdr_type, Cfg_byte);
      int funcs = (hdr_type & 0x80) ? 8 : 1;
      for (int function = 0; function < funcs; ++function)
	{
	  l4_uint32_t vendor, _class;
	  cfg_read(num, devfn(device, function), Pci_dev::C_vendor, &vendor, Cfg_short);
	  if (vendor == 0xffff)
            {
              if (function == 0)
                break;
              else
	        continue;
            }

	  cfg_read(num, devfn(device, function), Pci_dev::C_vendor, &vendor, Cfg_long);
	  cfg_read(num, devfn(device, function), Pci_dev::C_class_rev, &_class, Cfg_long);

	  if (function)
	    cfg_read(num, devfn(device, function), Pci_dev::C_header_type, &hdr_type, Cfg_byte);

	  Hw::Device *child = host()->get_child_dev_adr(devfn(device, function), true);

	  Pci_dev *d;
	  if ((_class >> 16) == 0x0604 || (_class >> 16) == 0x0607)
	    {
	      Pci_pci_bridge_basic *b = 0;
	      if ((hdr_type & 0x7f) == 1)
		b = new Pci_pci_bridge(child, this);
	      else if((hdr_type & 0x7f) == 2)
		b = new Pci_cardbus_bridge(child, this);

	      child->set_discover_bus_if(b);

	      l4_uint32_t busses;
	      bool reassign_busses = false;

	      cfg_read(num, devfn(device, function), Pci_dev::C_primary, &busses, Cfg_long);

	      if ((busses & 0xff) != num
	          || ((busses >> 8) & 0xff) <= num)
		reassign_busses = true;

	      if (reassign_busses)
		{
		  unsigned new_so = subordinate + 1;
		  b->pri = num;
		  b->num = new_so;
		  b->subordinate = b->num;

		  busses = (busses & 0xff000000)
		    | ((l4_uint32_t)(b->pri))
		    | ((l4_uint32_t)(b->num) << 8)
		    | ((l4_uint32_t)(b->subordinate) << 16);

		  cfg_write(num, devfn(device, function), Pci_dev::C_primary, busses, Cfg_long);
		  increase_subordinate(new_so);
		}
	      else
		{
		  b->pri = busses & 0xff;
		  b->num = (busses >> 8) & 0xff;
		  b->subordinate = (busses >> 16) & 0xff;
		}

	      d = b;
	    }
	  else
	    d = new Pci_dev(child, this);

	  child->add_feature(d);
	  child->add_resource_discoverer(d);

	  d->vendor_device = vendor;
	  d->cls_rev = _class;
	  d->hdr_type = hdr_type;
	}
    }
}


class Msi
{
public:
  enum Addrs
  {
    Base_hi = 0,
    Base_lo = 0xfee00000,
  };

  enum Mode
  {
    Dest_mode_phys = 0 << 2,
    Dest_mode_log  = 1 << 2,
  };

  enum Redir
  {
    Dest_redir_cpu     = 0 << 3,
    Dest_redir_lowprio = 1 << 3,
  };

  enum Delivery
  {
    Data_del_fixed   = 0 << 8,
    Data_del_lowprio = 1 << 8,
  };

  enum Level
  {
    Data_level_deassert = 0 << 14,
    Data_level_assert   = 1 << 14,
  };

  enum Trigger
  {
    Data_trigger_edge  = 0 << 15,
    Data_trigger_level = 1 << 15,
  };

  static unsigned data(int vector) { return vector & 0xff; }
};

l4_uint32_t const *
Pci_dev::cfg_word(unsigned w) const
{
  switch (w)
    {
    default: return 0;
    case 0: return &vendor_device;
    case 2: return &cls_rev;
    case 11: return &subsys_ids;
    }
}

unsigned
Pci_dev::bus_nr() const
{ return _bus->num; }

unsigned
Pci_dev::disable_decoders()
{
  l4_uint32_t v = 0;
  // disable decoders
  cfg_read(C_command, &v, Hw::Pci::Cfg_byte);
  cfg_write(C_command, v & ~3, Hw::Pci::Cfg_byte);
  return v & 0xff;
}

void
Pci_dev::restore_decoders(unsigned cmd)
{
  cfg_write(C_command, cmd, Hw::Pci::Cfg_byte);
}

int
Pci_dev::discover_bar(int bar)
{
  using namespace Hw::Pci;
  l4_uint32_t v, x;

  _bars[bar] = 0;
  int r = C_bar_0 + bar * 4;
  l4_uint8_t cmd = disable_decoders();

  cfg_read(r, &v, Cfg_long);
  cfg_write(r, ~0U, Cfg_long);
  cfg_read(r, &x, Cfg_long);
  cfg_write(r, v, Cfg_long);

  restore_decoders(cmd);

  if (!x)
    return bar + 1;

  unsigned io_flags = (cmd & CC_io) ? 0 : Resource::F_disabled;
  unsigned mem_flags = (cmd & CC_mem) ? 0 : Resource::F_disabled;

  io_flags |= Resource::Io_res
           | Resource::F_size_aligned
           | Resource::F_hierarchical;

  mem_flags |= Resource::Mmio_res
            | Resource::F_size_aligned
            | Resource::F_hierarchical;

  Adr_resource *res = 0;
  if (!(x & 1))
    {
      //printf("%08x: BAR[%d] mmio ... %x\n", adr(), bar, x );
      res = new Adr_resource(mem_flags);
      if ((x & 0x6) == 0x4)
	res->add_flags(Adr_resource::F_width_64bit);

      if (x & 0x8)
	res->add_flags(Resource::F_prefetchable);

      int order;
      l4_uint64_t size = x & ~0x7f;
      l4_uint64_t a = v & ~0x7f;
      if (res->is_64bit())
	{
	  ++bar;
	  r = 0x10 + bar * 4;
	  cmd = disable_decoders();
	  cfg_read(r, &v, Cfg_long);
	  cfg_write(r, ~0U, Cfg_long);
	  cfg_read(r, &x, Cfg_long);
	  cfg_write(r, v, Cfg_long);
	  restore_decoders(cmd);
	  a |= l4_uint64_t(v) << 32;
	  size |= l4_uint64_t(x) << 32;
	}

      for (int s = 7; s < 64; ++s)
	if ((size >> s) & 1)
	  {
	    order = s;
	    size = 1 << s;
	    break;
	  }

      res->start_size(a, size);

      // printf("%08x: BAR[%d] mem ...\n", adr(), bar*4 + 10 );
      _bars[bar - res->is_64bit()] = res;
      if (res->is_64bit())
	_bars[bar] = (Adr_resource*)1;

    }
  else
    {
      // printf("%08x: BAR[%d] io ...\n", adr(), bar );
      int s;
      for (s = 2; s < 32; ++s)
	if ((x >> s) & 1)
	  break;

      res = new Adr_resource(io_flags);

      _bars[bar] = res;
      res->start_size(v & ~3, 1 << s);
    }

  res->validate();
  _host->add_resource(res);
  return bar + 1;
}

void
Pci_dev::discover_legacy_ide_resources()
{

  if (!Io_config::cfg->legacy_ide_resources(_host))
    return;

  unsigned io_flags = Resource::Io_res
           | Resource::F_size_aligned
           | Resource::F_hierarchical;

  // IDE legacy IO interface
  if (cls_rev >> 16 == 0x101 && !(cls_rev & 0x100))
    {
      _host->add_resource(new Adr_resource(io_flags, 0x1f0, 0x1f7));
      _host->add_resource(new Adr_resource(io_flags, 0x3f6, 0x3f6));
      _host->add_resource(new Adr_resource(Resource::Irq_res | Resource::Irq_edge, 14, 14));
    }
  if (cls_rev >> 16 == 0x101 && !(cls_rev & 0x400))
    {
      _host->add_resource(new Adr_resource(io_flags, 0x170, 0x177));
      _host->add_resource(new Adr_resource(io_flags, 0x376, 0x376));
      _host->add_resource(new Adr_resource(Resource::Irq_res | Resource::Irq_edge, 15, 15));
    }
}

void
Pci_dev::quirk_8086_8108()
{
  using namespace Hw::Pci;

  if (!(vendor() == 0x8086 && device() == 0x8108))
    return;

  l4_uint32_t v;

  // GC - Graphics Control
  cfg_read(0x52, &v, Cfg_short);

  unsigned gfx_mem_sz = 0;

  // VGA disabled?
  if (v & 2)
    return;

  switch ((v >> 4) & 7)
    {
    case 1: gfx_mem_sz = 1 << 20; break;
    case 2: gfx_mem_sz = 4 << 20; break;
    case 3: gfx_mem_sz = 8 << 20; break;
    default: return;
    }

  // BSM - Base of Stolen Memory
  cfg_read(0x5c, &v, Cfg_long);
  v &= 0xfff00000;

  unsigned flags = Resource::Mmio_res
                   | Resource::F_prefetchable
                   | Resource::F_fixed_addr
                   | Resource::F_fixed_size;
  l4_addr_t end = v + gfx_mem_sz - 1;

  _host->add_resource(new Adr_resource(flags, v, end));
  for (Pci_bridge *p = _bus; p;)
    {
      p->host()->add_resource(new Adr_resource_provider(flags, v, end));
      if (Pci_dev *d = dynamic_cast<Pci_dev *>(p))
        p = d->_bus;
      else
        break;
    }
}

void
Pci_dev::discover_expansion_rom()
{
  using namespace Hw::Pci;

  if (!Io_config::cfg->expansion_rom(host()))
    return;

  l4_uint32_t v, x;
  unsigned rom_register = ((hdr_type & 0x7f) == 0) ? 12 * 4 : 14 * 4;

  cfg_read(rom_register, &v, Cfg_long);

  if (v == 0xffffffff || v == 0)
    return; // no expansion ROM

  cfg_write(rom_register, 0xfffffffe, Cfg_long);
  cfg_read(rom_register, &x, Cfg_long);
  cfg_write(rom_register, v, Cfg_long);

  v &= ~0x3ff;
  x &= ~0x3ff;

  //printf("ROM %08x: %08x %08x\n", adr(), x, v);

  int s;
  for (s = 2; s < 32; ++s)
    if ((x >> s) & 1)
      break;

  if (0) // currently we do not add a resource record for ROMs
    {
      unsigned flags = Adr_resource::Mmio_res | Adr_resource::F_size_aligned
                       | Adr_resource::F_rom | Adr_resource::F_prefetchable;
      Adr_resource *res = new Adr_resource(flags);

      _rom = res;
      res->start_size(v & ~3, 1 << s);
      res->validate();
      _host->add_resource(res);
    }
}

void
Pci_dev::discover_pci_caps()
{
  using namespace Hw::Pci;

    {
      l4_uint32_t status;
      cfg_read(C_status, &status, Cfg_short);
      if (!(status & CS_cap_list))
	return;
    }

  l4_uint32_t cap_ptr;
  cfg_read(C_capability_ptr, &cap_ptr, Cfg_byte);
  cap_ptr &= ~0x3;
  for (; cap_ptr; cfg_read(cap_ptr + 1, &cap_ptr, Cfg_byte), cap_ptr &= ~0x3)
    {
      l4_uint32_t id;
      cfg_read(cap_ptr, &id, Cfg_byte);
      //printf("  PCI-cap: %x\n", id);
      if (id == 0x05 && Io_config::cfg->transparent_msi(host())
	  && system_icu()->info.supports_msi())
	{
	  // MSI capability
	  l4_uint32_t t, ctl, data;
	  l4_uint64_t addr = 0;

	  int msg_offs = 8;
	  cfg_read(cap_ptr + 2, &ctl, Cfg_short);
	  cfg_read(cap_ptr + 4, &t, Cfg_long);
	  addr |= t;
	  if (ctl & (1 << 7))
	    {
	      cfg_read(cap_ptr + 8, &t, Cfg_long);
	      addr |= ((l4_uint64_t)t) << 32;
	      msg_offs = 12;
	    }

	  cfg_read(cap_ptr + msg_offs, &data, Cfg_short);

	  cfg_write(cap_ptr + 4, Msi::Base_lo | Msi::Dest_mode_phys | Msi::Dest_redir_cpu, Cfg_long);

	  if (ctl & (1 << 7))
	    cfg_write(cap_ptr + 8, Msi::Base_hi, Cfg_long);

	  unsigned msi = _last_msi++;
	  if (msi >= system_icu()->info.nr_msis)
	    {
	      printf("WARNING: run out of MSI vectors, use normal IRQ\n");
	      continue;
	    }

	  l4_umword_t msg = 0;
	  if (l4_error(system_icu()->icu->msi_info(msi, &msg)) < 0)
	    {
	      printf("WARNING: could not get MSI message, use normal IRQ\n");
	      continue;
	    }

	  cfg_write(cap_ptr + msg_offs, Msi::Data_level_assert | Msi::Data_trigger_edge | Msi::Data_del_fixed | Msi::data(msg), Cfg_short);

	  cfg_write(cap_ptr + 2, ctl | 1, Cfg_short);

	  printf("  MSI cap: %x %llx %x\n", ctl, addr, data);
	  msi |= 0x80;
	  Adr_resource *res = new Adr_resource(Resource::Irq_res | Resource::Irq_edge, msi, msi);
	  flags |= F_msi;
	  _host->add_resource(res);
	}
    }
}

void
Pci_dev::discover_resources(Hw::Device *host)
{
  using namespace Hw::Pci;

  // printf("survey ... %x.%x\n", dynamic_cast<Pci_bridge*>(parent())->num, adr());
  l4_uint32_t v;
  cfg_read(C_subsys_vendor, &v, Cfg_long);
  subsys_ids = v;
  cfg_read(C_irq_pin, &v, Cfg_byte);
  irq_pin = v;

  if (irq_pin)
    host->add_resource(new Adr_resource(Resource::Irq_res | Resource::F_relative
                                  | Resource::F_hierarchical,
                                  irq_pin - 1, irq_pin - 1));

  cfg_read(C_command, &v, Cfg_short);

  int bars = ((hdr_type & 0x7f) == 0) ? 6 : 2;

  discover_legacy_ide_resources();

  quirk_8086_8108();

  for (int bar = 0; bar < bars;)
    bar = discover_bar(bar);

  discover_expansion_rom();
  discover_pci_caps();
}

void
Pci_dev::setup_resources(Hw::Device *)
{
  using namespace Hw::Pci;

  for (unsigned i = 0; i < sizeof(_bars)/sizeof(_bars[0]); ++i)
    {
      Adr_resource *r = bar(i);
      if (!r || r->type() == Resource::Io_res)
	continue;

      if (r->empty())
	continue;

      int reg = 0x10 + i * 4;
      l4_uint64_t s = r->start();
      l4_uint8_t cmd = disable_decoders();
      cfg_write(reg, s, Cfg_long);
      if (r->is_64bit())
	{
	  cfg_write(reg + 4, s >> 32, Cfg_long);
	  ++i;
	}
      restore_decoders(cmd);


      l4_uint32_t v;
      cfg_read(reg, &v, Cfg_long);
      if (l4_uint32_t(v & ~0x7f) != l4_uint32_t(s & 0xffffffff))
	printf("ERROR: could not set PCI BAR %d\n", i);

      // printf("%08x: set BAR[%d] to %08x\n", adr(), i, v);
    }
}


bool
Pci_dev::match_cid(cxx::String const &_cid) const
{
  cxx::String const prefix("PCI/");
  cxx::String cid(_cid);
  if (!cid.starts_with(prefix))
    return false;

  cid = cid.substr(prefix.len());
  cxx::String::Index n;
  for (; cid.len() > 0; cid = cid.substr(n + 1))
    {
      n = cid.find("&");
      cxx::String tok = cid.head(n);
      if (tok.empty())
	continue;

      if (tok.starts_with("CC_"))
	{
	  tok = tok.substr(3);
	  if (tok.len() < 2)
	    return false;

	  l4_uint32_t _csr;
	  int l = tok.from_hex(&_csr);
	  if (l < 0 || l > 6 || l % 2)
	    return false;

	  if ((cls_rev >> (8 + (6-l) * 4)) == _csr)
	    continue;
	  else
	    return false;
	}
      else if (tok.starts_with("REV_"))
	{
	  tok = tok.substr(4);
	  unsigned char r;
	  if (tok.len() != 2 || tok.from_hex(&r) != 2)
	    return false;

	  if (r != (cls_rev & 0xff))
	    return false;
	}
      else if (tok.starts_with("VEN_"))
	{
	  tok = tok.substr(4);
	  l4_uint32_t v;
	  if (tok.len() != 4 || tok.from_hex(&v) != 4)
	    return false;

	  if (((vendor_device >> 16) & 0xffff) != v)
	    return false;
	}
      else if (tok.starts_with("DEV_"))
	{
	  tok = tok.substr(4);
	  l4_uint32_t d;
	  if (tok.len() != 4 || tok.from_hex(&d) != 4)
	    return false;

	  if ((vendor_device & 0xffff) != d)
	    return false;
	}
      else if (tok.starts_with("SUBSYS_"))
	{
	  l4_uint32_t s;
	  tok = tok.substr(7);
	  if (tok.len() != 8 || tok.from_hex(&s) != 8)
	    return false;
	  if (subsys_ids != s)
	    return false;
	}
      else
	return false;
    }

  return true;
}

void
Pci_pci_bridge::setup_resources(Hw::Device *host)
{
  using namespace Hw::Pci;

  Pci_dev::setup_resources(host);

  if (!mmio->empty() && mmio->valid())
    {
      l4_uint32_t v = (mmio->start() >> 16) & 0xfff0;
      v |= mmio->end() & 0xfff00000;
      cfg_write(0x20, v, Cfg_long);
      // printf("%08x: set mmio to %08x\n", adr(), v);
      l4_uint32_t r;
      cfg_read(0x20, &r, Cfg_long);
      // printf("%08x: mmio =      %08x\n", adr(), r);
      cfg_read(0x04, &r, Cfg_short);
      r |= 3;
      cfg_write(0x4, r, Cfg_short);
    }

  if (!pref_mmio->empty() && pref_mmio->valid())
    {
      l4_uint32_t v = (pref_mmio->start() >> 16) & 0xfff0;
      v |= pref_mmio->end() & 0xfff00000;
      cfg_write(0x24, v, Cfg_long);
      // printf("%08x: set pref mmio to %08x\n", adr(), v);
    }
}

void
Pci_pci_bridge::discover_resources(Hw::Device *host)
{
  using namespace Hw::Pci;

  l4_uint32_t v;
  l4_uint64_t s, e;

  cfg_read(C_mem_base, &v, Cfg_long);
  s = (v & 0xfff0) << 16;
  e = (v & 0xfff00000) | 0xfffff;

  Adr_resource *r = new Adr_resource_provider(Resource::Mmio_res);
  r->alignment(0xfffff);
  if (s < e)
    r->start_end(s, e);
  else
    r->set_empty();

  // printf("%08x: mmio = %08x\n", adr(), v);
  mmio = r;
  mmio->validate();
  _host->add_resource(mmio);

  r = new Adr_resource_provider(Resource::Mmio_res | Resource::F_prefetchable);
  cfg_read(C_pref_mem_base, &v, Cfg_long);
  s = (v & 0xfff0) << 16;
  e = (v & 0xfff00000) | 0xfffff;

  if ((v & 0x0f) == 1)
    {
      r->add_flags(Adr_resource::F_width_64bit);
      cfg_read(C_pref_mem_base_hi, &v, Cfg_long);
      s |= l4_uint64_t(v) << 32;
      cfg_read(C_pref_mem_limit_hi, &v, Cfg_long);
      e |= l4_uint64_t(v) << 32;
    }

  r->alignment(0xfffff);
  if (s < e)
    r->start_end(s, e);
  else
    r->set_empty();

  pref_mmio = r;
  r->validate();
  _host->add_resource(r);

  cfg_read(C_io_base, &v, Cfg_short);
  s = (v & 0xf0) << 8;
  e = (v & 0xf000) | 0xfff;

  r = new Adr_resource_provider(Resource::Io_res);
  r->alignment(0xfff);
  if (s < e)
    r->start_end(s, e);
  else
    r->set_empty();
  io = r;
  r->validate();
  _host->add_resource(r);

  Pci_dev::discover_resources(host);
}


void
Pci_root_bridge::discover_resources(Hw::Device *)
{}




static const char * const pci_classes[] =
    { "unknown", "mass storage contoller", "network controller", 
      "display controller", "multimedia device", "memory controller",
      "bridge device", "simple communication controller", 
      "system peripheral", "input device", "docking station", 
      "processor", "serial bus controller", "wireless controller",
      "intelligent I/O controller", "satellite communication controller",
      "encryption/decryption controller", 
      "data aquisition/signal processing controller" };

static char const * const pci_bridges[] =
{ "Host/PCI Bridge", "ISA Bridge", "EISA Bridge", "Micro Channel Bridge",
  "PCI Bridge", "PCMCIA Bridge", "NuBus Bridge", "CardBus Bridge" };


static void
dump_res_rec(Resource_list const *r, int indent)
{
  for (Resource_list::iterator i = r->begin(); i!= r->end(); ++i)
    if (*i)
      {
        i->dump(indent + 2);
        //dump_res_rec(i->child(), indent + 2);
      }
}

void
Pci_dev::dump(int indent) const
{
  char const *classname = "";

  if (cls_rev >> 24 < sizeof(pci_classes)/sizeof(pci_classes[0]))
    classname = pci_classes[cls_rev >> 24];

  if ((cls_rev >> 24) == 0x06)
    {
      unsigned sc = (cls_rev >> 16) & 0xff;
      if (sc < sizeof(pci_bridges)/sizeof(pci_bridges[0]))
	classname = pci_bridges[sc];
    }

  printf("%*.s%04x:%02x:%02x.%x: %s [%d]\n", indent, " ",
         0, (int)_bus->num, _host->adr() >> 16, _host->adr() & 0xffff,
         classname, (int)hdr_type);

  char buf[130];
  printf("%*.s              0x%04x 0x%04x\n", indent, " ", vendor(), device());
  libpciids_name_device(buf, sizeof(buf), vendor(), device());
  printf("%*.s              %s\n", indent, " ", buf);
#if 0
  if (verbose_lvl)
    dump_res_rec(resources(), 0);
#endif
}

void
Pci_bridge::dump(int) const
{
#if 0
  "bridge %04x:%02x:%02x.%x\n",
         b->num, 0, b->parent() ? (int)static_cast<Pci_bridge*>(b->parent())->num : 0,
	 b->adr() >> 16, b->adr() & 0xffff);
#endif
#if 0
  //dump_res_rec(resources(), 0);

  for (iterator c = begin(0); c != end(); ++c)
    c->dump();
#endif
};



void
Pci_cardbus_bridge::discover_resources(Hw::Device *host)
{
  using namespace Hw::Pci;
  l4_uint32_t v;
  cfg_read(C_subsys_vendor, &v, Cfg_long);
  subsys_ids = v;

  Adr_resource *r = new Adr_resource_provider(Resource::Mmio_res);
  cfg_read(C_cb_mem_base_0, &v, Cfg_long);
  r->start(v);
  cfg_read(C_cb_mem_limit_0, &v, Cfg_long);
  r->end(v);
  if (!r->end())
    r->set_empty();
  r->validate();
  host->add_resource(r);

  r = new Adr_resource_provider(Resource::Mmio_res);
  cfg_read(C_cb_mem_base_1, &v, Cfg_long);
  r->start(v);
  cfg_read(C_cb_mem_limit_1, &v, Cfg_long);
  r->end(v);
  if (!r->end())
    r->set_empty();
  r->validate();
  host->add_resource(r);

  r = new Adr_resource_provider(Resource::Io_res);
  cfg_read(C_cb_io_base_0, &v, Cfg_long);
  r->start(v);
  cfg_read(C_cb_io_limit_0, &v, Cfg_long);
  r->end(v);
  if (!r->end())
    r->set_empty();
  r->validate();
  host->add_resource(r);

  r = new Adr_resource_provider(Resource::Io_res);
  cfg_read(C_cb_io_base_1, &v, Cfg_long);
  r->start(v);
  cfg_read(C_cb_io_limit_1, &v, Cfg_long);
  r->end(v);
  if (!r->end())
    r->set_empty();
  r->validate();
  host->add_resource(r);
}

void
Pci_irq_router::dump(int indent) const
{
  printf("%*sPCI IRQ ROUTER: %s (%p)\n", indent, "", typeid(*this).name(),
         this);
}

bool
Pci_pci_bridge_irq_router_rs::request(Resource *parent, Device *pdev,
                                      Resource *child, Device *cdev)
{
  Adr_resource *cr = dynamic_cast<Adr_resource*>(child);

  if (!cr)
    {
      // assume we allocate an abstract IRQ router resource as a child
      // that always fits
      child->parent(parent);
      return true;
    }

  bool res = false;

  Hw::Device *cd = dynamic_cast<Hw::Device*>(cdev);

  if (!cd)
    return res;

  if (pdev->parent())
    {
      cr->start((cr->start() + (cd->adr() >> 16)) & 3);
      res = pdev->parent()->request_child_resource(cr, pdev);
      if (res)
	cr->parent(parent);
    }

  return res;
}

