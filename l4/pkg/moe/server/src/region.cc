/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "debug.h"
#include "region.h"
#include "exception.h"

#include <l4/sys/kip>

#include <l4/re/rm>
#include <l4/re/rm-sys.h>
#include <l4/re/protocols>

#include <l4/cxx/ipc_stream>
#include <l4/cxx/iostream>
#include <l4/cxx/exceptions>

#include <l4/re/util/region_mapping_svr>

Region_map::Region_map()
  : Base(Moe::Virt_limit::start, Moe::Virt_limit::end)
{
  L4::Kip::Mem_desc *md = L4::Kip::Mem_desc::first(const_cast<l4_kernel_info_t*>(kip()));
  unsigned long cnt = L4::Kip::Mem_desc::count(const_cast<l4_kernel_info_t*>(kip()));

  for (L4::Kip::Mem_desc *m = md; m < md + cnt; ++m)
    {
      if (m->type() != L4::Kip::Mem_desc::Reserved || !m->is_virtual())
        continue;

      l4_addr_t start = m->start();
      l4_addr_t end = m->end();

      attach_area(start, end - start + 1, L4Re::Rm::Reserved);
    }

  attach_area(0, L4_PAGESIZE);
}

int Region_ops::map(Region_handler const *h, l4_addr_t adr,
                    L4Re::Util::Region const &r, bool writable,
                    L4::Snd_fpage *result)
{
  l4_addr_t offs = adr - r.start();
  offs = l4_trunc_page(offs);
  Moe::Dataspace::Ds_rw rw = !h->is_ro() && writable
                             ? Moe::Dataspace::Writable
                             : Moe::Dataspace::Read_only;
  if (h->is_ro() && writable)
    Dbg(Dbg::Warn).printf("WARNING: "
         "Writable mapping request on read-only region at %lx!\n",
         adr);
  *result = L4::Snd_fpage(h->memory()->address(offs + h->offset(), rw, adr,
                          r.start(), r.end()).fp(), offs + r.start());

  return L4_EOK;
}

void
Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
  if (h->is_ro())
    return;

  h->memory()->clear(h->offset() + start, size);
}


class Rm_server
{
public:
  typedef Moe::Dataspace const *Dataspace;
  enum { Have_find = false };
  static int validate_ds(L4::Snd_fpage const &ds_cap,
                         unsigned flags, Dataspace *ds)
  {
    if (flags & L4Re::Rm::Pager)
      return -L4_EINVAL;

    if (!ds_cap.id_received())
      return -L4_ENOENT;

    *ds = dynamic_cast<Moe::Dataspace*>(object_pool.find(ds_cap.data()));

    if (!*ds)
      return -L4_ENOENT;

    if (flags & L4Re::Rm::Read_only)
      return L4_EOK;

    if (!(*ds)->is_writable() || !(ds_cap.data() & L4_FPAGE_X))
      return -L4_EPERM;

    return L4_EOK;
  }

  static l4_umword_t find_res(Dataspace const &) { return 0; }
};

int
Region_map::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  Dbg warn(Dbg::Warn, "WARN");
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4_PROTO_PAGE_FAULT:
    {
      try
        {
          return L4Re::Util::region_pf_handler<Dbg>(this, ios);
	}
      catch (L4::Out_of_memory const &oom)
	{
	  warn.printf("insufficient memory to resolve page fault!\n");
	  return -L4_ENOREPLY;
	}
    }
    case L4Re::Protocol::Rm:
      return L4Re::Util::region_map_server<Rm_server>(this, ios);
    default:
      return -L4_EBADPROTO;
    }
}
