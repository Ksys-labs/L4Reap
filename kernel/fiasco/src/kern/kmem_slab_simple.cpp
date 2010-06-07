INTERFACE:

#include <cstddef>		// size_t
//#include "helping_lock.h"	// Helping_lock
#include "lock_guard.h"
#include "spin_lock.h"

#include "slab_cache_anon.h"		// slab_cache_anon

class Kmem_slab_simple : public slab_cache_anon
{
  friend class Jdb_kern_info_memory;

  // DATA
  //typedef Helping_lock Lock;
  typedef Spin_lock Lock;
  Lock _lock;
  Kmem_slab_simple* _reap_next;

  // STATIC DATA
  static Kmem_slab_simple* reap_list;
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

// We only support slab size == PAGE_SIZE.
PUBLIC
Kmem_slab_simple::Kmem_slab_simple(unsigned elem_size, unsigned alignment,
				   char const *name)
  : slab_cache_anon(Config::PAGE_SIZE, elem_size, alignment, name)
{
  _lock.init();
  enqueue_reap_list();
}

// Specializations providing their own block_alloc()/block_free() can
// also request slab sizes larger than one page.
PROTECTED
Kmem_slab_simple::Kmem_slab_simple(unsigned long slab_size, 
				   unsigned elem_size, 
				   unsigned alignment,
				   char const *name)
  : slab_cache_anon(slab_size, elem_size, alignment, name)
{
  _lock.init();
  enqueue_reap_list();
}

// Specializations providing their own block_alloc()/block_free() can
// also request slab sizes larger than one page.
PROTECTED
Kmem_slab_simple::Kmem_slab_simple(unsigned elem_size, 
				   unsigned alignment,
				   char const *name,
				   unsigned long min_size,
				   unsigned long max_size)
  : slab_cache_anon(elem_size, alignment, name, min_size, max_size)
{
  _lock.init();
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
  Lock_guard<Lock> guard(&_lock);
  destroy();
}

// We overwrite some of slab_cache_anon's functions to faciliate locking.
PUBLIC
void *
Kmem_slab_simple::alloc()		// request initialized member from cache
{
  Lock_guard<Lock> guard(&_lock);
  return slab_cache_anon::alloc();
}

PUBLIC
void 
Kmem_slab_simple::free(void *cache_entry) // return initialized member to cache
{
  Lock_guard<Lock> guard(&_lock);
  slab_cache_anon::free(cache_entry);
}

PUBLIC
unsigned long
Kmem_slab_simple::reap()
{
  if (_lock.test())
    return 0;			// this cache is locked -- can't get memory now

  Lock_guard<Lock> guard(&_lock);
  return slab_cache_anon::reap();
}

// Callback functions called by our super class, slab_cache_anon, to
// allocate or free blocks

virtual void *
Kmem_slab_simple::block_alloc(unsigned long size, unsigned long)
{
  // size must be exactly PAGE_SIZE
  assert(size == Config::PAGE_SIZE);
  (void)size;

  return Mapped_allocator::allocator()->alloc(Config::PAGE_SHIFT);
}

virtual void 
Kmem_slab_simple::block_free(void *block, unsigned long)
{
  Mapped_allocator::allocator()->free(Config::PAGE_SHIFT,block);
}

// memory management

static void *slab_mem = 0;

PUBLIC
void *
Kmem_slab_simple::operator new(size_t size)
{
//#warning do we really need dynamic allocation of slab allocators?
  assert(size<=sizeof(Kmem_slab_simple));
  (void)size; // prevent gcc warning
  if(!slab_mem)
    {
      slab_mem = Mapped_allocator::allocator()->alloc(Config::PAGE_SHIFT);
      if(!slab_mem)
	panic("Out of memory (new Kmem_slab_simple)");

      char* s;
      for( s = (char*)slab_mem; 
	  s < ((char*)slab_mem) + Config::PAGE_SIZE - sizeof(Kmem_slab_simple); 
	  s+=sizeof(Kmem_slab_simple) ) 
	{
	  *((void**)s) = s+sizeof(Kmem_slab_simple);
	}

      *((void**)s) = 0;
    }

  void *sl = slab_mem;
  slab_mem = *((void**)slab_mem);
  return sl;
  
}

PUBLIC
void 
Kmem_slab_simple::operator delete(void *block) 
{
  *((void**)block) = slab_mem;
  slab_mem = block;
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

static Mapped_alloc_reaper 
  kmem_slab_simple_reaper (Kmem_slab_simple::reap_all);
