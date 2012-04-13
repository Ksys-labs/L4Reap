
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/tick.h>
#include <linux/kprobes.h>

#include <asm/processor.h>
#include <asm/mmu_context.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <asm/i387.h>
#include <asm/traps.h>

#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/utcb.h>
#include <l4/sys/segment.h>
#include <l4/sys/ktrace.h>
#include <l4/util/util.h>
#include <l4/log/log.h>

#include <asm/l4lxapi/task.h>
#include <asm/l4lxapi/thread.h>
#include <asm/l4lxapi/memory.h>
#include <asm/api/macros.h>

#include <asm/generic/dispatch.h>
#include <asm/generic/ferret.h>
#include <asm/generic/task.h>
#include <asm/generic/upage.h>
#include <asm/generic/memory.h>
#include <asm/generic/process.h>
#include <asm/generic/setup.h>
#include <asm/generic/ioremap.h>
#include <asm/generic/hybrid.h>
#include <asm/generic/syscall_guard.h>
#include <asm/generic/stats.h>
#include <asm/generic/smp.h>

#include <asm/l4x/exception.h>
#include <asm/l4x/fpu.h>
#include <asm/l4x/l4_syscalls.h>
#include <asm/l4x/lx_syscalls.h>
#include <asm/l4x/utcb.h>
#include <asm/l4x/signal.h>

#if 1
#define TBUF_LOG_IDLE(x)        TBUF_DO_IT(x)
#define TBUF_LOG_WAKEUP_IDLE(x)	TBUF_DO_IT(x)
#define TBUF_LOG_USER_PF(x)     TBUF_DO_IT(x)
#define TBUF_LOG_INT80(x)       TBUF_DO_IT(x)
#define TBUF_LOG_EXCP(x)        TBUF_DO_IT(x)
#define TBUF_LOG_START(x)       TBUF_DO_IT(x)
#define TBUF_LOG_SUSP_PUSH(x)   TBUF_DO_IT(x)
#define TBUF_LOG_DSP_IPC_IN(x)  TBUF_DO_IT(x)
#define TBUF_LOG_DSP_IPC_OUT(x) TBUF_DO_IT(x)
#define TBUF_LOG_SUSPEND(x)     TBUF_DO_IT(x)
#define TBUF_LOG_SWITCH(x)      TBUF_DO_IT(x)
#define TBUF_LOG_HYB_BEGIN(x)   TBUF_DO_IT(x)
#define TBUF_LOG_HYB_RETURN(x)  TBUF_DO_IT(x)

#else

#define TBUF_LOG_IDLE(x)
#define TBUF_LOG_WAKEUP_IDLE(x)
#define TBUF_LOG_USER_PF(x)
#define TBUF_LOG_INT80(x)
#define TBUF_LOG_EXCP(x)
#define TBUF_LOG_START(x)
#define TBUF_LOG_SUSP_PUSH(x)
#define TBUF_LOG_DSP_IPC_IN(x)
#define TBUF_LOG_DSP_IPC_OUT(x)
#define TBUF_LOG_SUSPEND(x)
#define TBUF_LOG_SWITCH(x)
#define TBUF_LOG_HYB_BEGIN(x)
#define TBUF_LOG_HYB_RETURN(x)

#endif

static DEFINE_PER_CPU(struct l4x_arch_cpu_fpu_state, l4x_cpu_fpu_state);

void l4x_fpu_set(int on_off)
{
	per_cpu(l4x_cpu_fpu_state, smp_processor_id()).enabled = on_off;
}

struct l4x_arch_cpu_fpu_state *l4x_fpu_get(unsigned cpu)
{
	return &per_cpu(l4x_cpu_fpu_state, cpu);
}

static inline int l4x_msgtag_fpu(unsigned cpu)
{
	return l4x_fpu_get(cpu)->enabled
	       ?  L4_MSGTAG_TRANSFER_FPU : 0;
}

static inline int l4x_msgtag_copy_ureg(l4_utcb_t *u)
{
	return 0;
}

static inline int l4x_is_triggered_exception(l4_umword_t val)
{
	return val == 0xff;
}

static inline unsigned long regs_pc(struct thread_struct *t)
{
	return L4X_THREAD_REGSP(t)->ip;
}

static inline unsigned long regs_sp(struct thread_struct *t)
{
	return L4X_THREAD_REGSP(t)->sp;
}

static inline void l4x_arch_task_setup(struct thread_struct *t)
{
#ifdef CONFIG_X86_32
	load_TLS(t, 0);
#endif
}

