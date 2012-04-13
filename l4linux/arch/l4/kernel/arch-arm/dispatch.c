
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tick.h>
#include <linux/slab.h>

#include <asm/processor.h>
#include <asm/mmu_context.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <asm/pgalloc.h>
#include <asm/tls.h>

#include <l4/sys/cache.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/ktrace.h>
#include <l4/sys/utcb.h>
#include <l4/sys/task.h>
#include <l4/util/util.h>
#include <l4/log/log.h>
#include <l4/re/consts.h>


#include <asm/l4lxapi/task.h>
#include <asm/l4lxapi/thread.h>
#include <asm/l4lxapi/memory.h>
#include <asm/l4lxapi/irq.h>
#include <asm/api/macros.h>

#include <asm/generic/dispatch.h>
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
#include <asm/l4x/upage.h>
#include <asm/l4x/signal.h>

//#define DEBUG_SYSCALL_PRINTFS

#ifdef DEBUG_SYSCALL_PRINTFS
#include <linux/fs.h>
#endif

#define USER_PATCH_CMPXCHG
#define USER_PATCH_GETTLS
#define USER_PATCH_DMB
enum {
 USER_PATCH_CMPXCHG_SHOW = 0,
 USER_PATCH_GETTLS_SHOW  = 0,
 USER_PATCH_DMB_SHOW     = 0,
};


#if 1
#define TBUF_LOG_IDLE(x)        TBUF_DO_IT(x)
#define TBUF_LOG_WAKEUP_IDLE(x)	TBUF_DO_IT(x)
#define TBUF_LOG_USER_PF(x)     TBUF_DO_IT(x)
#define TBUF_LOG_SWI(x)         TBUF_DO_IT(x)
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
#define TBUF_LOG_SWI(x)
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

struct l4x_arch_cpu_fpu_state *l4x_fpu_get(unsigned cpu)
{
	return &per_cpu(l4x_cpu_fpu_state, cpu);
}

static inline int l4x_msgtag_fpu(unsigned cpu)
{
	return (l4x_fpu_get(cpu)->fpexc & (1 << 30)) ?  L4_MSGTAG_TRANSFER_FPU : 0;
}

#ifdef CONFIG_VFP
static void l4x_fpu_get_info(l4_utcb_t *utcb)
{
	struct l4x_arch_cpu_fpu_state *x = l4x_fpu_get(smp_processor_id());
	x->fpexc   = l4_utcb_mr_u(utcb)->mr[21];
	x->fpinst  = l4_utcb_mr_u(utcb)->mr[22];
	x->fpinst2 = l4_utcb_mr_u(utcb)->mr[23];
}
#endif

static inline int l4x_msgtag_copy_ureg(l4_utcb_t *u)
{
	if (tls_emu || has_tls_reg) {
		l4_utcb_mr_u(u)->mr[25] = current_thread_info()->tp_value;
		return 0x8000;
	}
	return 0;
}

asmlinkage void syscall_trace(int why, struct pt_regs *regs, int scno);


static inline int l4x_is_triggered_exception(l4_umword_t val)
{
	return (val & 0x00f00000) == 0x00500000;
}

static inline unsigned long regs_pc(struct thread_struct *t)
{
	return L4X_THREAD_REGSP(t)->ARM_pc;
}

static inline unsigned long regs_sp(struct thread_struct *t)
{
	return L4X_THREAD_REGSP(t)->ARM_sp;
}

static inline void l4x_arch_task_setup(struct thread_struct *t)
{

}

static inline void l4x_arch_do_syscall_trace(struct task_struct *p,
                                             struct thread_struct *t)
{
	if (unlikely(test_tsk_thread_flag(p, TIF_SYSCALL_TRACE)))
		syscall_trace(1, L4X_THREAD_REGSP(t), __NR_fork);
}

static inline int l4x_hybrid_check_after_syscall(l4_utcb_t *utcb)
{
	l4_exc_regs_t *exc = l4_utcb_exc_u(utcb);
	return exc->err == 0x00310000 // after L4 syscall
	       //|| exc->err == 0x00200000
	       || exc->err == 0x00500000; // L4 syscall exr
}

static inline void l4x_dispatch_delete_polling_flag(void)
{
}

static inline void l4x_dispatch_set_polling_flag(void)
{
}

#ifdef CONFIG_L4_VCPU
static inline void l4x_arch_task_start_setup(l4_vcpu_state_t *v,
		                             struct task_struct *p)
#else
static inline void l4x_arch_task_start_setup(struct task_struct *p)
#endif
{
}

extern void schedule_tail(struct task_struct *prev);

static inline l4_umword_t l4x_l4pfa(struct thread_struct *t)
{
	return (t->address & ~3) | (!!(t->error_code & (1 << 11)) << 1);
}

static inline int l4x_ispf(struct thread_struct *t)
{
	return t->error_code & 0x00010000;
}

void l4x_finish_task_switch(struct task_struct *prev);
int  l4x_deliver_signal(int exception_nr, int error_code);

DEFINE_PER_CPU(struct thread_info *, l4x_current_ti) = &init_thread_info;
DEFINE_PER_CPU(struct thread_info *, l4x_current_proc_run);
#ifndef CONFIG_L4_VCPU
static DEFINE_PER_CPU(unsigned, utcb_snd_size);
#endif


