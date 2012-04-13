#ifndef __ASM_L4__GENERIC__DO_IRQ_H__
#define __ASM_L4__GENERIC__DO_IRQ_H__

#include <linux/spinlock.h>
#include <linux/thread_info.h>

#include <asm/irq.h>

#include <asm/generic/sched.h>
#include <asm/generic/task.h>
#include <asm/l4lxapi/irq.h>
#include <asm/l4x/exception.h>

#ifndef CONFIG_L4_VCPU
static inline void l4x_do_IRQ(int irq, struct thread_info *ctx)
{
	unsigned long flags, old_cpu_state;
	struct pt_regs *r;
	int cpu = smp_processor_id();

	local_irq_save(flags);
	ctx->task = per_cpu(l4x_current_ti, cpu)->task;
	ctx->preempt_count = per_cpu(l4x_current_ti, cpu)->preempt_count;
	r = &per_cpu(l4x_current_ti, cpu)->task->thread.regs;
	old_cpu_state = l4x_get_cpu_mode(r);
	l4x_set_cpu_mode(r, l4x_in_kernel() ? L4X_MODE_KERNEL : L4X_MODE_USER);
#ifdef CONFIG_X86
	do_IRQ(irq, &per_cpu(l4x_current_ti, cpu)->task->thread.regs);
#else
	asm_do_IRQ(irq, &per_cpu(l4x_current_ti, cpu)->task->thread.regs);
#endif
	l4x_set_cpu_mode(r, old_cpu_state);
	local_irq_restore(flags);

	l4x_wakeup_idle_if_needed();
}

#ifdef CONFIG_SMP
#include <asm/generic/smp.h>

static inline void l4x_do_IPI(int vector, struct thread_info *ctx)
{
	unsigned long flags, old_cpu_state;
	struct pt_regs *r;
	int cpu = smp_processor_id();

	local_irq_save(flags);
	ctx->task = per_cpu(l4x_current_ti, cpu)->task;
	ctx->preempt_count = per_cpu(l4x_current_ti, cpu)->preempt_count;
	r = &per_cpu(l4x_current_ti, cpu)->task->thread.regs;
	old_cpu_state = l4x_get_cpu_mode(r);
	l4x_set_cpu_mode(r, l4x_in_kernel() ? L4X_MODE_KERNEL : L4X_MODE_USER);
	l4x_smp_process_IPI(vector, &per_cpu(l4x_current_ti, cpu)->task->thread.regs);
	l4x_set_cpu_mode(r, old_cpu_state);
	local_irq_restore(flags);

	l4x_wakeup_idle_if_needed();
}
#endif
#endif /* !vcpu */

#endif /* ! __ASM_L4__GENERIC__DO_IRQ_H__ */