static inline void l4x_arch_do_syscall_trace(struct task_struct *p,
                                             struct thread_struct *t)
{
	if (unlikely(current_thread_info()->flags & _TIF_WORK_SYSCALL_EXIT))
		syscall_trace_leave(L4X_THREAD_REGSP(t));
}

static inline int l4x_hybrid_check_after_syscall(l4_utcb_t *utcb)
{
	l4_exc_regs_t *exc = l4_utcb_exc_u(utcb);
	return (exc->trapno == 0xd /* after L4 syscall */
	        && l4x_l4syscall_get_nr(exc->err, exc->ip) != -1
	        && (exc->err & 4))
	       || (exc->trapno == 0xff /* L4 syscall exr'd */
	           && exc->err == 0);
}

static inline void l4x_dispatch_delete_polling_flag(void)
{
	current_thread_info()->status &= ~TS_POLLING;
}

static inline void l4x_dispatch_set_polling_flag(void)
{
	current_thread_info()->status |= TS_POLLING;
}

#ifdef CONFIG_L4_VCPU
static inline void l4x_arch_task_start_setup(l4_vcpu_state_t *v,
		                             struct task_struct *p)
#else
static inline void l4x_arch_task_start_setup(struct task_struct *p)
#endif
{
	// - remember GS in FS so that programs can find their UTCB
	//   libl4sys-l4x.a uses %fs to get the UTCB address
	// - do not set GS because glibc does not seem to like if gs is not 0
	// - only do this if this is the first usage of the L4 thread in
	//   this task, otherwise gs will have the glibc-gs
	// - ensure this by checking if the segment is one of the user ones or
	//   another one (then it's the utcb one)
#ifdef CONFIG_X86_32
#ifdef CONFIG_L4_VCPU
	unsigned int gs = l4x_vcpu_state(smp_processor_id())->r.gs;
#else
	unsigned int gs = l4_utcb_exc()->gs;
#endif
	unsigned int val = (gs & 0xffff) >> 3;
	if (   val < l4x_fiasco_gdt_entry_offset
	    || val > l4x_fiasco_gdt_entry_offset + 3)
		L4X_THREAD_REGSP(&p->thread)->fs = gs;

	/* Setup LDTs */
	if (p->mm && p->mm->context.size
#ifdef CONFIG_L4_VCPU
	    && !l4_is_invalid_cap(p->mm->context.task)
#endif
	    ) {
		unsigned i;
		L4XV_V(f);
		L4XV_L(f);
		for (i = 0; i < p->mm->context.size;
		     i += L4_TASK_LDT_X86_MAX_ENTRIES) {
			unsigned sz = p->mm->context.size - i;
			int r;
			if (sz > L4_TASK_LDT_X86_MAX_ENTRIES)
				sz = L4_TASK_LDT_X86_MAX_ENTRIES;
			r = fiasco_ldt_set(p->mm->context.task, p->mm->context.ldt,
			                   sz, i, l4_utcb());
			if (r)
				LOG_printf("fiasco_ldt_set(%d, %d): failed %d\n",
				           sz, i, r);

		}
		L4XV_U(f);
	}
#endif
}

static inline l4_umword_t l4x_l4pfa(struct thread_struct *t)
{
	return (t->pfa & ~3) | (t->error_code & 2);
}

static inline int l4x_ispf(struct thread_struct *t)
{
	return t->trap_no == 14;
}

static inline void l4x_print_regs(struct thread_struct *t, struct pt_regs *r)
{
	printk("ip: %08lx sp: %08lx err: %08lx trp: %08lx\n",
	       r->ip, r->sp, t->error_code, t->trap_no);
	printk("ax: %08lx bx: %08lx  cx: %08lx  dx: %08lx\n",
	       r->ax, r->bx, r->cx, r->dx);
#ifdef CONFIG_X86_32
	printk("di: %08lx si: %08lx  bp: %08lx  gs: %08lx fs: %08lx\n",
	       r->di, r->si, r->bp, r->gs, r->fs);
#else
	printk("di: %08lx si: %08lx  bp: %08lx\n",
	       r->di, r->si, r->bp);
#endif
}

