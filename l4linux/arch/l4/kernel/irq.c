
#include <linux/bitops.h>
#include <linux/spinlock.h>

#include <asm/ptrace.h>
#include <asm/irq.h>

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