asm(
".section .text				\n"
".global ret_from_fork			\n"
"ret_from_fork:				\n"
"	bl	schedule_tail		\n"
#ifdef CONFIG_L4_VCPU
"	b	l4x_vcpu_ret_from_fork  \n"
#else
"	bl	l4x_user_dispatcher	\n"
#endif
".previous				\n"
);

#include <asm/generic/stack_id.h>
void l4x_switch_to(struct task_struct *prev, struct task_struct *next)
{
#ifdef CONFIG_L4_VCPU
	l4_vcpu_state_t *vcpu = l4x_vcpu_state(smp_processor_id());
#endif
#if 0
	L4XV_V(f);
	L4XV_L(f);
	LOG_printf("%s: cpu%d: %s(%d)[%ld] -> %s(%d)[%ld]\n",
	           __func__, smp_processor_id(),
	           prev->comm, prev->pid, prev->state,
	           next->comm, next->pid, next->state);
	LOG_printf("      :: %p -> %p\n", prev->stack, next->stack);
	L4XV_U(f);
#endif
#ifdef CONFIG_L4_VCPU
	TBUF_LOG_SWITCH(
	     fiasco_tbuf_log_3val("SWITCH", (l4_umword_t)prev->stack,
		     (l4_umword_t)next->stack, 0));
#else
	TBUF_LOG_SWITCH(
	     fiasco_tbuf_log_3val("SWITCH",
				  TBUF_TID(prev->thread.user_thread_id),
				  TBUF_TID(next->thread.user_thread_id),
				  0));
#endif

#ifdef CONFIG_L4_VCPU
	if (vcpu->state & L4_VCPU_F_IRQ)
		enter_kdebug("switch + irq");

	{
		register unsigned long sp asm ("sp");
		if (((l4_umword_t)prev->stack & 0xffffe000) != (sp & 0xffffe000)) {
			enter_kdebug("prev->stack != sp");
		}
	}

	if (next->mm && prev->mm != next->mm)
		vcpu->user_task = next->mm->context.task;
#endif

#ifndef CONFIG_L4_VCPU
	per_cpu(l4x_current_ti, smp_processor_id())
	  = (struct thread_info *)((unsigned long)next->stack & ~(THREAD_SIZE - 1));
#endif

#ifdef CONFIG_L4_ARM_UPAGE_TLS
	*(unsigned long *)(upage_addr + UPAGE_USER_TLS_OFFSET) = task_thread_info(next)->tp_value;
#endif
	if (has_tls_reg)
		asm volatile("mcr p15, 0, %0, c13, c0, 2"
		             : : "r" (task_thread_info(next)->tp_value));

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
	mb();
	vcpu->entry_sp = task_thread_info(next)->cpu_context.extra[0];

	if (next->mm && (vcpu->entry_sp <= (l4_umword_t)next->stack
	    || vcpu->entry_sp > (l4_umword_t)next->stack + 0x2000)) {
		LOG_printf("stack: %lx  %lx\n", (l4_umword_t)next->stack,
				vcpu->entry_sp);
		enter_kdebug("Wrong entry-sp?");
	}
	mb();
#endif
}

static inline void l4x_pte_add_access_and_mapped(pte_t *ptep)
{
	pte_val(*ptep) |= (L_PTE_YOUNG + L_PTE_MAPPED);
}

static inline void l4x_pte_add_access_mapped_and_dirty(pte_t *ptep)
{
	pte_val(*ptep) |= (L_PTE_YOUNG + L_PTE_DIRTY + L_PTE_MAPPED);
}

#ifdef CONFIG_L4_VCPU
static inline void vcpu_to_thread_struct(l4_vcpu_state_t *v,
                                         struct thread_struct *t)
{
	t->error_code = v->r.err;
	t->address    = v->r.pfa;
}

static inline void thread_struct_to_vcpu(l4_vcpu_state_t *v,
                                         struct thread_struct *t)
{
}
#endif

static inline void utcb_to_thread_struct(l4_utcb_t *utcb,
                                         struct thread_struct *t)
{
	l4_exc_regs_t *exc = l4_utcb_exc_u(utcb);
	utcb_exc_to_ptregs(exc, L4X_THREAD_REGSP(t));
	t->error_code     = exc->err;
	t->address        = exc->pfa;
}

static inline void thread_struct_to_utcb(struct thread_struct *t,
                                         l4_utcb_t *utcb,
                                         unsigned int send_size)
{
	ptregs_to_utcb_exc(L4X_THREAD_REGSP(t), l4_utcb_exc_u(utcb));
#ifndef CONFIG_L4_VCPU
	per_cpu(utcb_snd_size, smp_processor_id()) = send_size;
#endif
}

#ifndef CONFIG_L4_VCPU
static int l4x_hybrid_begin(struct task_struct *p,
                            struct thread_struct *t);


static void l4x_dispatch_suspend(struct task_struct *p,
                                 struct thread_struct *t);
#endif

