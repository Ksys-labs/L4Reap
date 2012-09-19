
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/irq.h>

#include <asm/ptrace.h>
#include <asm/irq.h>
#include <asm/generic/irq.h>
#include <asm/l4lxapi/irq.h>

#include <l4/sys/types.h>

enum {
	NR_REQUESTABLE = NR_IRQS - NR_IRQS_HW,
	BASE = NR_IRQS_HW,
};

static l4_cap_idx_t caps[NR_REQUESTABLE];
static int init_done;
static DEFINE_SPINLOCK(lock);

static void init_array(void)
{
	int i;

	BUG_ON(NR_REQUESTABLE < 1);

	for (i = 0; i < NR_REQUESTABLE; ++i)
		caps[i] = L4_INVALID_CAP;

	init_done = 1;
}


int l4x_register_irq(l4_cap_idx_t irqcap)
{
	unsigned long flags;
	int i, ret = -1;

	if (!init_done)
		init_array();

	spin_lock_irqsave(&lock, flags);

	for (i = 0; i < NR_REQUESTABLE; ++i) {
		if (l4_is_invalid_cap(caps[i])) {
			caps[i] = irqcap;
			ret = i + BASE;
			break;
		}
	}
	spin_unlock_irqrestore(&lock, flags);

	return ret;
}

void l4x_unregister_irq(int irqnum)
{
	if (irqnum >= BASE && (irqnum - BASE) < NR_REQUESTABLE)
		caps[irqnum - BASE] = L4_INVALID_CAP;
}

l4_cap_idx_t l4x_have_irqcap(int irqnum)
{
	if (!init_done)
		init_array();

	if (irqnum >= BASE && (irqnum - BASE) < NR_REQUESTABLE)
		return caps[irqnum - BASE];

	return L4_INVALID_CAP;
}

struct irq_chip l4x_irq_dev_chip = {
	.name                   = "L4-irq",
	.irq_startup            = l4lx_irq_dev_startup,
	.irq_shutdown           = l4lx_irq_dev_shutdown,
	.irq_enable             = l4lx_irq_dev_enable,
	.irq_disable            = l4lx_irq_dev_disable,
	.irq_ack                = l4lx_irq_dev_ack,
	.irq_mask               = l4lx_irq_dev_mask,
	.irq_unmask             = l4lx_irq_dev_unmask,
	.irq_eoi                = l4lx_irq_dev_eoi,
	.irq_set_type           = l4lx_irq_set_type,
#ifdef CONFIG_L4_VCPU
#ifdef CONFIG_SMP
	.irq_set_affinity       = l4lx_irq_dev_set_affinity,
#endif
#endif
};

#if defined(CONFIG_X86) && defined(CONFIG_SMP)
#include <linux/interrupt.h>
#include <linux/sched.h>

void l4x_smp_timer_interrupt(struct pt_regs *regs)
{
	struct pt_regs *oldregs;
	unsigned long flags;
	oldregs = set_irq_regs(regs);

	local_irq_save(flags);
	irq_enter();
	profile_tick(CPU_PROFILING);
	update_process_times(user_mode_vm(get_irq_regs()));
	irq_exit();
	local_irq_restore(flags);
	set_irq_regs(oldregs);
}
#endif
