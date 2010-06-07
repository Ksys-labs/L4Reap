INTERFACE [ia32 || amd64 || ux]:

EXTENSION class Mem_layout
{
public:
  enum { Io_port_max = (1UL << 16) };
};

IMPLEMENTATION [ia32 || amd64 || ux]:

#include "static_assert.h"

PUBLIC static inline NEEDS["static_assert.h"]
template< typename V >
bool
Mem_layout::read_special_safe(V const *address, V &v)
{
  static_assert(sizeof(v) <= sizeof(Mword), "wrong sized argument");
  Mword value;
  bool res;
  asm volatile ("clc; mov (%[adr]), %[val]; setnc %b[ex] \n"
      : [val] "=acd" (value), [ex] "=r" (res)
      : [adr] "acdbSD" (address)
      : "cc");
  v = V(value);
  return res;
}

PUBLIC static inline NEEDS["static_assert.h"]
template< typename T >
T
Mem_layout::read_special_safe(T const *a)
{
  static_assert(sizeof(T) <= sizeof(Mword), "wrong sized return type");
  Mword res;
  asm volatile ("mov (%1), %0 \n\t"
      : "=acd" (res) : "acdbSD" (a) : "cc");
  return T(res);

}

PUBLIC static inline
bool
Mem_layout::is_special_mapped(void const *a)
{
  // Touch the state to page in the TCB. If we get a pagefault here,
  // the handler doesn't handle it but returns immediatly after
  // setting eax to 0xffffffff
  Mword pagefault_if_0;
  asm volatile (
      "clc; mov (%2), %0 \n\t"
      "setnc %b0         \n\t"
      : "=acd" (pagefault_if_0)
      : "0"(0UL), "acdbSD"(a)
      : "cc");
  return pagefault_if_0;
}