static inline void l4x_print_regs(struct thread_struct *t, struct pt_regs *r)
{
#define R(nr) r->uregs[nr]
	LOG_printf("0: %08lx %08lx %08lx %08lx  4: %08lx %08lx %08lx %08lx\n",
	           R(0), R(1), R(2), R(3), R(4), R(5), R(6), R(7));
	LOG_printf("8: %08lx %08lx %08lx %08lx 12: %08lx [01;34m%08lx[0m "
	           "%08lx [01;34m%08lx[0m\n",
	           R(8), R(9), R(10), R(11), R(12), R(13), R(14), R(15));
	LOG_printf("CPSR: %08lx Err: %08lx\n", r->ARM_cpsr, t->error_code);
#undef R
}

//#include <linux/fs.h>

static inline void call_system_call_args(syscall_t *sctbl,
                                         unsigned long syscall,
                                         unsigned long arg1,
                                         unsigned long arg2,
                                         unsigned long arg3,
                                         unsigned long arg4,
                                         unsigned long arg5,
                                         unsigned long arg6,
                                         struct pt_regs *regsp)
{
	syscall_t syscall_fn;

#ifdef DEBUG_SYSCALL_PRINTFS
	if (1)
		printk("Syscall call: %ld for %d(%s, pc=%p, lr=%p, sp=%08lx, "
		       "cpu=%d) (%08lx %08lx %08lx %08lx %08lx %08lx)\n",
		       syscall, current->pid, current->comm,
		       (void *)regsp->ARM_pc,
		       (void *)regsp->ARM_lr, regsp->ARM_sp,
		       smp_processor_id(),
		       arg1, arg2, arg3, arg4, arg5, arg6);

	if (0 && syscall == 11) {
		char *filename = getname((char *)arg1);
		printk("execve: pid: %d(%s) %d: %s (%08lx)\n",
		       current->pid, current->comm, current_uid(),
		       IS_ERR(filename) ? "INVALID" : filename, arg1);
		putname(filename);
	}
	if (0 && syscall == 1) {
		printk("exit: pid: %d(%s)\n", current->pid, current->comm);
	}
	if (0 && syscall == 2) {
		printk("fork: pid: %d(%s)\n",
		       current->pid, current->comm);
	}
	if (0 && syscall == 3) {
		printk("read: pid: %d(%s): fd = %ld len = %ld\n",
		       current->pid, current->comm, arg1, arg3);
	}
	if (0 && syscall == 4) {
		printk("write: pid: %d(%s): fd = %ld len = %ld\n",
		       current->pid, current->comm, arg1, arg3);
	}
	if (0 && syscall == 5) {
		char *filename = getname((char *)arg1);
		printk("open: pid: %d(%s): %s (%lx)\n",
		       current->pid, current->comm,
		       IS_ERR(filename) ? "INVALID" : filename, arg1);
		putname(filename);
	}
	if (0 && syscall == 12) {
		char *f1 = getname((char *)arg1);
		printk("chdir: pid: %d(%s): %s\n",
		       current->pid, current->comm,
		       IS_ERR(f1) ? "INVALID" : f1);
		putname(f1);
	}
	if (0 && syscall == 14) {
		char *f1 = getname((char *)arg1);
		printk("mknod: pid: %d(%s): %s dev=%lx\n",
		       current->pid, current->comm,
		       IS_ERR(f1) ? "INVALID" : f1, arg3);
		putname(f1);
	}
	if (0 && syscall == 39) {
		char *filename = getname((char *)arg1);
		printk("mkdir: pid: %d(%s): %s (%lx)\n",
		       current->pid, current->comm,
		       IS_ERR(filename) ? "INVALID" : filename, arg1);
		putname(filename);
	}
	if (0 && syscall == 21) {
		char *f1 = getname((char *)arg1);
		char *f2 = getname((char *)arg2);
		printk("mount: pid: %d(%s): %s -> %s\n",
		       current->pid, current->comm,
		       IS_ERR(f1) ? "INVALID" : f1,
		       IS_ERR(f2) ? "INVALID" : f2);
		putname(f1);
		putname(f2);
	}
	if (0 && syscall == 45)
		printk("brk: pid: %d(%s): %lx\n", current->pid, current->comm, arg1);
	if (0 && syscall == 54)
		printk("ioctl: pid: %d(%s): %lx\n",
		       current->pid, current->comm, arg1);

	if (0 && syscall == 119)
		printk("sigreturn: pid: %d(%s)\n", current->pid, current->comm);
	if (0 && syscall == 120)
		printk("clone: pid: %d(%s)\n", current->pid, current->comm);
	if (0 && syscall == 190)
		printk("vfork: pid: %d(%s)\n", current->pid, current->comm);
	if (0 && syscall == 192)
		printk("mmap2 size: pid: %d(%s): %lx\n",
		       current->pid, current->comm, arg2);
	if (0 && syscall == 195) {
		char *path = getname((char *)arg1);
		printk("stat64: pid: %d(%s): %s (%lx)\n",
		       current->pid, current->comm,
		       IS_ERR(path) ? "INVALID" : path, arg1);
		putname(path);
	}
	if (0 && syscall == 221) {
		printk("fcntl64: pid: %d(%s): (%lx, %lx, %lx)\n",
		       current->pid, current->comm,
		       arg1, arg2, arg3);
	}
	if (0 && syscall == 266)
		printk("statfs: pid: %d(%s): (%lx, %lx, %lx) sp=%lx\n",
		       current->pid, current->comm,
		       arg1, arg2, arg3, regsp->ARM_sp);
#endif

