/*
 * This header files defines the IRQ functions which need to be provided by
 * every implementation of this interface.
 * The functions mostly correspond to the "struct hw_interrupt_type"
 * members.
 *
 *
 */
#ifndef __ASM_L4__L4LXAPI__IRQ_H__
#define __ASM_L4__L4LXAPI__IRQ_H__

#include <linux/irq.h>
#include <asm/l4lxapi/generic/irq_gen.h>

/**
 * \defgroup irq Interrupt handling functionality.
 * \ingroup l4lxapi
 */

/**
 * \brief Initialize the interrupt handling.
 * \ingroup irq
 */
void l4lx_irq_init(void);

/**
 * \brief Get defined priority of a certain interrupt thread.
 * \ingroup irq
 *
 * \param	irq	Interrupt.
 * \return	Defined priority of the interrupt thread.
 *
 * Every API implementation has to define this function which
 * returns the priority of the specific interrupt thread. This function does
 * not return the actual thread priority!
 */
int l4lx_irq_prio_get(unsigned int irq);

unsigned int l4lx_irq_dev_startup(struct irq_data *data);
void l4lx_irq_dev_shutdown(struct irq_data *data);
int l4lx_irq_set_type(struct irq_data *data, unsigned int type);
void l4lx_irq_dev_enable(struct irq_data *data);
void l4lx_irq_dev_disable(struct irq_data *data);
void l4lx_irq_dev_ack(struct irq_data *data);
void l4lx_irq_dev_mask(struct irq_data *data);
void l4lx_irq_dev_unmask(struct irq_data *data);
int l4lx_irq_dev_set_affinity(struct irq_data *data,
                              const struct cpumask *dest, bool force);
void l4lx_irq_dev_eoi(struct irq_data *data);

unsigned int l4lx_irq_timer_startup(struct irq_data *data);
void l4lx_irq_timer_shutdown(struct irq_data *data);
void l4lx_irq_timer_enable(struct irq_data *data);
void l4lx_irq_timer_disable(struct irq_data *data);
void l4lx_irq_timer_ack(struct irq_data *data);
void l4lx_irq_timer_mask(struct irq_data *data);
void l4lx_irq_timer_unmask(struct irq_data *data);
int l4lx_irq_timer_set_affinity(struct irq_data *data, const struct cpumask *dest);

#endif /* ! __ASM_L4__L4LXAPI__IRQ_H__ */
