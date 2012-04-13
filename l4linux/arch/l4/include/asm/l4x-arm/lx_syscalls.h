#ifndef __ASM_L4__L4X__I386__LX_SYSCALLS_H__
#define __ASM_L4__L4X__I386__LX_SYSCALLS_H__

typedef asmlinkage int (*syscall_t)(long a0,...);
extern syscall_t sys_call_table[];
extern syscall_t sys_oabi_call_table[];
extern char NR_syscalls[];

static inline int is_lx_syscall(int nr)
{
	return nr < (unsigned long)NR_syscalls;
}

#endif /* ! __ASM_L4__L4X__I386__LX_SYSCALLS_H__ */