	/* ============================================================ */

	if (likely(is_lx_syscall(syscall))
	           && ((syscall_fn = sctbl[syscall]))) {
		//if (unlikely(!current->user))
		//	enter_kdebug("call_system_call_args: !current->user");

		/* valid system call number.. */
		if (likely(!test_tsk_thread_flag(current, TIF_SYSCALL_TRACE))) {
			regsp->ARM_r0 = syscall_fn(arg1, arg2, arg3, arg4, arg5, arg6);
		} else {
			syscall_trace(0, regsp, syscall);
			regsp->ARM_r0 = syscall_fn(arg1, arg2, arg3, arg4, arg5, arg6);
			syscall_trace(1, regsp, syscall);
		}
	} else
		regsp->ARM_r0 = -ENOSYS;

	/* ============================================================ */
#ifdef DEBUG_SYSCALL_PRINTFS
	if (0 && syscall == 192) {
		printk("mmap2 result: pid: %d(%s): %lx\n",
		       current->pid, current->comm,
		       regsp->ARM_r0);
	}
	if (0 && syscall == 195) {
		printk("stat64 result: pid: %d(%s): %lx\n",
		       current->pid, current->comm,
		       regsp->ARM_r0);
	}
	if (0 && syscall == 65) {
		printk("getpgrp result: pid: %d(%s): %lx\n",
		       current->pid, current->comm,
		       regsp->ARM_r0);
	}
	if (1)
		printk("Syscall %ld return: 0x%lx\n", syscall, regsp->ARM_r0);
#endif
}

static inline void dispatch_system_call(syscall_t *sctbl, struct task_struct *p,
                                        unsigned long syscall,
                                        struct pt_regs *regsp)
{
#ifdef CONFIG_L4_VCPU
	local_irq_enable();
	p->thread.regsp = regsp;
#endif

	//syscall_count++;

	regsp->ARM_ORIG_r0 = regsp->ARM_r0;

	if (unlikely(syscall == __NR_syscall - __NR_SYSCALL_BASE)) {
		call_system_call_args(sctbl, regsp->ARM_r0,
		                      regsp->ARM_r1, regsp->ARM_r2,
		                      regsp->ARM_r3, regsp->ARM_r4,
		                      regsp->ARM_r5, regsp->ARM_r6,
		                      regsp);
	} else {
		call_system_call_args(sctbl, syscall,
		                      regsp->ARM_r0, regsp->ARM_r1,
		                      regsp->ARM_r2, regsp->ARM_r3,
		                      regsp->ARM_r4, regsp->ARM_r5,
		                      regsp);
	}

	if (signal_pending(p))
		l4x_do_signal(regsp, syscall);

	if (need_resched())
		schedule();
}

static char *l4x_arm_decode_error_code(unsigned long error_code)
{
	switch (error_code & 0x00f00000) {
		case 0x00100000:
			return "Undefined instruction";
		case 0x00200000:
			return "SWI";
		case 0x00400000:
			if (error_code & 0x00020000)
				return "Data abort (read)";
			return "Data abort";
		case 0x00500000:
			return "Forced exception";
	}
	return "Unknown";
}

/* XXX: Move that out to a separate file to avoid the VM_EXEC clash
 *      (they have the same value, but anyway... */
#undef VM_EXEC
#include <asm/asm-offsets.h>
#include <linux/stringify.h>

/* FP emu */
asm(
"	.data						\n"
"	.global fp_enter				\n"
"fp_enter:						\n"
"	.word callswi					\n"
"	.text						\n"
"callswi:						\n"
"	swi #2						\n"
);

/*
 * We directly call into the nwfpe code and do not take the fp_enter hook,
 * because otherwise the sp handling would be a bit too tricky.
 */
#ifndef CONFIG_FPE_NWFPE
static inline unsigned int EmulateAll(unsigned int opcode)
{
	return 0;
}
static inline unsigned int checkCondition(const unsigned int opcode,
                                          const unsigned int ccode)
{
	return 0;
}
#else
unsigned int EmulateAll(unsigned int opcode);
unsigned int checkCondition(const unsigned int opcode, const unsigned int ccode);

struct pt_regs *l4x_fp_get_user_regs(void)
{
	return L4X_THREAD_REGSP(&current->thread);
}
#endif

