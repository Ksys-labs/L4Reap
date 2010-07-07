/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/exceptions>
#include <gc.h>

class Single_page_alloc_base
{
public:
  enum Nothrow { nothrow };

protected:
  Single_page_alloc_base();
  static void *_alloc(Nothrow);

  static void *_alloc()
  {
    void *r = _alloc(nothrow);
    if (!r)
      throw L4::Out_of_memory();


    GC_remove_roots(r, (char*)r + L4_PAGESIZE);
    return r;
  }

  static void _free(void *p);

public:
  static void *_alloc(Nothrow, unsigned long size, unsigned long align = 0);
  static void *_alloc(unsigned long size, unsigned long align = 0)
  {
    void *r = _alloc(nothrow, size, align);
    if (!r)
      throw L4::Out_of_memory();
    GC_remove_roots(r, (char*)r + size);
    return r;
  }
  static void _free(void *p, unsigned long size, bool initial_mem = false);
  static unsigned long _avail();
};

template<typename A>
class Single_page_alloc : public Single_page_alloc_base
{
public:
  enum { can_free = true };
  A *alloc() { return reinterpret_cast<A*>(_alloc()); }
  A *alloc(Nothrow) { return reinterpret_cast<A*>(_alloc(nothrow)); }
  void free(A *o) { _free(o); }
};

