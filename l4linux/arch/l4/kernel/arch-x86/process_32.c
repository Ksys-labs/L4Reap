/*
 *  Copyright (C) 1995  Linus Torvalds
 *
 *  Pentium III FXSR, SSE support
 *	Gareth Hughes <gareth@valinux.com>, May 2000
 */

/*
 * This file handles the architecture-dependent parts of process handling..
 */

#include <linux/stackprotector.h>
#include <linux/cpu.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/elfcore.h>
#include <linux/smp.h>
#include <linux/stddef.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/user.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/mc146818rtc.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/ptrace.h>
#include <linux/personality.h>
#include <linux/tick.h>
#include <linux/percpu.h>
#include <linux/prctl.h>
#include <linux/ftrace.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/kdebug.h>

#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/ldt.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/desc.h>
#ifdef CONFIG_MATH_EMULATION
#include <asm/math_emu.h>
#endif

#include <linux/err.h>

#include <asm/tlbflush.h>
#include <asm/cpu.h>
#include <asm/idle.h>
#include <asm/syscalls.h>
#include <asm/debugreg.h>

#include <asm/api/macros.h>

#include <asm/generic/sched.h>
#include <asm/generic/dispatch.h>
#include <asm/generic/upage.h>
#include <asm/generic/assert.h>
#include <asm/generic/task.h>
#include <asm/generic/stack_id.h>

#include <asm/l4lxapi/task.h>

/*
 * Return saved PC of a blocked thread.
 */
unsigned long thread_saved_pc(struct task_struct *tsk)
{
	return ((unsigned long *)tsk->thread.sp)[3];
}

#ifndef CONFIG_SMP
static inline void play_dead(void)
{
	BUG();
}
#endif

/*
 * The idle thread. There's no useful work to be
 * done, so just try to conserve power and have a
 * low exit latency (ie sit in a loop waiting for
 * somebody to say that they'd like to reschedule)
 */
