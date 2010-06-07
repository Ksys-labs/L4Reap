IMPLEMENTATION[ppc32]:

#include "cpu.h"
#include <cassert>

IMPLEMENT inline NEEDS [<cassert>, "mem.h", Mem_space::current_pdir]
template < typename T >
void
Mem_space::copy_from_user (T *kdst, T const *usrc, size_t n)
{
  assert (dir() == current_pdir());

  Address phys, offs;
  Mword len = n * sizeof(T);

  while(len)
    {
      phys = lookup((void*)usrc);

      assert(phys != ~0UL);

      offs = (Address)usrc & ~Config::PAGE_MASK;

      /* check page boundary */
      if(offs + len >= Config::PAGE_SIZE)
        len = Config::PAGE_SIZE - offs;

      Mem::memcpy_bytes(kdst,  (void*)(phys + offs), len);

      kdst += len;
      usrc += len;
      len = n -= len;
   }
}

IMPLEMENT inline
template <>
void
Mem_space::copy_from_user (Mword *kdst, Mword const *usrc, size_t n)
{
  copy_from_user<Mword>(kdst, usrc, n / sizeof(Mword));
}

IMPLEMENT inline
template < typename T >
void
Mem_space::copy_to_user (T * /*udst*/, T const * /*ksrc*/, size_t /*n*/)
{
  NOT_IMPL_PANIC;
}

IMPLEMENT inline
template <>
void
Mem_space::copy_to_user <Mword> (Mword * /*udst*/, Mword const * /*ksrc*/,
                                 size_t /* n */)
{
  NOT_IMPL_PANIC;
}

//------------------------------------------------------------------------------
IMPLEMENT inline NEEDS[<cassert>]
template < typename T>
T
Mem_space::peek_user(T const *addr)
{
  Address phys = lookup((void *)addr);

  assert(phys != ~0U);
  phys += (Address)addr & ~Config::PAGE_MASK;

  return *(reinterpret_cast<T*>(phys));
}

IMPLEMENT inline NEEDS[<cassert>]
template < typename T >
void
Mem_space::poke_user(T *addr, T value)
{
  Address phys = lookup((void *)addr);

  assert(phys != ~0U);
  phys += (Address)addr & ~Config::PAGE_MASK;

  *(reinterpret_cast<T*>(phys)) = value;
}
