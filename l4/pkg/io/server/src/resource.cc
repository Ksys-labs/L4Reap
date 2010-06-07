/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/mem_alloc>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/cxx/exceptions>
#include <l4/re/error_helper>

#include "device.h"

#include <cstdio>
#include <cassert>

void
Adr_resource::dump(int indent) const
{
  //bool abs = true;
  static char const *ty[] = { "INVALID", "IRQ   ", "IOMEM ", "IOPORT",
                              "BUS   ", "unk" };

  //if (!valid())
  //  return;

  l4_uint64_t s, e;
#if 0
  if (abs)
    {
      s = abs_start();
      e = abs_end();
    }
  else
#endif
    {
      s = _d.start();
      e = _d.end();
    }

  static char const * const irq_trigger[]
    //= { "high", "rising", "low", "falling" };
    = { "rising edge", "level high", "falling edge", "level low" };

  char const *tp = prefetchable() ? "pref" : "non-pref";
  if (type() == Irq_res)
    tp = irq_trigger[(flags() / Irq_info_base) & 3];

  printf("%*.s%s%c [%014llx-%014llx %llx] %s (%dbit) (align=%llx flags=%lx)\n",
         indent, " ",
         ty[type()], provided() ? '*' : ' ',
         s, e, (l4_uint64_t)_d.size(),
         tp,
         is_64bit() ? 64 : 32, (unsigned long long)alignment(), flags());
}



bool
Adr_resource_provider::_RS::request(Resource *parent, Device *,
                                    Resource *cld, Device *)
{
  Adr_resource *_provider = dynamic_cast<Adr_resource*>(parent);
  Adr_resource *child = dynamic_cast<Adr_resource*>(cld);
  if (!child || !_provider)
    return false;

  Addr start = child->start();
  Addr end   = child->end();

  if (end < start)
    return false;

  if (start < _provider->start())
    return false;

  if (end > _provider->end())
    return false;

  Rl::iterator r = _rl.begin();
  while (true)
    {
      if (r == _rl.end() || (*r)->start() > end)
	{
	  // insert before r
	  _rl.insert(r, child);
	  child->parent(_provider);
	  return true;
	}

      if ((*r)->end() >= start)
	return false;

      ++r;
    }
}


bool
Adr_resource_provider::_RS::alloc(Resource *parent, Device *pdev,
                                  Resource *cld, Device *cdev,
                                  bool resize)
{
  Adr_resource *_provider = dynamic_cast<Adr_resource*>(parent);
  Adr_resource *child = dynamic_cast<Adr_resource*>(cld);
  if (!child || !_provider)
    return false;

  Rl::iterator p = _rl.begin();
  Addr start = _provider->start();
  Addr end;
  Size min_align = L4_PAGESIZE - 1;

  if (p != _rl.end() && (*p)->start() == 0)
    {
      start = (*p)->end() + 1;
      ++p;
    }

  while (true)
    {
      if (p != _rl.end())
	end = (*p)->start() - 1;
      else
	end = _provider->end();

      Size align = cxx::max<Size>(child->alignment(), min_align);
      start = (start + align) & ~align; // pad to get alignment

      if (start < end && end - start >= child->size() - 1)
	{
	  child->start(start);
	  break;
	}

      if (p == _rl.end() && !resize)
	return false;

      if (p == _rl.end() && resize)
	{
	  end = start + child->size() - 1;
	  if (end < start)
	    return false; // wrapped around

	  _provider->end(end);
	  child->start(start);
	  break;
	}

      start = (*p)->end() + 1;
      ++p;
    }
  return request(_provider, pdev, child, cdev);
}

void Mmio_data_space::alloc_ram(Size size, unsigned long alloc_flags)
{
  long ma_flags = L4Re::Mem_alloc::Continuous;

  ma_flags |= alloc_flags ? L4Re::Mem_alloc::Super_pages : 0;

  _ds_ram = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if (!_ds_ram.is_valid())
    throw(L4::Out_of_memory(""));

  L4Re::chksys(L4Re::Env::env()->mem_alloc()->alloc(size, _ds_ram.get(),
                                                    ma_flags));

  l4_size_t ds_size = size;
  l4_addr_t phys_start;
  L4Re::chksys(_ds_ram->phys(0, phys_start, ds_size));
  if (size > ds_size)
    throw(L4::Out_of_memory("not really"));

  start(phys_start);

  L4Re::chksys(L4Re::Env::env()->rm()->attach(&_r, ds_size,
                                              L4Re::Rm::Search_addr |
                                                L4Re::Rm::Eager_map,
                                              _ds_ram.get()));
}
