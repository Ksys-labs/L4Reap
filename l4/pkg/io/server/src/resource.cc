/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
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
Resource::dump(char const *ty, int indent) const
{ printf("<%p>", this);
  //bool abs = true;

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
      s = _s;
      e = _e;
    }

  static char const * const irq_trigger[]
    = { "none", // 0
        "raising edge", // 1
        "<unkn>", // 2
        "level high", // 3
        "<unkn>", // 4
        "falling edge", // 5
        "<unkn>", // 6
        "level low", // 7
        "<unkn>", // 8
        "both edges", // 9
        "<unkn>", // 10
        "<unkn>", // 11
        "<unkn>", // 12
        "<unkn>", // 13
        "<unkn>", // 14
        "<unkn>", // 15
    };

  char const *tp = prefetchable() ? "pref" : "non-pref";
  if (type() == Irq_res)
    tp = irq_trigger[(flags() / Irq_type_base) & 0xf];

  printf("%*.s%s%c [%014llx-%014llx %llx] %s (%dbit) (align=%llx flags=%lx)\n",
         indent, " ",
         ty, provided() ? '*' : ' ',
         s, e, (l4_uint64_t)size(),
         tp,
         is_64bit() ? 64 : 32, (unsigned long long)alignment(), flags());
}


void
Resource::dump(int indent) const
{
  static char const *ty[] = { "INVALID", "IRQ   ", "IOMEM ", "IOPORT",
                              "BUS   ", "unk" };

  dump(ty[type()], indent);
}



bool
Resource_provider::_RS::request(Resource *parent, Device *,
                                Resource *child, Device *)
{
  Addr start = child->start();
  Addr end   = child->end();

  if (end < start)
    return false;

  if (start < parent->start())
    return false;

  if (end > parent->end())
    return false;

  Resource_list::iterator r = _rl.begin();
  while (true)
    {
      if (r == _rl.end() || (*r)->start() > end)
	{
	  // insert before r
	  _rl.insert(r, child);
	  child->parent(parent);
	  return true;
	}

      if ((*r)->end() >= start)
	return false;

      ++r;
    }
}


bool
Resource_provider::_RS::alloc(Resource *parent, Device *pdev,
                              Resource *child, Device *cdev,
                              bool resize)
{
  Resource_list::iterator p = _rl.begin();
  Addr start = parent->start();
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
	end = parent->end();

      Size align = cxx::max<Size>(child->alignment(), min_align);
      start = (start + align) & ~align; // pad to get alignment

      if (start < end && end - start >= (Addr)child->size() - 1)
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

	  parent->end(end);
	  child->start(start);
	  break;
	}

      start = (*p)->end() + 1;
      ++p;
    }
  return request(parent, pdev, child, cdev);
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

  add_flags(Resource::F_fixed_size | Resource::F_fixed_addr);

  L4Re::chksys(L4Re::Env::env()->rm()->attach(&_r, ds_size,
                                              L4Re::Rm::Search_addr |
                                                L4Re::Rm::Eager_map,
                                              _ds_ram.get()));
}
