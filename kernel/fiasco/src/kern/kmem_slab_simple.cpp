INTERFACE:

#include <cstddef>		// size_t
#include "config.h"
//#include "helping_lock.h"	// Helping_lock
#include "lock_guard.h"
#include "spin_lock.h"

#include "slab_cache_anon.h"		// slab_cache_anon

class Kmem_slab_simple : public slab_cache_anon
{
  friend class Jdb_kern_info_memory;

  // DATA
  //typedef Helping_lock Lock;
  Kmem_slab_simple* _reap_next;

  // STATIC DATA
  static Kmem_slab_simple* reap_list;
};

template< typename T >
class Kmem_slab_t : public Kmem_slab_simple
{
public:
  explicit Kmem_slab_t(char const *name)
  : Kmem_slab_simple(sizeof(T), __alignof(T), name) {}
};

IMPLEMENTATION:

Kmem_slab_simple* Kmem_slab_simple::reap_list;

// Kmem_slab_simple -- A type-independent slab cache allocator for Fiasco,
// derived from a generic slab cache allocator (slab_cache_anon in
// lib/slab.cpp).

// This specialization adds low-level page allocation and locking to
// the slab allocator implemented in our base class (slab_cache_anon).
//-

#include <cassert>
#include "config.h"
#include "atomic.h"
#include "panic.h"
#include "mapped_alloc.h"

// Specializations providing their own block_alloc()/block_free() can
// also request slab sizes larger than one page.
PROTECTED
Kmem_slab_simple::Kmem_slab_simple(unsigned long slab_size,
				   unsigned elem_size,
				   unsigned alignment,
				   char const *name)
  : slab_cache_anon(slab_size, elem_size, alignment, name)
{
  enqueue_reap_list();
}

// Specializations providing their own block_alloc()/block_free() can
// also request slab sizes larger than one page.
PUBLIC
Kmem_slab_simple::Kmem_slab_simple(unsigned elem_size,
				   unsigned alignment,
				   char const *name,
				   unsigned long min_size = Config::PAGE_SIZE,
				   unsigned long max_size = Config::PAGE_SIZE * 32)
  : slab_cache_anon(elem_size, alignment, name, min_size, max_size)
{
  enqueue_reap_list();
}

void
Kmem_slab_simple::enqueue_reap_list()
{
  do {
    _reap_next = reap_list;
  } while (! cas (&reap_list, _reap_next, this));
}

PUBLIC
Kmem_slab_simple::~Kmem_slab_simple()
{
  destroy();
}

// We overwrite some of slab_cache_anon's functions to faciliate locking.
PUBLIC
void *
Kmem_slab_simple::alloc()		// request initialized member from cache
{
  return slab_cache_anon::alloc();
}

PUBLIC
void
Kmem_slab_simple::free(void *cache_entry) // return initialized member to cache
{
  slab_cache_anon::free(cache_entry);
}

PUBLIC
unsigned long
Kmem_slab_simple::reap()
{
  return slab_cache_anon::reap();
}

// Callback functions called by our super class, slab_cache_anon, to
// allocate or free blocks

virtual void *
Kmem_slab_simple::block_alloc(unsigned long size, unsigned long)
{
  assert (size >= Config::PAGE_SIZE && !(size & (size - 1)));
  (void)size;
  return Mapped_allocator::allocator()->unaligned_alloc(size);
}

virtual void
Kmem_slab_simple::block_free(void *block, unsigned long size)
{
  Mapped_allocator::allocator()->unaligned_free(size, block);
}

// 
// Memory reaper
// 
PUBLIC static
size_t
Kmem_slab_simple::reap_all (bool desperate)
{
  size_t freed = 0;

  for (Kmem_slab_simple* alloc = reap_list;
       alloc;
       alloc = alloc->_reap_next)
    {
      size_t got;
      do
	{
	  got = alloc->reap();
	  freed += got;
	}
      while (desperate && got);
    }

  return freed;
}

static Mapped_alloc_reaper kmem_slab_simple_reaper(Kmem_slab_simple::reap_all);
