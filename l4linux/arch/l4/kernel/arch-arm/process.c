/*
 *  linux/arch/arm/kernel/process.c
 *
 *  Copyright (C) 1996-2000 Russell King - Converted to ARM.
 *  Original Copyright (C) 1995  Linus Torvalds
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdarg.h>

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/user.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/elfcore.h>
#include <linux/pm.h>
#include <linux/tick.h>
#include <linux/utsname.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/hw_breakpoint.h>

#include <asm/cacheflush.h>
#include <asm/leds.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/thread_notify.h>
#include <asm/stacktrace.h>
#include <asm/mach/time.h>

#include <asm/generic/dispatch.h>
#include <asm/generic/task.h>
#include <asm/generic/sched.h>
#include <asm/generic/stack_id.h>
#include <asm/generic/hybrid.h>

#include <asm/l4lxapi/task.h>

#include <l4/sys/kdebug.h>

#ifdef CONFIG_CC_STACKPROTECTOR
#include <linux/stackprotector.h>
unsigned long __stack_chk_guard __read_mostly;
EXPORT_SYMBOL(__stack_chk_guard);
#endif

static const char *processor_modes[] = {
  "USER_26", "FIQ_26" , "IRQ_26" , "SVC_26" , "UK4_26" , "UK5_26" , "UK6_26" , "UK7_26" ,
  "UK8_26" , "UK9_26" , "UK10_26", "UK11_26", "UK12_26", "UK13_26", "UK14_26", "UK15_26",
  "USER_32", "FIQ_32" , "IRQ_32" , "SVC_32" , "UK4_32" , "UK5_32" , "UK6_32" , "ABT_32" ,
  "UK8_32" , "UK9_32" , "UK10_32", "UND_32" , "UK12_32", "UK13_32", "UK14_32", "SYS_32"
};

static const char *isa_modes[] = {
  "ARM" , "Thumb" , "Jazelle", "ThumbEE"
};

extern void setup_mm_for_reboot(char mode);

int thread_create_user(struct task_struct *p, int fork);

static volatile int hlt_counter;

#include <mach/system.h>

void disable_hlt(void)
{
	hlt_counter++;
}

EXPORT_SYMBOL(disable_hlt);

void enable_hlt(void)
{
	hlt_counter--;
}

EXPORT_SYMBOL(enable_hlt);

static int __init nohlt_setup(char *__unused)
{
	hlt_counter = 1;
	return 1;
}

static int __init hlt_setup(char *__unused)
{
	hlt_counter = 0;
	return 1;
}

__setup("nohlt", nohlt_setup);
__setup("hlt", hlt_setup);

void arm_machine_restart(char mode, const char *cmd)
{
	/* Disable interrupts first */
	local_irq_disable();
	//l4/local_fiq_disable();

	/*
	 * Tell the mm system that we are going to reboot -
	 * we may need it to insert some 1:1 mappings so that
	 * soft boot works.
	 */
	setup_mm_for_reboot(mode);

	/* Clean and invalidate caches */
	flush_cache_all();

	/* Turn off caching */
	cpu_proc_fin();

	/* Push out any further dirty data, and ensure cache is empty */
	flush_cache_all();

	/*
	 * Now call the architecture specific reboot code.
	 */
	arch_reset(mode, cmd);

	/*
	 * Whoops - the architecture was unable to reboot.
	 * Tell the user!
	 */
	mdelay(1000);
	printk("Reboot failed -- System halted\n");
	while (1);
}

/*
 * Function pointers to optional machine specific functions
 */
void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

void (*arm_pm_restart)(char str, const char *cmd) = arm_machine_restart;
EXPORT_SYMBOL_GPL(arm_pm_restart);

static void do_nothing(void *unused)
{
}

/*
 * cpu_idle_wait - Used to ensure that all the CPUs discard old value of
 * pm_idle and update to new pm_idle value. Required while changing pm_idle
 * handler on SMP systems.
 *
 * Caller must have changed pm_idle to the new value before the call. Old
 * pm_idle value will not be used by any CPU after the return of this function.
 */
void cpu_idle_wait(void)
{
	smp_mb();
	/* kick all the CPUs so that they exit out of pm_idle */
	smp_call_function(do_nothing, NULL, 1);
}
EXPORT_SYMBOL_GPL(cpu_idle_wait);

/*
 * This is our default idle handler.  We need to disable
 * interrupts here to ensure we don't miss a wakeup call.
 */
#ifdef CONFIG_L4_VCPU
static void default_idle(void)
{
	if (!need_resched())
		arch_idle();
	local_irq_enable();
}

void (*pm_idle)(void) = default_idle;
EXPORT_SYMBOL(pm_idle);
#endif

/*
 * The idle thread, has rather strange semantics for calling pm_idle,
 * but this is what x86 does and we need to do the same, so that
 * things like cpuidle get called in the same way.  The only difference
 * is that we always respect 'hlt_counter' to prevent low power idle.
 */
