/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "region.h"

#include <l4/sys/kip>

#include <l4/re/dataspace>
#include <l4/re/rm>
#include <l4/re/rm-sys.h>
#include <l4/re/env>
#include <l4/re/protocols>

#include <l4/sys/task>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include <l4/re/util/region_mapping_svr>

#include "global.h"
//#include "dispatcher.h"
#include "debug.h"

#include <cstdio>

using L4Re::Rm;
using L4Re::Dataspace;
using L4Re::Util::Region;

Region_map::Region_map()
  : Base(0,0)
{}

void
Region_map::init()
{
  extern char __L4_KIP_ADDR__[];

  L4::Kip::Mem_desc *md = L4::Kip::Mem_desc::first(__L4_KIP_ADDR__);
  unsigned long cnt = L4::Kip::Mem_desc::count(__L4_KIP_ADDR__);

  for (L4::Kip::Mem_desc *m = md; m < md + cnt; ++m)
    {
      if (!m->is_virtual())
	continue;

      l4_addr_t start = m->start();
      l4_addr_t end = m->end();
      
      switch (m->type())
	{
	case L4::Kip::Mem_desc::Conventional:
	  set_limits(start, end);
	  break;
	case L4::Kip::Mem_desc::Reserved:
	  attach_area(start, end - start + 1, L4Re::Rm::Reserved);
	  break;
	default:
	  break;
	}
    }

  // reserve page at 0
  attach_area(0, L4_PAGESIZE);
}

static l4_addr_t map_area;
static l4_addr_t map_end;

void Region_map::global_init()
{
  L4Re::chksys(L4Re::Env::env()->rm()->reserve_area(&map_area, L4_PAGESIZE,
                 L4Re::Rm::Reserved | L4Re::Rm::Search_addr));
  map_end = map_area + L4_PAGESIZE -1;
  Dbg(Dbg::Boot, "rm").printf("map area %lx-%lx\n", map_area, map_end);
}

int
Region_ops::map(Region_handler const *h, l4_addr_t local_adr,
                Region const &r, bool writable, L4::Ipc::Snd_fpage *result)
{
  if ((h->flags() & Rm::Reserved) || !h->memory().is_valid())
    return -L4_ENOENT;

  if (h->flags() & Rm::Pager)
    return -L4_ENOENT;

  if (h->is_ro() && writable)
    Dbg(Dbg::Warn).printf("WARNING: Writable mapping request on read-only region at %lx!\n", local_adr);

  l4_addr_t offset = local_adr - r.start() + h->offset();
  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());

  if (int err = ds->map(offset, writable, map_area, map_area, map_end))
    return err;

  *result = L4::Ipc::Snd_fpage::mem(map_area, L4_PAGESHIFT, L4_FPAGE_RWX,
                               local_adr, L4::Ipc::Snd_fpage::Grant);
  return L4_EOK;
}

void Region_ops::unmap(Region_handler const * /*h*/, l4_addr_t  /*vaddr*/,
                       l4_addr_t  /*offs*/, unsigned long  /*size*/)
{
}

void
Region_ops::take(Region_handler const * /*h*/)
{
}

void
Region_ops::release(Region_handler const * /*h*/)
{
}

template<typename T>
inline
T extract(L4::Ipc::Istream &s)
{ T t; s >> t; return t; }

void
Region_map::debug_dump(unsigned long /*function*/) const
{
  printf("Region maping: limits [%lx-%lx]\n", min_addr(), max_addr());
  printf(" Area map:\n");
  for (Region_map::Const_iterator i = area_begin(); i != area_end(); ++i)
    printf("  [%10lx-%10lx] -> flags=%x\n",
           i->first.start(), i->first.end(),
	   i->second.flags());
  printf(" Region map:\n");
  for (Region_map::Const_iterator i = begin(); i != end(); ++i)
    printf("  [%10lx-%10lx] -> (offs=%lx, ds=%lx, flags=%x)\n",
           i->first.start(), i->first.end(),
	   i->second.offset(), i->second.memory().cap(),
	   i->second.flags());
}

void
Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
  if (h->is_ro())
    return;

  if ((h->flags() & Rm::Reserved) || !h->memory().is_valid())
    return;

  if (h->flags() & Rm::Pager)
    return;

  L4::Cap<L4Re::Dataspace> ds = L4::cap_cast<L4Re::Dataspace>(h->memory());
  ds->clear(start + h->offset(), size);
}


class Rm_server
{
public:
  typedef L4::Cap<L4Re::Dataspace> Dataspace;
  enum { Have_find = true };
  static int validate_ds(L4::Ipc::Snd_fpage const & /*ds_cap*/, unsigned, L4::Cap<L4Re::Dataspace> *ds)
  {
    // XXX: must check that ds is from trusted allocator!
    *ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
    // XXX: use L4::Cap::move()!
    L4::Cap<L4::Task>(L4Re::This_task)->map(L4Re::This_task, Glbl::rcv_cap.fpage(),
	              ds->snd_base(L4_MAP_ITEM_GRANT));
    return L4_EOK;
  }

  static l4_umword_t find_res(L4::Cap<void> const &ds) { return ds.cap(); }


};

int
Region_map::handle_rm_request(L4::Ipc::Iostream &ios)
{
  return L4Re::Util::region_map_server<Rm_server>(this, ios);
}

int
Region_map::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4_PROTO_PAGE_FAULT:
      return L4Re::Util::region_pf_handler<Dbg>(this, ios);
    case L4Re::Protocol::Rm:
      return L4Re::Util::region_map_server<Rm_server>(this, ios);
    default:
      return -L4_EBADPROTO;
    }
}
