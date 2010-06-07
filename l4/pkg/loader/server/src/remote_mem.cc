/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <cstring>
#include <cstdio>

#include "remote_mem.h"
#include "app_task.h"

l4_addr_t
Stack::add(l4_addr_t start, l4_umword_t size, Region_map *rm,
                L4::Cap<L4Re::Dataspace> m, unsigned long offs, unsigned flags,
                unsigned char align, char const *what)
{
  (void) what;
  unsigned rh_flags = flags;
  if (!m.is_valid())
    rh_flags |= L4Re::Rm::Reserved;

  void *x = rm->attach((void*)start, size, Region_handler(m, L4_INVALID_CAP, offs, rh_flags),
        rh_flags, align);
  if (x == L4_INVALID_PTR)
    return 0;

  l4re_mem_area_t a;
  a.start = (l4_addr_t)x;
  a.size = size;
  push(a);
  return l4_addr_t(x);
}