void cpu_idle(void)
{
#ifdef CONFIG_L4_VCPU
	local_fiq_enable();

	/* endless idle loop with no priority at all */
	while (1) {
		tick_nohz_stop_sched_tick(1);
		leds_event(led_idle_start);
		while (!need_resched()) {
#ifdef CONFIG_HOTPLUG_CPU
			if (cpu_is_offline(smp_processor_id()))
				cpu_die();
#endif

			local_irq_disable();
			if (hlt_counter) {
				local_irq_enable();
				cpu_relax();
			} else {
				stop_critical_timings();
				pm_idle();
				start_critical_timings();
				/*
				 * This will eventually be removed - pm_idle
				 * functions should always return with IRQs
				 * enabled.
				 */
				WARN_ON(irqs_disabled());
				local_irq_enable();
			}
		}
		leds_event(led_idle_end);
		tick_nohz_restart_sched_tick();
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
#else
	while (1)
		l4x_idle();
#endif
}

static char reboot_mode = 'h';

int __init reboot_setup(char *str)
{
	reboot_mode = str[0];
	return 1;
}

__setup("reboot=", reboot_setup);

void machine_shutdown(void)
{
#ifdef CONFIG_SMP
	smp_send_stop();
#endif
}

void machine_halt(void)
{
	machine_shutdown();
	while (1);
}

void machine_power_off(void)
{
	machine_shutdown();
	if (pm_power_off)
		pm_power_off();
}

void machine_restart(char *cmd)
{
	machine_shutdown();
	arm_pm_restart(reboot_mode, cmd);
}

void __show_regs(struct pt_regs *regs)
{
	unsigned long flags;
	char buf[64];

	printk("CPU: %d    %s  (%s %.*s)\n",
		raw_smp_processor_id(), print_tainted(),
		init_utsname()->release,
		(int)strcspn(init_utsname()->version, " "),
		init_utsname()->version);
	print_symbol("PC is at %s\n", instruction_pointer(regs));
	print_symbol("LR is at %s\n", regs->ARM_lr);
	printk("pc : [<%08lx>]    lr : [<%08lx>]    psr: %08lx\n"
	       "sp : %08lx  ip : %08lx  fp : %08lx\n",
		regs->ARM_pc, regs->ARM_lr, regs->ARM_cpsr,
		regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printk("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9,
		regs->ARM_r8);
	printk("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6,
		regs->ARM_r5, regs->ARM_r4);
	printk("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2,
		regs->ARM_r1, regs->ARM_r0);

	flags = regs->ARM_cpsr;
	buf[0] = flags & PSR_N_BIT ? 'N' : 'n';
	buf[1] = flags & PSR_Z_BIT ? 'Z' : 'z';
	buf[2] = flags & PSR_C_BIT ? 'C' : 'c';
	buf[3] = flags & PSR_V_BIT ? 'V' : 'v';
	buf[4] = '\0';

	printk("Flags: %s  IRQs o%s  FIQs o%s  Mode %s  ISA %s  Segment %s\n",
		buf, interrupts_enabled(regs) ? "n" : "ff",
		fast_interrupts_enabled(regs) ? "n" : "ff",
		processor_modes[processor_mode(regs)],
		isa_modes[isa_mode(regs)],
		get_fs() == get_ds() ? "kernel" : "user");
#if 0
#ifdef CONFIG_CPU_CP15
	{
		unsigned int ctrl;

		buf[0] = '\0';
#ifdef CONFIG_CPU_CP15_MMU
		{
			unsigned int transbase, dac;
			asm("mrc p15, 0, %0, c2, c0\n\t"
			    "mrc p15, 0, %1, c3, c0\n"
			    : "=r" (transbase), "=r" (dac));
			snprintf(buf, sizeof(buf), "  Table: %08x  DAC: %08x",
			  	transbase, dac);
		}
#endif
		asm("mrc p15, 0, %0, c1, c0\n" : "=r" (ctrl));

		printk("Control: %08x%s\n", ctrl, buf);
	}
#endif
#endif
}

void show_regs(struct pt_regs * regs)
{
	printk("\n");
	printk("Pid: %d, comm: %20s\n", task_pid_nr(current), current->comm);
	__show_regs(regs);
	//__backtrace();
}

ATOMIC_NOTIFIER_HEAD(thread_notify_head);

EXPORT_SYMBOL_GPL(thread_notify_head);

/*
 * Free current thread data structures etc..
 */
void exit_thread(void)
{
	thread_notify(THREAD_NOTIFY_EXIT, current_thread_info());
	l4x_exit_thread();
}

void flush_thread(void)
{
	struct thread_info *thread = current_thread_info();
	struct task_struct *tsk = current;
#ifndef CONFIG_L4_VCPU
	int i;
#endif

	flush_ptrace_hw_breakpoint(tsk);

	memset(thread->used_cp, 0, sizeof(thread->used_cp));
	memset(&tsk->thread.debug, 0, sizeof(struct debug_info));
	memset(&thread->fpstate, 0, sizeof(union fp_state));

	thread_notify(THREAD_NOTIFY_FLUSH, thread);

	set_fs(USER_DS);

#ifdef CONFIG_L4_VCPU
	if (!tsk->mm)
		return;
#endif

#ifndef CONFIG_L4_VCPU
	if (!current->thread.started)
		return;
#endif

	current->mm->context.l4x_unmap_mode = L4X_UNMAP_MODE_IMMEDIATELY;

#ifndef CONFIG_L4_VCPU
	for (i = 0; i < NR_CPUS; i++) {
		l4_cap_idx_t thread_id = tsk->thread.user_thread_ids[i];

		if (l4_is_invalid_cap(thread_id))
			continue;

		if (!l4lx_task_delete_thread(thread_id))
			do_exit(9);

		l4lx_task_number_free(thread_id);
#if 0
		if (ret == L4LX_TASK_DELETE_THREAD)
			l4x_hybrid_list_thread_remove(id);
		else {
			l4lx_task_number_free(id);
			l4x_hybrid_list_task_remove(id);
		}
#endif

		current->thread.user_thread_ids[i] = L4_INVALID_CAP;
	}
	current->thread.started = 0;
	current->thread.threads_up = 0;
	current->thread.user_thread_id = L4_INVALID_CAP;
	current->thread.cloner = L4_INVALID_CAP;
#endif
}

void release_thread(struct task_struct *dead_task)
{
}

extern void kernel_thread_helper(void);

#ifndef CONFIG_L4_VCPU
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
		task_thread_info(p)->cpu_context.pc = (unsigned long)kernel_thread_helper;
		return 0;
	}

	/* Fix up stack pointer from copy_thread */
	task_thread_info(p)->cpu_context.sp
	   = (unsigned long)p->stack + THREAD_SIZE;

	return 0;
}
#endif

