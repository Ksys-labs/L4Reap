// ------------------------------------------------------------------------
IMPLEMENTATION [arm && armv5]:

#include "mem_layout.h"

IMPLEMENT inline NEEDS["mem_layout.h"]
Utcb *
Utcb_support::current()
{ return *((Utcb**)Mem_layout::Utcb_ptr_page); }

IMPLEMENT inline NEEDS["mem_layout.h"]
void
Utcb_support::current(Utcb *utcb)
{ *((Utcb**)Mem_layout::Utcb_ptr_page) = utcb; }

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && (armv6 || armv7)]:

IMPLEMENT inline
Utcb *
Utcb_support::current()
{
  Utcb *u;
  asm volatile ("mrc p15, 0, %0, c13, c0, 3" : "=r" (u));
  return u;
}

IMPLEMENT inline
void
Utcb_support::current(Utcb *utcb)
{
  asm volatile ("mcr p15, 0, %0, c13, c0, 3" : : "r" (utcb) : "memory");
}