void cpu_idle(void)
{
#ifdef CONFIG_L4_VCPU
	int cpu = smp_processor_id();

	/*
	 * If we're the non-boot CPU, nothing set the stack canary up
	 * for us.  CPU0 already has it initialized but no harm in
	 * doing it again.  This is a good place for updating it, as
	 * we wont ever return from this function (so the invalid
	 * canaries already on the stack wont ever trigger).
	 */
	boot_init_stack_canary();

	current_thread_info()->status |= TS_POLLING;

	/* endless idle loop with no priority at all */
	while (1) {
		tick_nohz_stop_sched_tick(1);
		while (!need_resched()) {

			check_pgt_cache();
			rmb();

			if (cpu_is_offline(cpu))
				play_dead();

			local_irq_disable();
			/* Don't trace irqs off for idle */
			stop_critical_timings();
			pm_idle();
			start_critical_timings();
		}
		tick_nohz_restart_sched_tick();
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
#else
	for (;;)
		l4x_idle();
#endif
}

void __show_regs(struct pt_regs *regs, int all)
{
#ifdef NOT_FOR_L4
	unsigned long cr0 = 0L, cr2 = 0L, cr3 = 0L, cr4 = 0L;
	unsigned long d0, d1, d2, d3, d6, d7;
#endif
	unsigned long sp;
	unsigned short ss, gs;

	if (user_mode_vm(regs)) {
		sp = regs->sp;
		ss = regs->ss & 0xffff;
		gs = get_user_gs(regs);
	} else {
		sp = kernel_stack_pointer(regs);
		savesegment(ss, ss);
		savesegment(gs, gs);
	}

	show_regs_common();

	printk(KERN_DEFAULT "EIP: %04x:[<%08lx>] EFLAGS: %08lx CPU: %d\n",
			(u16)regs->cs, regs->ip, regs->flags,
			smp_processor_id());
	print_symbol("EIP is at %s\n", regs->ip);

	printk(KERN_DEFAULT "EAX: %08lx EBX: %08lx ECX: %08lx EDX: %08lx\n",
		regs->ax, regs->bx, regs->cx, regs->dx);
	printk(KERN_DEFAULT "ESI: %08lx EDI: %08lx EBP: %08lx ESP: %08lx\n",
		regs->si, regs->di, regs->bp, sp);
	printk(KERN_DEFAULT " DS: %04x ES: %04x FS: %04x GS: %04x SS: %04x\n",
	       (u16)regs->ds, (u16)regs->es, (u16)regs->fs, gs, ss);

	if (!all)
		return;

#ifdef NOT_FOR_L4
	cr0 = read_cr0();
	cr2 = read_cr2();
	cr3 = read_cr3();
	cr4 = read_cr4_safe();
	printk(KERN_DEFAULT "CR0: %08lx CR2: %08lx CR3: %08lx CR4: %08lx\n",
			cr0, cr2, cr3, cr4);

	get_debugreg(d0, 0);
	get_debugreg(d1, 1);
	get_debugreg(d2, 2);
	get_debugreg(d3, 3);
	printk(KERN_DEFAULT "DR0: %08lx DR1: %08lx DR2: %08lx DR3: %08lx\n",
			d0, d1, d2, d3);

	get_debugreg(d6, 6);
	get_debugreg(d7, 7);
	printk(KERN_DEFAULT "DR6: %08lx DR7: %08lx\n",
			d6, d7);
#endif
}

#if 0
/*
 * Create a kernel thread
 */
int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	struct pt_regs regs;
#ifdef CONFIG_L4_VCPU
	unsigned ds, cs;
	asm volatile ("mov %%ds, %0; mov %%cs, %1\n" : "=r"(ds), "=r"(cs));
#endif

	memset(&regs, 0, sizeof(regs));

	regs.bx = (unsigned long) fn;
	regs.dx = (unsigned long) arg;

#ifdef CONFIG_L4_VCPU
	regs.ds = ds;
	regs.ds = ds;
	regs.es = ds;
#ifdef CONFIG_SMP
	regs.fs = (l4x_fiasco_gdt_entry_offset + 2) * 8 + 3;
#else
	regs.fs = ds;
#endif
#else
	regs.ds = __USER_DS;
	regs.es = __USER_DS;
	regs.fs = __KERNEL_PERCPU;
	regs.gs = __KERNEL_STACK_CANARY;
#endif
	regs.orig_ax = -1;
#ifdef CONFIG_L4_VCPU
	regs.ip = (unsigned long) kernel_thread_helper;
	regs.cs = (cs & ~3) | get_kernel_rpl();
#else
	regs.cs = __KERNEL_CS | get_kernel_rpl();
#endif
	regs.flags = X86_EFLAGS_IF | X86_EFLAGS_SF | X86_EFLAGS_PF | 0x2;

	/* Ok, create the new process.. */
	return do_fork(flags | CLONE_VM | CLONE_UNTRACED, 0, &regs, COPY_THREAD_STACK_SIZE___FLAG_INKERNEL, NULL, NULL);
}
EXPORT_SYMBOL(kernel_thread);
#endif

void release_thread(struct task_struct *dead_task)
{
	//outstring("release_thread\n");
	//printk("%s %d(%s)\n", __func__, current->pid, current->comm);
}

/* defined in kernel/sched.c -- other archs only use this in ASM */
asmlinkage void schedule_tail(struct task_struct *prev);

#ifndef CONFIG_L4_VCPU
/* helpers for copy_thread() */
void ret_kernel_thread_start(void);

asm(".section .text\n"
    ".align 4\n"
    "ret_kernel_thread_start: \n\t"
    "call kernel_thread_start \n\t"
    ".previous");

void kernel_thread_start(struct task_struct *p)
{
	struct pt_regs *r = &current->thread.regs;
	int (*func)(void *) = (void *)r->si;

	schedule_tail(p);
	do_exit(func((void *)r->di));
}

/*
 * Create the kernel context for a new process.  Our main duty here is
 * to fill in p->thread, the arch-specific part of the process'
 * task_struct */
