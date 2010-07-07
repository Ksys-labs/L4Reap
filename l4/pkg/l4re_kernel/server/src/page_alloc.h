/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/mem_alloc>
class Region_map;

class Single_page_alloc_base
{
public:
  static void init(L4::Cap<L4Re::Mem_alloc> alloc);
  static void init_local_rm(Region_map *lrm);
protected:
  Single_page_alloc_base();
  static void *_alloc();
  static void _free(void *p);
};

template<typename A>
class Single_page_alloc : public Single_page_alloc_base
{
public:
  enum { can_free = true };
  A *alloc() { return reinterpret_cast<A*>(_alloc()); }
  void free(A *o) { _free(o); }
};

