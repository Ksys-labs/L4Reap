/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "remote_mem.h"

#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/error_helper>
#include <cstdio>
l4_addr_t
Stack::add(l4_addr_t start, l4_umword_t size, L4::Cap<L4Re::Rm> rm,
           L4::Cap<L4Re::Dataspace> m, unsigned long offs, unsigned flags,
           unsigned char align, char const *what)
{
  unsigned rh_flags = flags;
  // printf("attaching %s %lx... @%lx\n", what, m.cap(), rm.cap());
  if (!m.is_valid())
    rh_flags |= L4Re::Rm::Reserved;

  l4_addr_t addr = start;

  L4Re::chksys(rm->attach(&addr, size, rh_flags, m, offs, align), what);

  l4re_mem_area_t a;
  a.start = addr;
  a.size = size;
  push(a);
  return addr;
}

void
Stack::set_stack(L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap const &ds, unsigned size)
{
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&_vma, size,
                                              L4Re::Rm::Search_addr, ds.get(), 0),
               "attaching stack vma");
  _stack_ds = ds;
  set_local_top((char*)(_vma.get() + size));
}

