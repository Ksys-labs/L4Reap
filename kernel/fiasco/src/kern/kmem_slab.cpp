INTERFACE:

#include "kmem_slab_simple.h"

#include "config.h"

class Kmem_slab : public Kmem_slab_simple
{
};

IMPLEMENTATION[!ppc32]:

// Kmem_slab -- A type-independent slab cache allocator for Fiasco,
// derived from a generic slab cache allocator (slab_cache_anon in
// lib/slab.{h,cc}) and from Kmem_slab_simple which handles locking
// and Kmem_slab-instance allocation for us.

// This specialization adds multi-page slabs.
// We allocate multi-page slabs as multiple, potentially
// non-contiguous physical pages and map them contiguously into a
// region of our virtual address space managed by region_t (in
// region.{h,cc}).

// XXX This implementation technique has one important drawback: When
// using virtual pages, we need TLB entries for these 4KB pages, thus
// increasing the number of 4KB-page TLB misses, whereas accesses to
// physical pages would use superpage (4MB) mappings using the
// 4MB-page TLB.  The load on the superpage TLB is substantially
// lower.  Maybe we should try to find a contiguous physical region
// first and only use virtual regions as a last resort?
//-

#include <cassert>

#include "vmem_alloc.h"
#include "kmem.h"
#include "mapped_alloc.h"
#include "mem_layout.h"
#include "region.h"

PRIVATE static
void
Kmem_slab::init_region_map()
{  
  static bool region_initialized = false;

  if (!region_initialized)
    {
      region_initialized = true; 
      Region::init (Mem_layout::Slabs_start, Mem_layout::Slabs_end);
    }
}


PUBLIC
Kmem_slab::Kmem_slab(unsigned long slab_size, unsigned elem_size, 
		     unsigned alignment, char const *name)
  : Kmem_slab_simple (slab_size, elem_size, alignment, name)
{
  init_region_map();
}

PUBLIC
Kmem_slab::Kmem_slab(unsigned elem_size, 
		     unsigned alignment, char const *name,
		     unsigned long min_size = Config::PAGE_SIZE,
		     unsigned long max_size = Config::PAGE_SIZE * 32)
  : Kmem_slab_simple (elem_size, alignment, name, min_size, max_size)
{
  init_region_map();
}

// Callback functions called by our super class, slab_cache_anon, to
// allocate or free blocks

virtual
void *
Kmem_slab::block_alloc(unsigned long size, unsigned long alignment)
{
  // size must be a power of two of PAGE_SIZE
  assert(size >= Config::PAGE_SIZE
	 && (size & (size - 1)) == 0); // only one bit set -> power of two

  // If the size is just one page, just allocate a page from kmem; do
  // not reserve a region for it.
  if (size == Config::PAGE_SIZE && alignment <= Config::PAGE_SIZE)
    return Mapped_allocator::allocator()->alloc(Config::PAGE_SHIFT);// 2^0 = 1 page

  Address vaddr = Region::reserve_pages(size, alignment);
  if (! vaddr)
    return 0;

  // Fine, we reserved virtual addresses for the buffer.  Now actually
  // allocate pages.
  for (Address a = vaddr; a < vaddr + size; a += Config::PAGE_SIZE)
    {
      if (Vmem_alloc::page_alloc((void*)a, Vmem_alloc::ZERO_FILL))
	continue;		// successfully allocated a page

      // Error - out of memory.  Undo everything.
      for (Address i = vaddr; i < a; i += Config::PAGE_SIZE)
	Vmem_alloc::page_free((void*)i);

      Region::return_pages(vaddr, size);
      return 0;			// return error
    }

  return reinterpret_cast<void*>(vaddr);
}

virtual
void
Kmem_slab::block_free(void *block, unsigned long size)
{
  // We didn't reserve a memory region if the allocation was just one
  // page.  Otherwise, we need to free the region.
  if (size == Config::PAGE_SIZE)
    {
      Mapped_allocator::allocator()->free(Config::PAGE_SHIFT, block);
      return;
    }

  for (char *p = reinterpret_cast<char*>(block);
       p < reinterpret_cast<char*>(block) + size;
       p += Config::PAGE_SIZE)
    {
      Vmem_alloc::page_free(p);
    }

  Region::return_pages(reinterpret_cast<Address>(block), size);
}