static inline int sc_get_user_4(unsigned long *store, unsigned long addr)
{
	if (l4x_is_upage_user_addr(addr)) {
		*store = *(unsigned long *)
			   (upage_addr + (addr - UPAGE_USER_ADDRESS));
		return 0;
	}
	return get_user(*store, (unsigned long *)addr);
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
	//struct pt_regs *regs = L4X_THREAD_REGSP(t);
	int handled = 0;

#ifndef CONFIG_L4_VCPU
	l4x_hybrid_do_regular_work();
#endif
	l4x_debug_stats_exceptions_hit();

#ifndef CONFIG_L4_VCPU
	if (l4x_is_triggered_exception(t->error_code)) {
		/* we come here for suspend events */
		TBUF_LOG_SUSPEND(fiasco_tbuf_log_3val("dsp susp", TBUF_TID(t->user_thread_id), regs->ARM_pc, 0));

		l4x_kuser_cmpxchg_check_and_fixup(regs);
		l4x_dispatch_suspend(p, t);

		return 0;
	}
#endif

	// adjust pc to point at the insn
	regs->ARM_pc -= thumb_mode(regs) ? 2 : 4;

	//fiasco_tbuf_log_3val("EXC", TBUF_TID(t->user_thread_id), regs->ARM_pc, t->error_code);

	if ((t->error_code & 0x00f00000) == 0x00200000) {
#if defined(CONFIG_OABI_COMPAT) || defined(CONFIG_ARM_THUMB) || !defined(CONFIG_AEABI)
		unsigned long ret = 0;
#endif
#if defined(CONFIG_OABI_COMPAT)
		unsigned long insn;
#endif
		unsigned long scno = regs->uregs[7];
		syscall_t *tbl;

#ifdef CONFIG_L4_DEBUG_SEGFAULTS
		if (0 && unlikely(l4x_is_upage_user_addr(regs->ARM_pc))) {
			printk("Got SWI at %08lx\n", regs->ARM_pc);
			l4x_print_vm_area_maps(p, regs->ARM_pc);
			l4x_print_regs(&p->thread, regs);
			goto go_away;
		}
#endif

#if defined(CONFIG_OABI_COMPAT)
		/*
		 * If we have CONFIG_OABI_COMPAT then we need to look at the swi
		 * value to determine if it is an EABI or an old ABI call.
		 */
		if (thumb_mode(regs))
			insn = 0;
		else
			ret = sc_get_user_4(&insn, regs->ARM_pc);

#elif defined(CONFIG_AEABI)
		/* Pure EABI user space always put syscall number into scno (r7) */
#elif defined(CONFIG_ARM_THUMB)
		/* Legacy ABI only, possibly thumb mode. */
		if (thumb_mode(regs))
			scno &= __NR_SYSCALL_BASE;
		else
			ret = sc_get_user_4(&scno, regs->ARM_pc);
#else
		ret = sc_get_user_4(&scno, regs->ARM_pc);
#endif


		//printk("1: insn: %lx   scno: %lx  ret: %lx pc=%lx\n", insn, scno, ret, regs->ARM_pc);

		tbl = sys_call_table;

#if defined(CONFIG_OABI_COMPAT)
		/*
		 * If the swi argument is zero, this is an EABI call and we do nothing.
		 *
		 * If this is an old ABI call, get the syscall number into scno and
		 * get the old ABI syscall table address.
		 */
		insn &= ~0xff000000;
		if (insn) {
			scno = insn ^ __NR_OABI_SYSCALL_BASE;
			tbl = sys_oabi_call_table;
		}
#elif !defined(CONFIG_AEABI)
		scno &= ~0xff000000;
		scno ^= __NR_SYSCALL_BASE;
#endif


		TBUF_LOG_SWI(fiasco_tbuf_log_3val("swi    ", TBUF_TID(t->user_thread_id), regs->ARM_pc, scno));

		regs->ARM_pc += thumb_mode(regs) ? 2 : 4;

		// handle private ARM syscalls?
		if (unlikely((__ARM_NR_BASE - __NR_SYSCALL_BASE) < scno
		             && scno <= (__ARM_NR_BASE - __NR_SYSCALL_BASE) + 5)) {
			// from traps.c
			asmlinkage int arm_syscall(int no, struct pt_regs *regs);
			regs->ARM_r0 = arm_syscall(scno | __ARM_NR_BASE, regs);
			return 0;
		}

#ifdef CONFIG_L4_DEBUG_SEGFAULTS
		if (unlikely(!is_lx_syscall(scno))) {
			printk("Hmm, rather unknown syscall nr 0x%lx/%ld\n", scno, scno);
			l4x_print_vm_area_maps(p, ~0UL);
			l4x_print_regs(&p->thread, regs);
			goto go_away;
		}
#endif

		dispatch_system_call(tbl, p, scno, regs);

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

#ifndef CONFIG_L4_VCPU
	} else if (t->error_code == 0x00300000) {
		/* Syscall alien exception */
		if (l4x_hybrid_begin(p, t))
			return 0;
#endif
	}

	TBUF_LOG_EXCP(fiasco_tbuf_log_3val("except ", TBUF_TID(t->user_thread_id), t->error_code, regs->ARM_pc));

#ifdef CONFIG_VFP
	if ((t->error_code & 0x01f00000) == 0x01100000) {
		unsigned long insn;
		int ret;
		l4x_fpu_get_info(l4_utcb());

		if (thumb_mode(regs))
			ret = get_user(insn, (unsigned short *)regs->ARM_pc);
		else
			ret = get_user(insn, (unsigned long *)regs->ARM_pc);

		if (!ret) {
			register unsigned long r0 asm ("r7") = insn;
			register unsigned long r2 asm ("r9") = regs->ARM_pc + 4;
			register unsigned long r8 asm ("r8") = (unsigned long)L4X_THREAD_REGSP(t);
			register unsigned long r10 asm ("r10") = (unsigned long) current_thread_info();
			asm volatile(
#ifdef CONFIG_FRAME_POINTER
			             "push    {fp}                   \t\n"
#endif
				     "sub     sp, sp, %[PTREGS_SIZE] \t\n"
				     "mov     r0, sp                 \t\n"
				     "mov     r1, %[ptregs]          \t\n"
				     "mov     r2, %[PTREGS_SIZE]     \t\n"
				     "bl      memcpy                 \t\n"
				     "mov     r0, %[insn]            \t\n"
				     "mov     r2, %[afterpc]         \t\n"
			             "adr     r9, 1f                 \t\n"
			             "bl      do_vfp                 \t\n"
				     "mov     r0, #0                 \t\n"
				     "b       2f                     \t\n"
			             "1: @success                    \t\n"
				     "        mov r0, #1             \t\n"
				     "2:                             \t\n"
				     "add     sp, sp, %[PTREGS_SIZE] \t\n"
#ifdef CONFIG_FRAME_POINTER
				     "pop {fp}                       \t\n"
#endif
			             : "=r" (r0)
			             : [insn]        "r" (r0),
			               [afterpc]     "r" (r2),
			                             "r" (r10),
				       [ptregs]      "r" (r8),
				       [PTREGS_SIZE] "I" (sizeof(*regs))
			             : "memory", "cc",
				       "r1", "r3", "r4", "r5", "r6",
				       "r0", "r2", "r12", "lr"
#ifndef CONFIG_FRAME_POINTER
				     , "r11"
#endif
				     );
			if (r0)
				return 0; // success with vfp stuff
			// possibility to do it better: loop as below
		}
	}
#endif

	while (1) {
		unsigned long insn;
		int ret;

		if (thumb_mode(regs)) {
			insn = 0;
			ret = get_user(insn, (unsigned short *)(regs->ARM_pc));
			regs->ARM_pc += 2;
		} else {
			ret = get_user(insn, (unsigned long *)(regs->ARM_pc));
			regs->ARM_pc += 4;
		}
		//LOG_printf("insn: %lx\n", insn);

		if (ret)
			break;


		if (!checkCondition(insn, regs->ARM_cpsr))
			break;

		if (EmulateAll(insn))
			handled = 1;
		else
			break;
	}

	// adjust pc to point at the insn again
	regs->ARM_pc -= thumb_mode(regs) ? 2 : 4;

	if (likely(handled))
		return 0; /* handled */

	// still not handled, PC is on the insn now

#ifdef CONFIG_L4_DEBUG_SEGFAULTS
	l4x_print_regs(t, regs);

	if (   t->error_code == 0x00100000
	    || t->error_code == 0x00110000
	    || t->error_code == 0x00200000) {
		unsigned long val;
		int ret;

		if (thumb_mode(regs)) {
			val = 0;
			ret = get_user(val, (unsigned short *)regs->ARM_pc);
		} else
			ret = get_user(val, (unsigned long *)regs->ARM_pc);

		if (ret) {
			printk("get_user error: %d\n", ret);
			val = ~0UL;
		}
		printk(" %s/%d: Undefined instruction at"
		       " %08lx with content %08lx, err %08lx\n",
		       p->comm, p->pid, regs->ARM_pc, val, t->error_code);
		l4x_print_vm_area_maps(p, regs->ARM_pc);
		l4x_print_regs(&p->thread, regs);
		enter_kdebug("undef insn");
	}
#endif

	if (l4x_deliver_signal(0, t->error_code))
		return 0; /* handled signal, reply */

#ifdef CONFIG_L4_DEBUG_SEGFAULTS
go_away:
#endif

	printk("Error code: %s\n", l4x_arm_decode_error_code(t->error_code));
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
	struct pt_regs *regs = L4X_THREAD_REGSP(t);

	if (0 && (regs->ARM_pc >= TASK_SIZE || l4x_l4pfa(t) >= TASK_SIZE))
		printk("PC/PF>TS: PC=%08lx PF=%08lx LR=%08lx\n",
		       regs->ARM_pc, l4x_l4pfa(t), regs->ARM_lr);

	l4x_kuser_cmpxchg_check_and_fixup(regs);

	if (l4x_l4pfa(t) == 0xffff0ff0) {
		unsigned long pc = regs->ARM_pc;
		int targetreg = -1;

		if (thumb_mode(regs)) {
			unsigned short op;
			get_user(op, (unsigned short *)pc);
			if ((op & 0xf800) == 0x6800) // ldr
				targetreg = op & 7;
			if (targetreg != -1)
				regs->uregs[targetreg] = current_thread_info()->tp_value;
			else
				LOG_printf("Lx: Unknown thumb opcode %hx at %lx\n", op, pc);
			regs->ARM_pc += 2;
		} else {
			unsigned long op;
			get_user(op, (unsigned long *)pc);
			if ((op & 0x0e100000) == 0x04100000)
				targetreg = (op >> 12) & 0xf;

			if (targetreg != -1)
				regs->uregs[targetreg] = current_thread_info()->tp_value;
			else
				LOG_printf("Lx: Unknown opcode %lx at %lx\n", op, pc);
			regs->ARM_pc += 4;
		}
		return 1; // handled
	}

	// __kuser_get_tls
	if (l4x_l4pfa(t) == 0xffff0fe0 && regs->ARM_pc == 0xffff0fe0) {
#ifdef USER_PATCH_GETTLS
		unsigned long val;

		if (USER_PATCH_GETTLS_SHOW) {
			unsigned long op;
			printk("GETTLS hit at lr=%lx pc=%lx\n", regs->ARM_lr, regs->ARM_pc);
			get_user(op, (unsigned long *)regs->ARM_lr);
			fiasco_tbuf_log_3val("TLSfnc", regs->ARM_lr, regs->ARM_pc, op);
		}

		if (regs->ARM_lr & 1)
			goto trap_and_emulate;

		if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 4))))
			goto trap_and_emulate;

		if ((val & 0xff000000) == 0xeb000000) {
			/* Code:
			   000080f0 <__aeabi_read_tp>:
			   __aeabi_read_tp():
			    80f0:       e3e00a0f        mvn     r0, #61440      ; 0xf000
			    80f4:       e240f01f        sub     pc, r0, #31
			    80f8:       e1a00000        nop                     ; (mov r0, r0)
			    80fc:       e1a00000        nop                     ; (mov r0, r0)

			   ....

			   xxxxx:       ebyyyyyy        bl      80f0 <__aeabi_read_tp>
			 */
			unsigned long tlsfunc, offset;

			val &= 0x00ffffff;
			if (val & (1 << 23))
				val = regs->ARM_lr + 4 - (0x1000000 - val) * 4;
			else
				val = regs->ARM_lr + 4 + val * 4;

			tlsfunc = val;

			val = parse_ptabs_read(val, &offset);
			if (val == -EFAULT)
				goto trap_and_emulate;
			val += offset;

			if (*(unsigned long *)val != 0xe3e00a0f)
				goto trap_and_emulate;

			if (unlikely((val & L4_PAGEMASK) != ((val + 4) & L4_PAGEMASK)))
				goto trap_and_emulate;

			if (*(unsigned long *)(val + 4) != 0xe240f01f)
				goto trap_and_emulate;

			*(unsigned long *)(val +  0) = 0xe3a00103; // mov r0, #0xc0000000
			*(unsigned long *)(val +  4) = 0xe240f020; // sub pc, r0, #32

			l4_cache_coherent(val, val + 12);

			regs->ARM_pc = tlsfunc;
			if (USER_PATCH_GETTLS_SHOW)
				printk("  handled (1)\n");
			return 1; // handled
		}

		if (val == 0xe240f01f) {
			unsigned long offset;

			// xxxxc:       e3e00a0f        mvn     r0, #61440      ; 0xf000
			// xxxx0:       e1a0e00f        mov     lr, pc
			// xxxx4:       e240f01f        sub     pc, r0, #31     ; 0x1f
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 8))))
				goto trap_and_emulate;
			if (val != 0xe1a0e00f)
				goto trap_and_emulate;
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 12))))
				goto trap_and_emulate;
			if (val != 0xe3e00a0f)
				goto trap_and_emulate;

			if (unlikely(((regs->ARM_lr - 4) & L4_PAGEMASK) != ((regs->ARM_lr - 12) & L4_PAGEMASK)))
				goto trap_and_emulate;

			val = parse_ptabs_read(regs->ARM_lr - 12, &offset);
			if (val == -EFAULT)
				goto trap_and_emulate;
			val += offset;

			*(unsigned long *)(val + 0) = 0xe3a00103; // mov r0, #0xc0000000
			*(unsigned long *)(val + 8) = 0xe240f020; // sub pc, r0, #32;

			l4_cache_coherent(val, val + 12);

			regs->ARM_pc = regs->ARM_lr - 12;
			if (USER_PATCH_GETTLS_SHOW)
				printk("  handled (2)\n");
			return 1; // handled
		}

