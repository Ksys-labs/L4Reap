/*
 *  arch/arm/include/asm/processor.h
 *
 *  Copyright (C) 1995-1999 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARM_PROCESSOR_H
#define __ASM_ARM_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#ifdef __KERNEL__

#include <asm/hw_breakpoint.h>
#include <asm/ptrace.h>
#include <asm/types.h>

#include <l4/sys/types.h>

#ifdef __KERNEL__
#define STACK_TOP	((current->personality & ADDR_LIMIT_32BIT) ? \
			 TASK_SIZE : TASK_SIZE_26)
#define STACK_TOP_MAX	TASK_SIZE
#endif

struct debug_info {
#ifdef CONFIG_HAVE_HW_BREAKPOINT
	struct perf_event	*hbp[ARM_MAX_HBP_SLOTS];
#endif
};

// same in x86...
#ifdef CONFIG_L4_VCPU
#define L4X_THREAD_REGSP(t)  (t)->regsp
#else
#define L4X_THREAD_REGSP(t)  (&(t)->regs)
#endif

struct thread_struct {
							/* fault info	  */
	unsigned long		address;
	unsigned long		trap_no;
	unsigned long		error_code;

#ifndef CONFIG_L4_VCPU
	l4_cap_idx_t user_thread_id;
	l4_cap_idx_t user_thread_ids[8]; //[NR_CPUS];
	l4_cap_idx_t cloner;
	unsigned int  start_cpu;
	unsigned long threads_up;
	unsigned int initial_state_set : 1;
	unsigned int started : 1;
	unsigned int restart : 1;
#endif
	unsigned int is_hybrid : 1;
	unsigned int hybrid_sc_in_prog : 1;
#ifdef CONFIG_L4_VCPU
	l4_cap_idx_t		hyb_user_thread_id;
#endif
#ifdef CONFIG_L4_VCPU
	struct pt_regs          *regsp;
#else
	struct pt_regs regs;
#endif

							/* debugging	  */
	struct debug_info	debug;
};

#define INIT_THREAD  {	}

extern void start_thread(struct pt_regs *regs, unsigned long ip, unsigned long sp);

#ifdef CONFIG_MMU
#define nommu_start_thread(regs) do { } while (0)
#else
#define nommu_start_thread(regs) regs->ARM_r10 = current->mm->start_data
#endif

#define start_thread_arm_orig(regs,pc,sp)					\
({									\
	unsigned long *stack = (unsigned long *)sp;			\
	set_fs(USER_DS);						\
	memset(regs->uregs, 0, sizeof(regs->uregs));			\
	if (current->personality & ADDR_LIMIT_32BIT)			\
		regs->ARM_cpsr = USR_MODE;				\
	else								\
		regs->ARM_cpsr = USR26_MODE;				\
	if (elf_hwcap & HWCAP_THUMB && pc & 1)				\
		regs->ARM_cpsr |= PSR_T_BIT;				\
	regs->ARM_cpsr |= PSR_ENDSTATE;					\
	regs->ARM_pc = pc & ~1;		/* pc */			\
	regs->ARM_sp = sp;		/* sp */			\
	regs->ARM_r2 = stack[2];	/* r2 (envp) */			\
	regs->ARM_r1 = stack[1];	/* r1 (argv) */			\
	regs->ARM_r0 = stack[0];	/* r0 (argc) */			\
	nommu_start_thread(regs);					\
})

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
extern void release_thread(struct task_struct *);

/* Prepare to copy thread state - unlazy all lazy status */
#define prepare_to_copy(tsk)	do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#if __LINUX_ARM_ARCH__ == 6 || defined(CONFIG_ARM_ERRATA_754327)
#define cpu_relax()			smp_mb()
#else
#define cpu_relax()			barrier()
#endif

/*
 * Create a new kernel thread
 */
extern int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

#define task_pt_regs_v(p) \
	((struct pt_regs *)(THREAD_START_SP + task_stack_page(p)) - 1)
#define task_pt_regs(p) \
	(L4X_THREAD_REGSP(&p->thread))

#define KSTK_EIP(tsk)	task_pt_regs(tsk)->ARM_pc
#define KSTK_ESP(tsk)	task_pt_regs(tsk)->ARM_sp

/*
 * Prefetching support - only ARMv5.
 */
#if __LINUX_ARM_ARCH__ >= 5

#define ARCH_HAS_PREFETCH
static inline void prefetch(const void *ptr)
{
	__asm__ __volatile__(
		"pld\t%a0"
		:
		: "p" (ptr)
		: "cc");
}

#define ARCH_HAS_PREFETCHW
#define prefetchw(ptr)	prefetch(ptr)

#define ARCH_HAS_SPINLOCK_PREFETCH
#define spin_lock_prefetch(x) do { } while (0)

#endif

#endif

#endif /* __ASM_ARM_PROCESSOR_H */
