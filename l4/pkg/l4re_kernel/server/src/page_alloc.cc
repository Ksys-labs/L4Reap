/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include "page_alloc.h"
#include "globals.h"
#include "region.h"

using L4Re::Mem_alloc;
using L4Re::Dataspace;
using L4Re::chksys;

enum { 
  Heap_max = L4_PAGESIZE * 64,
  Heap_base = 0xb0100000,
};


static L4Re::Util::Item_alloc<(Heap_max+L4_PAGESIZE-1)/L4_PAGESIZE> _heap_alloc;
static L4::Cap<L4Re::Dataspace> heap;

void
Single_page_alloc_base::init(L4::Cap<Mem_alloc> alloc)
{
  heap = Global::cap_alloc.alloc<L4Re::Dataspace>();
  chksys(alloc->alloc(Heap_max, heap));
  void *hp = (void*)Heap_base;
  chksys(L4Re::Env::env()->rm()->attach(&hp, Heap_max, 0, heap, 0));
}

void
Single_page_alloc_base::init_local_rm(Region_map *lrm)
{
  lrm->attach((void*)Heap_base, Heap_max, Region_handler(heap, heap.cap(), 0, 0), 0);
}

Single_page_alloc_base::Single_page_alloc_base()
{}

void *Single_page_alloc_base::_alloc()
{
  long p = _heap_alloc.alloc();
  if (p < 0)
    return 0;

  return (void*)(Heap_base + (p*L4_PAGESIZE));
}

void Single_page_alloc_base::_free(void *p)
{
  unsigned long idx = ((unsigned long)p - Heap_base) / L4_PAGESIZE;
  _heap_alloc.free(idx);
}