asmlinkage void ret_from_fork(void) __asm__("ret_from_fork");

int
copy_thread(unsigned long clone_flags, unsigned long stack_start,
	    unsigned long stk_sz___used_for_inkernel_process_flag, struct task_struct *p, struct pt_regs *regs)
{
	struct thread_info *thread = task_thread_info(p);
	struct pt_regs *childregs;

#ifdef CONFIG_L4_VCPU
	p->thread.regsp = task_pt_regs_v(p);
#else
	if (unlikely(stk_sz___used_for_inkernel_process_flag == COPY_THREAD_STACK_SIZE___FLAG_INKERNEL))
		childregs = (void *)thread + THREAD_START_SP - sizeof(*regs);
	else
#endif
		childregs = task_pt_regs(p);
	*childregs = *regs;
	childregs->ARM_r0 = 0;
	childregs->ARM_sp = stack_start;

	memset(&thread->cpu_context, 0, sizeof(struct cpu_context_save));
	thread->cpu_context.sp = (unsigned long)childregs;
	thread->cpu_context.pc = (unsigned long)ret_from_fork;

	clear_ptrace_hw_breakpoint(p);

	if (clone_flags & CLONE_SETTLS)
		thread->tp_value = regs->ARM_r3;

	thread_notify(THREAD_NOTIFY_COPY, thread);

#ifdef CONFIG_L4_VCPU
	thread->cpu_context.extra[0] = (unsigned long) (childregs+0);
	l4x_stack_setup(p->stack, l4_utcb(),
	                ((struct thread_info *)p->stack)->cpu);
	return 0;
#else
	/* create the user task */
	return l4x_thread_create(p, clone_flags, stk_sz___used_for_inkernel_process_flag == COPY_THREAD_STACK_SIZE___FLAG_INKERNEL);
#endif
}

/*
 * Fill in the task's elfregs structure for a core dump.
 */
int dump_task_regs(struct task_struct *t, elf_gregset_t *elfregs)
{
	elf_core_copy_regs(elfregs, task_pt_regs(t));
	return 1;
}

/*
 * fill in the fpe structure for a core dump...
 */
int dump_fpu (struct pt_regs *regs, struct user_fp *fp)
{
	struct thread_info *thread = current_thread_info();
	int used_math = thread->used_cp[1] | thread->used_cp[2];

	if (used_math)
		memcpy(fp, &thread->fpstate.soft, sizeof (*fp));

	return used_math != 0;
}
EXPORT_SYMBOL(dump_fpu);

/*
 * Shuffle the argument into the correct register before calling the
 * thread function.  r4 is the thread argument, r5 is the pointer to
 * the thread function, and r6 points to the exit function.
 */

