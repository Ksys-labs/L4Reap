#ifndef __ASM_L4__L4X__I386__LX_SYSCALLS_H__
#define __ASM_L4__L4X__I386__LX_SYSCALLS_H__

#include <asm/syscall.h>

typedef asmlinkage int (*syscall_t)(long a0,...);

static inline int is_lx_syscall(int nr)
{
	return nr < NR_syscalls;
}

#endif /* ! __ASM_L4__L4X__I386__LX_SYSCALLS_H__ */