trap_and_emulate:
		if (USER_PATCH_GETTLS_SHOW)
			printk("   failed... emulating get-tls-func lr=%lx\n", regs->ARM_lr);
#endif
		regs->ARM_r0 = current_thread_info()->tp_value;
		regs->ARM_pc = regs->ARM_lr;
#ifdef CONFIG_ARM_THUMB
		if (regs->ARM_lr & 1)
			regs->ARM_cpsr |= PSR_T_BIT;
#endif
		return 1; // handled
	}

	if (regs->ARM_pc == 0xffff0fc0 && l4x_l4pfa(t) == 0xffff0fc0) {
		asmlinkage int arm_syscall(int no, struct pt_regs *regs);
#ifdef USER_PATCH_CMPXCHG
		unsigned long val;

		if (USER_PATCH_CMPXCHG_SHOW) {
			printk("CMPXCHG hit at lr=%lx pc=%lx\n", regs->ARM_lr, regs->ARM_pc);
			fiasco_tbuf_log_3val("cmpxchg", regs->ARM_lr, regs->ARM_pc, 0);
#ifdef CONFIG_L4_DEBUG_SEGFAULTS
			l4x_print_vm_area_maps(current, regs->ARM_pc);
#endif
		}

		if (regs->ARM_lr & 1)
			goto trap_and_emulate_cmpxchg;

		if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 4))))
			goto trap_and_emulate_cmpxchg;

		if (val == 0xe243f03f) {
			unsigned long offset;

			// 2fca0:       e3e03a0f        mvn     r3, #61440      ; 0xf000
			// 2fca4:       e1a0e00f        mov     lr, pc
			// 2fca8:       e243f03f        sub     pc, r3, #63     ; 0x3f
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 8))))
				goto trap_and_emulate_cmpxchg;
			if (val != 0xe1a0e00f)
				goto trap_and_emulate_cmpxchg;
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 12))))
				goto trap_and_emulate_cmpxchg;
			if (val != 0xe3e03a0f)
				goto trap_and_emulate_cmpxchg;

			if (unlikely(((regs->ARM_lr - 4) & L4_PAGEMASK) != ((regs->ARM_lr - 12) & L4_PAGEMASK)))
				goto trap_and_emulate_cmpxchg;

			val = parse_ptabs_read(regs->ARM_lr - 12, &offset);
			if (val == -EFAULT)
				goto trap_and_emulate_cmpxchg;
			val += offset;

			*(unsigned long *)(val + 0) = 0xe3a03103;  // mov r3, #0xc0000000
			*(unsigned long *)(val + 8) = 0xe243f040;  // sub pc, r3, #64

			l4_cache_coherent(val, val + 12);

			regs->ARM_pc = regs->ARM_lr - 12;
			if (USER_PATCH_CMPXCHG_SHOW)
				printk("   patched\n");
			return 1; // handled
		}