asm(	".pushsection .text\n"
"	.align\n"
"	.type	kernel_thread_helper, #function\n"
"kernel_thread_helper:\n"
#ifndef CONFIG_L4_VCPU
"	bl	schedule_tail\n" // (everything) questionable??
"	ldmia	sp, {r0 - r9}\n"
// XXX: Correct stack pointer?
//"	add	sp, sp, 12..\n"
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
"	bl	trace_hardirqs_on\n"
#endif
"	msr	cpsr_c, r7\n"
"	mov	r0, r4\n"
"	mov	lr, r6\n"
"	mov	pc, r5\n"
"	.size	kernel_thread_helper, . - kernel_thread_helper\n"
"	.popsection");

#ifdef CONFIG_ARM_UNWIND
extern void kernel_thread_exit(long code);
asm(	".pushsection .text\n"
"	.align\n"
"	.type	kernel_thread_exit, #function\n"
"kernel_thread_exit:\n"
"	.fnstart\n"
"	.cantunwind\n"
"	bl	do_exit\n"
"	nop\n"
"	.fnend\n"
"	.size	kernel_thread_exit, . - kernel_thread_exit\n"
"	.popsection");
#else
#define kernel_thread_exit	do_exit
#endif

/*
 * Create a kernel thread.
 */
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));

	regs.ARM_r4 = (unsigned long)arg;
	regs.ARM_r5 = (unsigned long)fn;
	regs.ARM_r6 = (unsigned long)kernel_thread_exit;
	regs.ARM_r7 = SVC_MODE | PSR_ENDSTATE | PSR_ISETSTATE;
#ifdef CONFIG_L4_VCPU
	regs.ARM_pc = (unsigned long)kernel_thread_helper;
	regs.ARM_cpsr = regs.ARM_r7 | PSR_I_BIT;
#endif

	return do_fork(flags|CLONE_VM|CLONE_UNTRACED, 0, &regs, COPY_THREAD_STACK_SIZE___FLAG_INKERNEL, NULL, NULL);
}
EXPORT_SYMBOL(kernel_thread);

void start_thread(struct pt_regs *regs, unsigned long pc,
		  unsigned long sp)
{
	//printk("%s %d regs=%p pc%lx sp%lx\n", __func__, __LINE__, regs, pc, sp); 

	unsigned long *stack = (unsigned long *)sp;
	set_fs(USER_DS);
	memset(regs->uregs, 0, sizeof(regs->uregs));
	regs->ARM_cpsr = USR_MODE;
	if (elf_hwcap & HWCAP_THUMB && pc & 1)
		regs->ARM_cpsr |= PSR_T_BIT;
	regs->ARM_cpsr |= PSR_ENDSTATE;
	regs->ARM_pc   = pc & ~1;
	regs->ARM_sp   = sp;
	//regs->ARM_r2 = stack[2];
	//regs->ARM_r1 = stack[1];
	//regs->ARM_r0 = stack[0];
	get_user(regs->ARM_r2, &stack[2]);
	get_user(regs->ARM_r1, &stack[1]);
	get_user(regs->ARM_r0, &stack[0]);

#ifndef CONFIG_L4_VCPU
	current->thread.restart = 1;
#endif

	if (pc > TASK_SIZE)
		force_sig(SIGSEGV, current);
}
EXPORT_SYMBOL(start_thread);

unsigned long get_wchan(struct task_struct *p)
{
	struct stackframe frame;
	int count = 0;
	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;

	frame.fp = thread_saved_fp(p);
	frame.sp = thread_saved_sp(p);
	frame.lr = 0;			/* recovered from the stack */
	frame.pc = thread_saved_pc(p);
	do {
		int ret = unwind_frame(&frame);
		if (ret < 0)
			return 0;
		if (!in_sched_functions(frame.pc))
			return frame.pc;
	} while (count ++ < 16);
	return 0;
}

unsigned long arch_randomize_brk(struct mm_struct *mm)
{
	unsigned long range_end = mm->brk + 0x02000000;
	return randomize_range(mm->brk, range_end, 0) ? : mm->brk;
}

#ifdef CONFIG_MMU
/*
 * The vectors page is always readable from user space for the
 * atomic helpers and the signal restart code.  Let's declare a mapping
 * for it so it is visible through ptrace and /proc/<pid>/mem.
 */

int vectors_user_mapping(void)
{
	struct mm_struct *mm = current->mm;
	return install_special_mapping(mm, 0xffff0000, PAGE_SIZE,
				       VM_READ | VM_EXEC |
				       VM_MAYREAD | VM_MAYEXEC |
				       VM_ALWAYSDUMP | VM_RESERVED,
				       NULL);
}

const char *arch_vma_name(struct vm_area_struct *vma)
{
	return (vma->vm_start == 0xffff0000) ? "[vectors]" : NULL;
}
#endif
