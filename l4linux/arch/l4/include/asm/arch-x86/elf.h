#ifndef __ASM_L4__ARCH_I386__ELF_H__
#define __ASM_L4__ARCH_I386__ELF_H__

/* L4Linux has a Linux native ABI... */
#include <asm-x86/elf.h>

/* ...but dosen't have segment registers */
#undef ELF_CORE_COPY_REGS
#ifdef CONFIG_X86_32
#define ELF_CORE_COPY_REGS(pr_reg, regs)		\
	pr_reg[0] = regs->bx;				\
	pr_reg[1] = regs->cx;				\
	pr_reg[2] = regs->dx;				\
	pr_reg[3] = regs->si;				\
	pr_reg[4] = regs->di;				\
	pr_reg[5] = regs->bp;				\
	pr_reg[6] = regs->ax;				\
	pr_reg[7] = 0; /* fake ds */			\
	pr_reg[8] = 0; /* fake es */			\
	pr_reg[9] = regs->fs;				\
	pr_reg[10] = 0; /* fake gs */			\
	pr_reg[11] = regs->orig_ax;			\
	pr_reg[12] = regs->ip;				\
	pr_reg[13] = 0; /* fake cs */			\
	pr_reg[14] = regs->flags;			\
	pr_reg[15] = regs->sp;				\
	pr_reg[16] = 0; /* fake ss */
#else
#define ELF_CORE_COPY_REGS(pr_reg, regs)			\
do {								\
	unsigned v;						\
	(pr_reg)[0] = (regs)->r15;				\
	(pr_reg)[1] = (regs)->r14;				\
	(pr_reg)[2] = (regs)->r13;				\
	(pr_reg)[3] = (regs)->r12;				\
	(pr_reg)[4] = (regs)->bp;				\
	(pr_reg)[5] = (regs)->bx;				\
	(pr_reg)[6] = (regs)->r11;				\
	(pr_reg)[7] = (regs)->r10;				\
	(pr_reg)[8] = (regs)->r9;				\
	(pr_reg)[9] = (regs)->r8;				\
	(pr_reg)[10] = (regs)->ax;				\
	(pr_reg)[11] = (regs)->cx;				\
	(pr_reg)[12] = (regs)->dx;				\
	(pr_reg)[13] = (regs)->si;				\
	(pr_reg)[14] = (regs)->di;				\
	(pr_reg)[15] = (regs)->orig_ax;				\
	(pr_reg)[16] = (regs)->ip;				\
	(pr_reg)[17] = (regs)->cs;				\
	(pr_reg)[18] = (regs)->flags;				\
	(pr_reg)[19] = (regs)->sp;				\
	(pr_reg)[20] = (regs)->ss;				\
	(pr_reg)[21] = current->thread.fs;			\
	(pr_reg)[22] = current->thread.gs;			\
	asm("movl %%ds,%0" : "=r" (v)); (pr_reg)[23] = v;	\
	asm("movl %%es,%0" : "=r" (v)); (pr_reg)[24] = v;	\
	asm("movl %%fs,%0" : "=r" (v)); (pr_reg)[25] = v;	\
	asm("movl %%gs,%0" : "=r" (v)); (pr_reg)[26] = v;	\
} while (0);
#endif

#include <asm/api/config.h>

#undef VDSO_HIGH_BASE
#define VDSO_HIGH_BASE (UPAGE_USER_ADDRESS)

#undef VDSO_SYM
#define VDSO_SYM(x) ((unsigned long)(x))

#endif /* ! __ASM_L4__ARCH_I386__ELF_H__ */