asmlinkage void ret_from_fork(void) __asm__("ret_from_fork");
asm(
".section .text			\n"
#ifdef CONFIG_L4_VCPU
".global ret_from_fork          \n"
#endif
"ret_from_fork:			\n"
// why do we need the push/pop?
#ifdef CONFIG_X86_32
"pushl	%ebx			\n"
#endif
"call	schedule_tail		\n"
#ifdef CONFIG_X86_32
"popl	%ebx			\n"
#endif
#ifdef CONFIG_L4_VCPU
"jmp	l4x_vcpu_ret_from_fork  \n"
#else
"jmp	l4x_user_dispatcher	\n"
#endif
".previous			\n"
);

#ifndef CONFIG_L4_VCPU
void l4x_idle(void);
#endif

int  l4x_deliver_signal(int exception_nr, int error_code);

DEFINE_PER_CPU(struct thread_info *, l4x_current_ti) = &init_thread_info;
DEFINE_PER_CPU(struct thread_info *, l4x_current_proc_run);
#ifndef CONFIG_L4_VCPU
static DEFINE_PER_CPU(unsigned, utcb_snd_size);
#endif

static void l4x_setup_next_exec(struct task_struct *p, unsigned long f)
{
	unsigned long *sp = (unsigned long *)
	                     ((unsigned long)p->stack + THREAD_SIZE);

	BUG_ON(current == p);

	/* setup stack of p to come out in f on next switch_to() */
	*--sp = 0;
	*--sp = f;

	p->thread.sp = (unsigned long)sp;
}

void l4x_setup_user_dispatcher_after_fork(struct task_struct *p)
{
	l4x_setup_next_exec(p, (unsigned long)ret_from_fork);
}

#include <asm/generic/stack_id.h>

void l4x_switch_to(struct task_struct *prev, struct task_struct *next)
{
#ifdef CONFIG_L4_VCPU
	l4_vcpu_state_t *vcpu = l4x_vcpu_state(smp_processor_id());
#endif
#if 0
	LOG_printf("%s: cpu%d: %s(%d)[%ld] -> %s(%d)[%ld]\n",
	           __func__, smp_processor_id(),
	           prev->comm, prev->pid, prev->state,
	           next->comm, next->pid, next->state);
#endif
#ifdef CONFIG_L4_VCPU
	TBUF_LOG_SWITCH(fiasco_tbuf_log_3val("SWITCH", (unsigned long)prev->stack, (unsigned long)next->stack, next->thread.sp0));
#else
	TBUF_LOG_SWITCH(fiasco_tbuf_log_3val("SWITCH", (prev->pid << 16) | TBUF_TID(prev->thread.user_thread_id), (next->pid << 16) | TBUF_TID(next->thread.user_thread_id), 0));
#endif

	__unlazy_fpu(prev);
#ifndef CONFIG_L4_VCPU
	per_cpu(l4x_current_ti, smp_processor_id())
	  = (struct thread_info *)((unsigned long)next->stack & ~(THREAD_SIZE - 1));
#endif

	if (unlikely(task_thread_info(prev)->flags & _TIF_WORK_CTXSW_PREV ||
	             task_thread_info(next)->flags & _TIF_WORK_CTXSW_NEXT))
		__switch_to_xtra(prev, next, NULL);


	percpu_write(current_task, next);

#ifdef CONFIG_SMP
#ifndef CONFIG_L4_VCPU
	next->thread.user_thread_id = next->thread.user_thread_ids[smp_processor_id()];
#else
	/* Migrated thread? */
	l4x_stack_struct_get(next->stack)->vcpu = vcpu;
#endif
	l4x_stack_struct_get(next->stack)->l4utcb
	  = l4x_stack_struct_get(prev->stack)->l4utcb;
#endif

#ifdef CONFIG_L4_VCPU
	vcpu->entry_sp = next->thread.sp0;
	if (next->mm && prev->mm != next->mm)
		vcpu->user_task = next->mm->context.task;
#endif

#ifdef CONFIG_X86_32
#ifdef CONFIG_L4_VCPU
	if (next->mm)
#else
	if (next->mm && !l4_is_invalid_cap(next->mm->context.task))
#endif
		load_TLS(&next->thread, 0);
#endif
}

static inline void l4x_pte_add_access_and_mapped(pte_t *ptep)
{
	ptep->pte |= (_PAGE_ACCESSED + _PAGE_MAPPED);
}

static inline void l4x_pte_add_access_mapped_and_dirty(pte_t *ptep)
{
	ptep->pte |= (_PAGE_ACCESSED + _PAGE_DIRTY + _PAGE_MAPPED);
}

