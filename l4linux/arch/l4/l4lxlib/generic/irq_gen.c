#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/l4lxapi/generic/irq_gen.h>

unsigned int l4lx_irq_max; ///< highest IRQ no + 1 available in the system

int l4x_alloc_irq_desc_data(int irq)
{
	struct l4x_irq_desc_private *p;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	return irq_set_chip_data(irq, p);
}
