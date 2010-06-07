INTERFACE:

#include <cstddef> // size_t
#include "types.h"

class Mapped_allocator
{
public:
  /// allocate s bytes size-aligned
  virtual void *alloc(size_t order) = 0;

  /// free s bytes previously allocated with alloc(s)
  virtual void free(size_t order, void *p) = 0;

  virtual void *unaligned_alloc(unsigned long size) = 0;
  virtual void unaligned_free(unsigned long size, void *p) = 0;

  virtual void dump() const {}
private:
  static Mapped_allocator *_alloc;
};

class Mapped_alloc_reaper
{
  size_t (*_reap)(bool desperate);
  Mapped_alloc_reaper* _next;

private:
  static Mapped_alloc_reaper* mem_reapers;
};


IMPLEMENTATION:

#include <cassert>

#include "mem_layout.h"

// 
// class Mapped_allocator
// 

Mapped_allocator* Mapped_allocator::_alloc;

PUBLIC static
Mapped_allocator *
Mapped_allocator::allocator()
{
  assert (_alloc /* uninitialized use of Mapped_allocator */);
  return _alloc;
}

PROTECTED static
void
Mapped_allocator::allocator(Mapped_allocator *a)
{
  _alloc=a;
}

PUBLIC inline NEEDS["mem_layout.h"]
void Mapped_allocator::free_phys(size_t s, Address p)
{
  void *va = (void*)Mem_layout::phys_to_pmem(p);
  if((unsigned long)va != ~0UL)
    free(s, va);
}

PUBLIC template< typename Q >
inline
void *
Mapped_allocator::q_alloc(Q *quota, size_t order)
{
  if (EXPECT_FALSE(!quota->alloc(1UL<<order)))
    return 0;

  void *b;
  if (EXPECT_FALSE(!(b=alloc(order))))
    {
      quota->free(1UL<<order);
      return 0;
    }

  return b;
}

PUBLIC template< typename Q >
inline
void *
Mapped_allocator::q_unaligned_alloc(Q *quota, size_t size)
{
  if (EXPECT_FALSE(!quota->alloc(size)))
    return 0;

  void *b;
  if (EXPECT_FALSE(!(b=unaligned_alloc(size))))
    {
      quota->free(size);
      return 0;
    }

  return b;
}

PUBLIC template< typename Q >
inline
void 
Mapped_allocator::q_free_phys(Q *quota, size_t order, Address obj)
{
  free_phys(order, obj);
  quota->free(1UL<<order);
}

PUBLIC template< typename Q >
inline
void 
Mapped_allocator::q_free(Q *quota, size_t order, void *obj)
{
  free(order, obj);
  quota->free(1UL<<order);
}

PUBLIC template< typename Q >
inline
void 
Mapped_allocator::q_unaligned_free(Q *quota, size_t size, void *obj)
{
  unaligned_free(size, obj);
  quota->free(size);
}

// 
// class Mapped_alloc_reaper
// 

#include "atomic.h"
#include "warn.h"

Mapped_alloc_reaper* Mapped_alloc_reaper::mem_reapers;

PUBLIC inline NEEDS["atomic.h"]
Mapped_alloc_reaper::Mapped_alloc_reaper (size_t (*reap)(bool desperate))
  : _reap (reap)
{
  do {
    _next = mem_reapers;
  } while (! cas (&mem_reapers, _next, this));
}

PUBLIC static
size_t
Mapped_alloc_reaper::morecore (bool desperate = false)
{
  size_t freed = 0;

  for (Mapped_alloc_reaper* reaper = mem_reapers;
       reaper;
       reaper = reaper->_next)
    {
      freed += reaper->_reap(desperate);
    }
#if 0
  if (desperate)
    WARN ("morecore freed %lu bytes of memory\n",
	  static_cast<unsigned long>(freed));
#endif

  return freed;
}
