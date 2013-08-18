INTERFACE:

#include "types.h"

//----------------------------------------------------------------------------
IMPLEMENTATION[arm && !armv6plus]:

PRIVATE static inline NOEXPORT NEEDS["types.h"]
void
Sys_call_page::set_utcb_get_code(Unsigned32 *sys_calls)
{
  unsigned offset = 0;
  sys_calls[offset++] = 0xe3e00a02; // mvn r0, #8192
  sys_calls[offset++] = 0xe5100fff; // ldr r0, [r0, -#4095]
  sys_calls[offset++] = 0xe1a0f00e; // mov pc, lr

  // set TLS REG
  offset = 0x20;
  sys_calls[offset++] = 0xe1a0f00e; // mov pc, lr
}

//----------------------------------------------------------------------------
IMPLEMENTATION[arm && armv6plus]:

PRIVATE static inline NOEXPORT NEEDS["types.h"]
void
Sys_call_page::set_utcb_get_code(Unsigned32 *sys_calls)
{
  unsigned offset = 0;

  // sys_calls[0] = 0xee1d0f70; // mrc 15, 0, r0, cr13, cr0, {3}
  sys_calls[offset++] = 0xee1d0f50; // mrc 15, 0, r0, cr13, cr0, {2}
  sys_calls[offset++] = 0xe1a0f00e; // mov pc, lr

  // set TLS REG
  offset = 0x20;
  sys_calls[offset++] = 0xe92d4010; // push    {r4, lr}
  sys_calls[offset++] = 0xee1d4f50; // mrc     15, 0, r4, cr13, cr0, {2}
  sys_calls[offset++] = 0xe5840004; // str     r0, [r4, #4]
  sys_calls[offset++] = 0xe3a02010; // mov     r2, #0x10 -> set tls opcode
  sys_calls[offset++] = 0xe5842000; // str     r2, [r4]
  sys_calls[offset++] = 0xe3a03000; // mov     r3, #0
  sys_calls[offset++] = 0xe30f2803; // movw    r2, #0xf803
  sys_calls[offset++] = 0xe3a00002; // mov     r0, #2
  sys_calls[offset++] = 0xe34f2fff; // movt    r2, #0xffff
  sys_calls[offset++] = 0xe34f0ff4; // movt    r0, #0xfff4
  sys_calls[offset++] = 0xe1a0e00f; // mov     lr, pc
  sys_calls[offset++] = 0xe3e0f00b; // mvn     pc, #11
  sys_calls[offset++] = 0xe8bd8010; // pop     {r4, pc}
}

//----------------------------------------------------------------------------
IMPLEMENTATION:

#include <cstring>
#include "kernel_task.h"
#include "mem_layout.h"
#include "vmem_alloc.h"
#include "panic.h"

IMPLEMENT static
void
Sys_call_page::init()
{
  Unsigned32 *sys_calls = (Unsigned32*)Kmem_alloc::allocator()->unaligned_alloc(Config::PAGE_SIZE);
  if (!sys_calls)
    panic("FIASCO: can't allocate system-call page.\n");

  for (unsigned i = 0; i < Config::PAGE_SIZE / sizeof(Unsigned32); ++i)
    sys_calls[i] = 0xef000000; // svc

  set_utcb_get_code(sys_calls + (0xf00 / sizeof(Unsigned32)));
  Mem_unit::flush_cache();

  Kernel_task::map_syscall_page(sys_calls);
}