#ifdef CONFIG_L4_VCPU
static inline void vcpu_to_thread_struct(l4_vcpu_state_t *v,
                                         struct thread_struct *t)
{
#ifdef CONFIG_X86_32
	t->gs         = v->r.gs;
#endif
	t->trap_no    = v->r.trapno;
	t->error_code = v->r.err;
	t->pfa        = v->r.pfa;
}

static inline void thread_struct_to_vcpu(l4_vcpu_state_t *v,
                                         struct thread_struct *t)
{
#ifdef CONFIG_X86_32
	v->r.gs = t->gs;
#endif
}

#else
static inline void utcb_to_thread_struct(l4_utcb_t *utcb,
                                         struct thread_struct *t)
{
	l4_exc_regs_t *exc = l4_utcb_exc_u(utcb);
	utcb_exc_to_ptregs(exc, L4X_THREAD_REGSP(t));
	t->gs         = exc->gs;
	t->trap_no    = exc->trapno;
	t->error_code = exc->err;
	t->pfa        = exc->pfa;
}

static inline void thread_struct_to_utcb(struct thread_struct *t,
                                         l4_utcb_t *utcb,
                                         unsigned int send_size)
{
	l4_exc_regs_t *exc = l4_utcb_exc_u(utcb);
	ptregs_to_utcb_exc(L4X_THREAD_REGSP(t), exc);
	exc->gs   = t->gs;
#ifndef CONFIG_L4_VCPU
	per_cpu(utcb_snd_size, smp_processor_id()) = send_size;
#endif
}
#endif

#ifndef CONFIG_L4_VCPU
static int l4x_hybrid_begin(struct task_struct *p,
                            struct thread_struct *t);

static void l4x_dispatch_suspend(struct task_struct *p,
                                 struct thread_struct *t);
#endif

static inline void dispatch_system_call(struct task_struct *p,
                                        struct pt_regs *regsp)
{
	unsigned int syscall;
	syscall_t syscall_fn = NULL;

#ifdef CONFIG_L4_VCPU
	local_irq_enable();
	p->thread.regsp = regsp;
#endif
	//printk("dispatch_system_call\n");

	//syscall_count++;

	regsp->orig_ax = syscall = regsp->ax;
	regsp->ax = -ENOSYS;

#ifdef CONFIG_L4_FERRET_SYSCALL_COUNTER
	ferret_histo_bin_inc(l4x_ferret_syscall_ctr, syscall);
#endif

#if 0
	printk("Syscall %3d for %s(%d at %p): args: %lx,%lx,%lx\n",
	           syscall, p->comm, p->pid, (void *)regsp->ip,
		   regsp->bx, regsp->cx, regsp->dx);
#endif
#if 0
	if (syscall == 11) {
		char *filename;
		printk("execve: pid: %d(%s), " PRINTF_L4TASK_FORM ": ",
		       p->pid, p->comm, PRINTF_L4TASK_ARG(p->thread.user_thread_id));
		filename = getname((char *)regsp->bx);
		printk("%s\n", IS_ERR(filename) ? "UNKNOWN" : filename);
	}
#endif
#if 0
	if (syscall == 120) {
		printk("Syscall %3d for %s(%d at %p): arg1 = %lx ebp=%lx\n",
		       syscall, p->comm, p->pid, (void *)regsp->ip,
		       regsp->bx, regsp->bp);
	}
#endif

#if 0
	if (syscall == 21)
		printk("Syscall %3d mount for %s(%d at %p): %lx %lx %lx %lx %lx %lx\n",
		       syscall, p->comm, p->pid, (void *)regsp->ip,
		       regsp->bx, regsp->cx, regsp->dx, regsp->si,
		       regsp->di, regsp->bp);
#endif
#if 0
	if (syscall == 5) {
		char *filename = getname((char *)regsp->bx);
		printk("open: pid: %d(%s): %s (%lx)\n",
		       current->pid, current->comm,
		       IS_ERR(filename) ? "UNKNOWN" : filename, regsp->bx);
		putname(filename);
	}
#endif

