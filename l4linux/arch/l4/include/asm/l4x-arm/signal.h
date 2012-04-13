#ifndef __ASM_L4__L4X_ARM__SIGNAL_H__
#define __ASM_L4__L4X_ARM__SIGNAL_H__

#include <asm/ptrace.h>

int do_signal(struct pt_regs *regs, int syscall);

extern void l4x_show_sigpending_processes(void);

static inline int l4x_do_signal(struct pt_regs *regs, int syscall)
{
	return do_signal(regs, syscall);
}

#endif /* ! __ASM_L4__L4X_ARM__SIGNAL_H__ */