static int l4x_thread_create(struct task_struct *p, unsigned long clone_flags,
                             int inkernel)
{
	struct thread_struct *t = &p->thread;
	int i;

	/* first, allocate task id for  client task */
	if (!inkernel && clone_flags & CLONE_VM) /* is this a user process and vm-cloned? */
		t->cloner = current->mm->context.task;
	else
		t->cloner = L4_INVALID_CAP;

	for (i = 0; i < NR_CPUS; i++)
		p->thread.user_thread_ids[i] = L4_INVALID_CAP;
	p->thread.user_thread_id = L4_INVALID_CAP;
	p->thread.threads_up = 0;

	/* put thread id in stack */
	l4x_stack_setup(p->stack, l4_utcb(), 0);

	/* if creating a kernel-internal thread, return at this point */
	if (inkernel) {
		/* compute pointer to end of stack */
		unsigned long *sp = (unsigned long *)
		       ((unsigned long)p->stack + sizeof(union thread_union));
		/* switch_to will expect the new program pointer
		 * on the stack */
		*(--sp) = (unsigned long) ret_kernel_thread_start;

		t->sp = (unsigned long) sp;
		return 0;
	}

	l4x_setup_user_dispatcher_after_fork(p);
	return 0;
}
#endif /* !vcpu */

/*
 * This gets called before we allocate a new thread and copy
 * the current task into it.
 */
void prepare_to_copy(struct task_struct *tsk)
{
	unlazy_fpu(tsk);
}

int copy_thread(unsigned long clone_flags, unsigned long sp,
	unsigned long stack_size___used_for_inkernel_process_flag,
	struct task_struct *p, struct pt_regs *regs)
{
	struct pt_regs *childregs;
	struct task_struct *tsk = current;
	int err;

#ifdef CONFIG_L4_VCPU
	asmlinkage void ret_from_fork(void);
	p->thread.regsp = task_pt_regs_v(p);
#endif

	childregs = task_pt_regs(p);
	*childregs = *regs;
	childregs->ax = 0;
	childregs->sp = sp;

	childregs->flags |= 0x200;	/* sanity: set EI flag */
	childregs->flags &= 0x1ffff;

#ifdef CONFIG_L4_VCPU
	p->thread.sp = (unsigned long) childregs;
	p->thread.sp0 = (unsigned long) (childregs+0);

	p->thread.sp -= sizeof(long);
	*(unsigned long *)p->thread.sp = 0;
	p->thread.sp -= sizeof(long);
	*(unsigned long *)p->thread.sp = (unsigned long)&ret_from_fork;
#endif
	//p->thread.ip = (unsigned long) ret_from_fork;

	//task_user_gs(p) = get_user_gs(regs);

	p->thread.io_bitmap_ptr = NULL;
	tsk = current;
	err = -ENOMEM;

	/* Copy segment registers */
	p->thread.gs = tsk->thread.gs;

	memset(p->thread.ptrace_bps, 0, sizeof(p->thread.ptrace_bps));

	if (unlikely(test_tsk_thread_flag(tsk, TIF_IO_BITMAP))) {
		p->thread.io_bitmap_ptr = kmemdup(tsk->thread.io_bitmap_ptr,
						IO_BITMAP_BYTES, GFP_KERNEL);
		if (!p->thread.io_bitmap_ptr) {
			p->thread.io_bitmap_max = 0;
			return -ENOMEM;
		}
		set_tsk_thread_flag(p, TIF_IO_BITMAP);
	}

	err = 0;

	/*
	 * Set a new TLS for the child thread?
	 */
	if (clone_flags & CLONE_SETTLS)
		err = do_set_thread_area(p, -1,
			(struct user_desc __user *)childregs->si, 0);

	if (err && p->thread.io_bitmap_ptr) {
		kfree(p->thread.io_bitmap_ptr);
		p->thread.io_bitmap_max = 0;
	}

#ifdef CONFIG_L4_VCPU
	if (!err)
		l4x_stack_setup(p->stack, l4_utcb(),
		                ((struct thread_info *)p->stack)->cpu);
#else
	/* create the user task */
	if (!err)
		err = l4x_thread_create(p, clone_flags, stack_size___used_for_inkernel_process_flag == COPY_THREAD_STACK_SIZE___FLAG_INKERNEL);
#endif
	return err;
}