	if (!is_lx_syscall(syscall)) {
		printk("Syscall %3d for %s(%d at %p): arg1 = %lx\n",
		       syscall, p->comm, p->pid, (void *)regsp->ip,
		       regsp->bx);
		l4x_print_regs(&p->thread, regsp);
		enter_kdebug("no syscall");
	}
	if (likely((is_lx_syscall(syscall))
		   && ((syscall_fn = sys_call_table[syscall])))) {
		//if (!p->user)
		//	enter_kdebug("dispatch_system_call: !p->user");

		/* valid system call number.. */
		if (unlikely(current_thread_info()->flags
		             & (_TIF_SYSCALL_EMU
		                | _TIF_SYSCALL_TRACE
		                | _TIF_SECCOMP
		                | _TIF_SYSCALL_AUDIT))) {
			syscall_trace_enter(regsp);
			regsp->ax = syscall_fn(regsp->bx, regsp->cx,
			                       regsp->dx, regsp->si,
			                       regsp->di, regsp->bp);
			syscall_trace_leave(regsp);
		} else {
			regsp->ax = syscall_fn(regsp->bx, regsp->cx,
			                       regsp->dx, regsp->si,
			                       regsp->di, regsp->bp);
		}
	}

	if (signal_pending(p))
		l4x_do_signal(regsp, syscall);

	if (need_resched())
		schedule();

#if 0
	printk("Syscall %3d for %s(%d at %p): return %lx/%ld\n",
	       syscall, p->comm, p->pid, (void *)regsp->ip,
	       regsp->ax, regsp->ax);
	if (unlikely(syscall == -38))
		enter_kdebug("no syscall");
#endif
}

/*
 * A primitive emulation.
 *
 * Returns 1 if something could be handled, 0 if not.
 */
static inline int l4x_port_emulation(struct pt_regs *regs)
{
	u8 op;

	if (get_user(op, (char *)regs->ip))
		return 0; /* User memory could not be accessed */

	//printf("OP: %x (ip: %08x) dx = 0x%x\n", op, regs->ip, regs->edx & 0xffff);

	switch (op) {
		case 0xed: /* in dx, eax */
		case 0xec: /* in dx, al */
			switch (regs->dx & 0xffff) {
				case 0xcf8:
				case 0x3da:
				case 0x3cc:
				case 0x3c1:
					regs->ax = -1;
					regs->ip++;
					return 1;
			};
		case 0xee: /* out al, dx */
			switch (regs->dx & 0xffff) {
				case 0x3c0:
					regs->ip++;
					return 1;
			};
	};

	return 0; /* Not handled here */
}

/*
 * Emulation of (some) jdb commands. The user program may not
 * be allowed to issue jdb commands, they trap in here. Nevertheless
 * hybrid programs may want to use some of them. Emulate them here.
 * Note:  When there's a failure reading the string from user we
 *        nevertheless return true.
 * Note2: More commands to be emulated can be added on request.
 */
static int l4x_kdebug_emulation(struct pt_regs *regs)
{
	u8 op = 0, val;
	char *addr = (char *)regs->ip - 1;
	int i, len;

	if (get_user(op, addr))
		return 0; /* User memory could not be accessed */

	if (op != 0xcc) /* Check for int3 */
		return 0; /* Not for us */

	/* jdb command group */
	if (get_user(op, addr + 1))
		return 0; /* User memory could not be accessed */

	if (op == 0xeb) { /* enter_kdebug */
		if (get_user(len, addr + 2))
			return 0; /* Access failure */
		regs->ip += len + 2;
		outstring("User enter_kdebug text: ");
		for (i = 3; len; len--) {
			if (get_user(val, addr + i++))
				break;
			outchar(val);
		}
		outchar('\n');
		enter_kdebug("User program enter_kdebug");

		return 1; /* handled */

	} else if (op == 0x3c) {
		if (get_user(op, addr + 2))
			return 0; /* Access failure */
		switch (op) {
			case 0: /* outchar */
				outchar(regs->ax & 0xff);
				break;
			case 1: /* outnstring */
				len = regs->bx;
				for (i = 0;
				     !get_user(val, (char *)(regs->ax + i++))
				     && len;
				     len--)
					outchar(val);
				break;
			case 2: /* outstring */
				for (i = 0;
				     !get_user(val, (char *)(regs->ax + i++))
				     && val;)
					outchar(val);
				break;
			case 5: /* outhex32 */
				outhex32(regs->ax);
				break;
			case 6: /* outhex20 */
				outhex20(regs->ax);
				break;
			case 7: /* outhex16 */
				outhex16(regs->ax);
				break;
			case 8: /* outhex12 */
				outhex12(regs->ax);
				break;
			case 9: /* outhex8 */
				outhex8(regs->ax);
				break;
			case 11: /* outdec */
				outdec(regs->ax);
				break;
			default:
				return 0; /* Did not understand */
		};
		regs->ip += 2;
		return 1; /* handled */
	}

	return 0; /* Not handled here */
}

