//---------------------------------------------------------------------------
INTERFACE [arm && mp]:

EXTENSION class Spin_lock
{
public:
  enum { Arch_lock = 2 };
};
//---------------------------------------------------------------------------
IMPLEMENTATION [arm && mp]:

#include "processor.h"

PRIVATE inline NEEDS["processor.h"]
void
Spin_lock::lock_arch()
{
  unsigned long dummy, tmp;

  __asm__ __volatile__ (
      "1: ldr     %[d], [%[lock]]           \n"
      "   tst     %[d], #2                  \n" // Arch_lock == #2
      "   wfene                             \n"
      "   bne     1b                        \n"
      "   ldrex   %[d], [%[lock]]           \n"
      "   tst     %[d], #2                  \n"
      "   orr     %[tmp], %[d], #2          \n"
      "   strexeq %[d], %[tmp], [%[lock]]   \n"
      "   teqeq   %[d], #0                  \n"
      "   bne     1b                        \n"
      : [d] "=&r" (dummy), [tmp] "=&r"(tmp), "+m" (_lock)
      : [lock] "r" (&_lock)
      : "cc"
      );
}

PRIVATE inline
void
Spin_lock::unlock_arch()
{
  unsigned long tmp;
  __asm__ __volatile__(
      "ldr %[tmp], %[lock]             \n"
      "bic %[tmp], %[tmp], #2          \n" // Arch_lock == #2
      "str %[tmp], %[lock]             \n"
      "mcr p15, 0, %[tmp], c7, c10, 4  \n" // drain write buffer
      "sev                             \n"
      : [lock] "=m" (_lock), [tmp] "=&r" (tmp));
}