void
start_thread(struct pt_regs *regs, unsigned long new_ip, unsigned long new_sp)
{
#ifdef CONFIG_L4_VCPU
	unsigned cs, ds;
	__asm__("mov %%cs, %0; mov %%ds, %1 \n" : "=r"(cs), "=r"(ds) );
#endif

	//set_user_gs(regs, 0);
	regs->fs		= 0;
#ifdef CONFIG_L4_VCPU
	regs->ds                = ds;
	regs->es                = ds;
	regs->ss                = ds;
	regs->cs                = cs;
#else
	//regs->ds		= __USER_DS;
	//regs->es		= __USER_DS;
	//regs->ss		= __USER_DS;
	//regs->cs		= __USER_CS;
#endif
	regs->ip		= new_ip;
	regs->sp		= new_sp;
	/*
	 * Free the old FP and other extended state
	 */
	free_thread_xstate(current);

	current->thread.gs = 0;

#ifndef CONFIG_L4_VCPU
	current->thread.restart = 1;
#endif

	if (new_ip > TASK_SIZE)
		force_sig(SIGSEGV, current);
}
EXPORT_SYMBOL(start_thread);

#ifndef CONFIG_L4_VCPU
/* kernel-internal execve() */
asmlinkage int
l4_kernelinternal_execve(const char * file,
                         const char * const * argv,
                         const char * const * envp)
{
	int ret;
	struct thread_struct *t = &current->thread;

	ASSERT(l4_is_invalid_cap(t->user_thread_id));

	/* we are going to become a real user task now, so prepare a real
	 * pt_regs structure. */
	/* Enable Interrupts, Set IOPL (needed for X, hwclock etc.) */
	t->regs.flags = 0x3200; /* XXX hardcoded */

	/* do_execve() will create the user task for us in start_thread()
	   and call set_fs(USER_DS) in flush_thread. I know this sounds
	   strange but there are places in the kernel (kernel/kmod.c) which
	   call execve with parameters inside the kernel. They set fs to
	   KERNEL_DS before calling execve so we can't set it back to
	   USER_DS before execve had a chance to look at the name of the
	   executable. */

	ASSERT(segment_eq(get_fs(), KERNEL_DS));
	ret = do_execve(file, argv, envp, &t->regs);

	if (ret < 0) {
		/* we failed -- become a kernel thread again */
		if (!l4_is_invalid_cap(t->user_thread_id))
			l4lx_task_number_free(t->user_thread_id);
		set_fs(KERNEL_DS);
		t->user_thread_id = L4_INVALID_CAP;
		return -1;
	}

	l4x_user_dispatcher();

	/* not reached */
	return 0;
}
#endif

#define top_esp                (THREAD_SIZE - sizeof(unsigned long))
#define top_ebp                (THREAD_SIZE - 2*sizeof(unsigned long))

unsigned long get_wchan(struct task_struct *p)
{
	unsigned long bp, sp, ip;
	unsigned long stack_page;
	int count = 0;
	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;
	stack_page = (unsigned long)task_stack_page(p);
	sp = p->thread.sp;
	if (!stack_page || sp < stack_page || sp > top_esp+stack_page)
		return 0;

	/* L4Linux has a different layout in switch_to(), but
	 *  the only difference is that we push a return
	 *  address after ebp. So we simply adjust the esp to
	 *  reflect that. And we leave the different name for
	 *  esp to catch direct usage of thread data. */

	sp += 4;/* add 4 to remove return address */

	/* include/asm-i386/system.h:switch_to() pushes bp last. */
	bp = *(unsigned long *) sp;
	do {
		if (bp < stack_page || bp > top_ebp+stack_page)
			return 0;
		ip = *(unsigned long *) (bp+4);
		if (!in_sched_functions(ip))
			return ip;
		bp = *(unsigned long *) bp;
	} while (count++ < 16);
	return 0;
}