trap_and_emulate_cmpxchg:
		if (USER_PATCH_CMPXCHG_SHOW)
			printk("   failed... still taking the cmpxchg hit lr=%lx\n", regs->ARM_lr);
#endif
#if 0
		regs->ARM_r0 = arm_syscall(0xfff0 | __ARM_NR_BASE, regs);
		regs->ARM_pc = regs->ARM_lr;
#else
		//printk("light stuff for cmpxchg... from %lx\n", regs->ARM_pc);
		regs->ARM_pc = 0xbfffffc0;
#endif
#ifdef CONFIG_ARM_THUMB
		if (regs->ARM_lr & 1)
			regs->ARM_cpsr |= PSR_T_BIT;
#endif
		return 1; // handled
	}

	if (regs->ARM_pc == 0xffff0fa0 && l4x_l4pfa(t) == 0xffff0fa0) {
#ifdef USER_PATCH_DMB
		// dmb
		unsigned long val;

		if (USER_PATCH_DMB_SHOW) {
			printk("DMB hit at lr=%lx pc=%lx\n", regs->ARM_lr, regs->ARM_pc);
			fiasco_tbuf_log_3val("ARMdmb", regs->ARM_lr, regs->ARM_pc, 0);
		}

		if (regs->ARM_lr & 1)
			goto trap_and_emulate_dmb;

		if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 4))))
			goto trap_and_emulate_dmb;

		if (val == 0xe24cf05f) {
			unsigned long offset;

			// xxxxc:       e3e0ca0f        mvn     ip, #61440      ; 0xf000
			// xxxx0:       e1a0e00f        mov     lr, pc
			// xxxx4:       e24cf05f        sub     pc, ip, #95     ; 0x5f
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 8))))
				goto trap_and_emulate_dmb;
			if (val != 0xe1a0e00f)
				goto trap_and_emulate_dmb;
			if (unlikely(get_user(val, (unsigned long *)(regs->ARM_lr - 12))))
				goto trap_and_emulate_dmb;
			if (val != 0xe3e0ca0f)
				goto trap_and_emulate_dmb;

			if (unlikely(((regs->ARM_lr - 4) & L4_PAGEMASK) != ((regs->ARM_lr - 12) & L4_PAGEMASK)))
				goto trap_and_emulate_dmb;

			val = parse_ptabs_read(regs->ARM_lr - 12, &offset);
			if (val == -EFAULT)
				goto trap_and_emulate_dmb;
			val += offset;

			*(unsigned long *)(val + 0) = 0xe3a0c103;  // mov ip, #0xc0000000
			*(unsigned long *)(val + 8) = 0xe24cf060;  // sub pc, ip, #96     ; 0x60

			l4_cache_coherent(val, val + 12);

			regs->ARM_pc = regs->ARM_lr - 12;
			return 1; // handled
		}

trap_and_emulate_dmb:
		if (USER_PATCH_DMB_SHOW)
			printk("   failed... still taking the dmb hit lr=%lx\n", regs->ARM_lr);

#endif
		regs->ARM_pc = regs->ARM_lr;
		return 1; // handled
	}

	return 0; // not for us
}

static inline int l4x_handle_io_page_fault(struct task_struct *p,
                                           l4_umword_t pfa,
                                           l4_umword_t *d0, l4_umword_t *d1)
{
	return 1;
}

#ifdef CONFIG_L4_VCPU
static inline void l4x_vcpu_entry_user_arch(void)
{
}

static inline bool l4x_vcpu_is_wr_pf(l4_vcpu_state_t *v)
{
	return v->r.err & (1 << 11);
}
#endif

#define __INCLUDED_FROM_L4LINUX_DISPATCH
#include "../dispatch.c"
