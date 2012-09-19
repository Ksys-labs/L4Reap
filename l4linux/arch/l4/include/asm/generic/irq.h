#ifndef __ASM_L4__GENERIC__IRQ_H__
#define __ASM_L4__GENERIC__IRQ_H__

#include <asm/generic/kthreads.h>

#ifdef CONFIG_L4_VCPU
#define L4X_VCPU_IRQ_IPI (NR_IRQS)
#else
#define L4_IRQ_DISABLED 0
#define L4_IRQ_ENABLED  1
#endif

int l4x_register_irq(l4_cap_idx_t irqcap);
void l4x_unregister_irq(int irqnum);
l4_cap_idx_t l4x_have_irqcap(int irqnum);

struct l4x_irq_desc_private {
	l4_cap_idx_t irq_cap;
#ifndef CONFIG_L4_VCPU
	l4lx_thread_t irq_thread;
#endif
	unsigned enabled;
	unsigned cpu;
	unsigned char trigger;
};

int l4x_alloc_irq_desc_data(int irq);

void l4x_init_IRQ(void);

extern struct irq_chip l4x_irq_dev_chip;
extern struct irq_chip l4x_irq_timer_chip;

void l4x_irq_set_type_at_icu(unsigned irq, unsigned type);

#endif /* ! __ASM_L4__GENERIC__IRQ_H__ */