static inline unsigned r_trapno(struct thread_struct *t, l4_vcpu_state_t *v)
{
#ifdef CONFIG_L4_VCPU
	return v->r.trapno;
#else
	return t->trap_no;
#endif
}

static inline unsigned r_err(struct thread_struct *t, l4_vcpu_state_t *v)
{
#ifdef CONFIG_L4_VCPU
	return v->r.err;
#else
	return t->error_code;
#endif
}

/*
 * Return values: 0 -> do send a reply
 *                1 -> don't send a reply
 */
static inline int l4x_dispatch_exception(struct task_struct *p,
                                         struct thread_struct *t,
                                         l4_vcpu_state_t *v,
                                         struct pt_regs *regs)
{
#ifndef CONFIG_L4_VCPU
	l4x_hybrid_do_regular_work();
#endif
	l4x_debug_stats_exceptions_hit();

	if (0) {
#ifndef CONFIG_L4_VCPU
	} else if (t->trap_no == 0xff) {
		/* we come here for suspend events */
		TBUF_LOG_SUSPEND(fiasco_tbuf_log_3val("dsp susp", TBUF_TID(t->user_thread_id), regs->ip, 0));
		l4x_dispatch_suspend(p, t);

		return 0;
#endif
	} else if (likely(r_trapno(t, v) == 0xd && r_err(t, v) == 0x402)) {
		/* int 0x80 is trap 0xd and err 0x402 (0x80 << 3 | 2) */

		TBUF_LOG_INT80(fiasco_tbuf_log_3val("int80  ", TBUF_TID(t->user_thread_id), regs->ip, regs->ax));

		/* set after int 0x80, before syscall so the forked childs
		 * get the increase too */
		regs->ip += 2;

		dispatch_system_call(p, regs);

		BUG_ON(p != current);

#ifdef CONFIG_L4_VCPU
		return 0;
#else

		if (likely(!t->restart))
			/* fine, go send a reply and return to userland */
			return 0;

		/* Restart whole dispatch loop, also restarts thread */
		t->restart = 0;
		return 2;
#endif

	} else if (r_trapno(t, v) == 7) {
		math_state_restore();

		/* XXX: math emu*/
		/* if (!cpu_has_fpu) math_emulate(..); */

		return 0;

	} else if (unlikely(r_trapno(t, v) == 1)) {
		/* Singlestep */
		return 0;
	} else if (r_trapno(t, v) == 0xd) {

#ifndef CONFIG_L4_VCPU
		if (l4x_hybrid_begin(p, t))
			return 0;
#endif

		/* Fall through otherwise */
	}

	if (r_trapno(t, v) == 3) {
		if (l4x_kdebug_emulation(regs))
			return 0; /* known and handled */
		do_int3(regs, r_err(t, v));
		if (signal_pending(p))
			l4x_do_signal(regs, 0);
		if (need_resched())
			schedule();
		return 0;
	}

	if (l4x_port_emulation(regs))
		return 0; /* known and handled */

	TBUF_LOG_EXCP(fiasco_tbuf_log_3val("except ", TBUF_TID(t->user_thread_id), t->trap_no, t->error_code));

	if (l4x_deliver_signal(r_trapno(t, v), r_err(t, v)))
		return 0; /* handled signal, reply */

	/* This path should never be reached... */

	printk("(Unknown) EXCEPTION\n");
	l4x_print_regs(t, regs);
	printk("will die...\n");

	enter_kdebug("check");

	/* The task somehow misbehaved, so it has to die */
	l4x_sig_current_kill();

	return 1; /* no reply */
}

static inline int l4x_handle_page_fault_with_exception(struct thread_struct *t)
{
	return 0; // not for us
}

#ifdef CONFIG_L4_VCPU
static inline void l4x_vcpu_entry_user_arch(void)
{
#ifdef CONFIG_X86_32
	asm ("cld          \n"
	     "mov %0, %%gs \n"
	     "mov %1, %%fs \n"
	     : : "r" (l4x_x86_utcb_get_orig_segment()),
#ifdef CONFIG_SMP
	     "r" ((l4x_fiasco_gdt_entry_offset + 2) * 8 + 3)
#else
	     "r" (l4x_x86_utcb_get_orig_segment())
#endif
	     : "memory");
#else
	asm ("cld" : : : "memory");
#endif
}

static inline bool l4x_vcpu_is_wr_pf(l4_vcpu_state_t *v)
{
	return v->r.err & 2;
}
#endif

#define __INCLUDED_FROM_L4LINUX_DISPATCH
#include "../dispatch.c"
