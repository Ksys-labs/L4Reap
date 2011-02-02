INTERFACE:

#include "mapped_alloc.h"
#include "spin_lock.h"
#include "lock_guard.h"
#include "initcalls.h"

class Buddy_alloc;
class Mem_region_map_base;
class Kip;

class Kmem_alloc : public Mapped_allocator
{
  Kmem_alloc();

public:
  typedef Buddy_alloc Alloc;
private:
  typedef Spin_lock<> Lock;
  static Lock lock;
  static Alloc *a;
  static unsigned long _orig_free;
};


IMPLEMENTATION:

#include <cassert>

#include "config.h"
#include "kdb_ke.h"
#include "kip.h"
#include "mem_region.h"
#include "buddy_alloc.h"
#include "panic.h"

static Kmem_alloc::Alloc _a;
Kmem_alloc::Alloc *Kmem_alloc::a = &_a;
unsigned long Kmem_alloc::_orig_free;
Kmem_alloc::Lock Kmem_alloc::lock;

PUBLIC static FIASCO_INIT
void
Kmem_alloc::init()
{
  static Kmem_alloc al;
  Mapped_allocator::allocator(&al);
}

PUBLIC
void
Kmem_alloc::dump() const
{ a->dump(); }

PUBLIC
void *
Kmem_alloc::alloc(size_t o)
{
  return unaligned_alloc(1UL << o);
}


PUBLIC
void
Kmem_alloc::free(size_t o, void *p)
{
  unaligned_free(1UL << o, p);
}

PUBLIC 
void *
Kmem_alloc::unaligned_alloc(unsigned long size)
{
  assert(size >=8 /*NEW INTERFACE PARANIOIA*/);
  void* ret;

  {
    Lock_guard<Lock> guard(&lock);
    ret = a->alloc(size);
  }

  if (!ret)
    {
      Mapped_alloc_reaper::morecore (/* desperate= */ true);

      Lock_guard<Lock> guard (&lock);
      ret = a->alloc(size);
    }

  return ret;
}

PUBLIC
void
Kmem_alloc::unaligned_free(unsigned long size, void *page)
{
  assert(size >=8 /*NEW INTERFACE PARANIOIA*/);
  Lock_guard<Lock> guard (&lock);
  a->free(page, size);
}


PRIVATE static FIASCO_INIT
unsigned long
Kmem_alloc::create_free_map(Kip const *kip, Mem_region_map_base *map)
{
  unsigned long available_size = 0;
  Mem_desc const *md = kip->mem_descs();
  Mem_desc const *const md_end = md + kip->num_mem_descs();

  for (; md < md_end; ++md)
    {
      if (!md->valid())
	{
	  const_cast<Mem_desc*>(md)->type(Mem_desc::Undefined);
	  continue;
	}

      if (md->is_virtual())
	continue;

      unsigned long s = md->start();
      unsigned long e = md->end();

      // Sweep out stupid descriptors (that have the end before the start)

      switch (md->type())
	{
	case Mem_desc::Conventional:
	  s = (s + Config::PAGE_SIZE - 1) & ~(Config::PAGE_SIZE - 1);
	  e = ((e + 1) & ~(Config::PAGE_SIZE - 1)) - 1;
	  if (e <= s)
	    break;
	  available_size += e - s + 1;
	  if (!map->add(Mem_region(s, e)))
	    panic("Kmem_alloc::create_free_map(): memory map too small");
	  break;
	case Mem_desc::Reserved:
	case Mem_desc::Dedicated:
	case Mem_desc::Shared:
	case Mem_desc::Arch:
	case Mem_desc::Bootloader:
	  s = s & ~(Config::PAGE_SIZE - 1);
	  e = ((e + Config::PAGE_SIZE) & ~(Config::PAGE_SIZE - 1)) - 1;
	  if (!map->sub(Mem_region(s, e)))
	    panic("Kmem_alloc::create_free_map(): memory map too small");
	  break;
	default:
	  break;
	}
    }

  return available_size;
}
