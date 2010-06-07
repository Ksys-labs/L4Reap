IMPLEMENTATION [ppc32]:

#include "mapped_alloc.h"
#include "util.h"

#include <cassert>

PUBLIC
Kmem_slab::Kmem_slab(unsigned long slab_size, unsigned elem_size, 
		     unsigned alignment, char const *name)
  : Kmem_slab_simple (slab_size, elem_size, alignment, name)
{
}

PUBLIC
Kmem_slab::Kmem_slab(unsigned elem_size, 
		     unsigned alignment, char const *name,
		     unsigned long min_size = Config::PAGE_SIZE,
		     unsigned long max_size = Config::PAGE_SIZE * 32)
  : Kmem_slab_simple (elem_size, alignment, name, min_size, max_size)
{
}
//XXX cbass: possibly need special allocator for block allocations
virtual
void *
Kmem_slab::block_alloc(unsigned long size, unsigned long alignment)
{
  
  //size must be a power of two of PAGE_SIZE
  assert(size >= Config::PAGE_SIZE
	 && (size & (size - 1)) == 0); // only one bit set -> power of two

  // just use the buddy allocator for now, calculate maximum of size and
  // alignment
  size = size > alignment ? Util::log2(size) : Util::log2(alignment);
  return Mapped_allocator::allocator()->alloc(size);
}

virtual
void
Kmem_slab::block_free(void *block, unsigned long size)
{
  Mapped_allocator::allocator()->free(Util::log2(size), block);
}
