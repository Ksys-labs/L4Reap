#include <asm/io.h>

#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irq.h>

#include <l4/sys/irq.h>
#include <l4/sys/factory.h>

#include <asm/api/config.h>
#include <asm/api/macros.h>

#include <asm/l4lxapi/irq.h>
#include <asm/l4lxapi/thread.h>
#include <asm/l4lxapi/misc.h>

#include <asm/generic/io.h>
#include <asm/generic/sched.h>
#include <asm/generic/setup.h>
#include <asm/generic/task.h>
#include <asm/generic/do_irq.h>
#include <asm/generic/cap_alloc.h>
#include <asm/generic/stack_id.h>
#include <asm/generic/smp.h>
#include <asm/generic/irq.h>

#include <l4/re/c/namespace.h>
#include <l4/log/log.h>
#include <l4/sys/debugger.h>

#define d_printk(format, args...)  printk(format , ## args)
#define dd_printk(format, args...) do { if (0) printk(format , ## args); } while (0)


/*
 * Return the priority of an interrupt thread.
 */
int l4lx_irq_prio_get(unsigned int irq)
{
	if (irq == 0)
		return CONFIG_L4_PRIO_IRQ_BASE + 1;
	if (irq < NR_IRQS)
		return CONFIG_L4_PRIO_IRQ_BASE;

	enter_kdebug("l4lx_irq_prio_get: wrong IRQ!");
	return -1;
}

static inline void attach_to_irq(struct irq_desc *desc)
{
	long ret;
	unsigned long flags;
	struct l4x_irq_desc_private *p = irq_desc_get_chip_data(desc);

	local_irq_save(flags);
	if ((ret  = l4_error(l4_irq_attach(p->irq_cap, irq_desc_get_irq_data(desc)->irq << 2,
	                                   l4x_cpu_thread_get_cap(p->cpu)))))
		dd_printk("%s: can't register to irq %u: return=%ld\n",
		          __func__, irq_desc_get_irq_data(desc)->irq, ret);
	local_irq_restore(flags);
}

static void detach_from_interrupt(struct irq_desc *desc)
{
	struct l4x_irq_desc_private *p = irq_desc_get_chip_data(desc);
	unsigned long flags;

	local_irq_save(flags);
	if (l4_error(l4_irq_detach(p->irq_cap)))
		dd_printk("%02d: Unable to detach from IRQ\n",
		          irq_desc_get_irq_data(desc)->irq);
	local_irq_restore(flags);
}

void l4lx_irq_init(void)
{
}

unsigned int l4lx_irq_dev_startup(struct irq_data *data)
{
	unsigned irq = data->irq;
	struct l4x_irq_desc_private *p = irq_get_chip_data(irq);

	/* First test whether a capability has been registered with
	 * this IRQ number */
	p->irq_cap = l4x_have_irqcap(irq);
	if (l4_is_invalid_cap(p->irq_cap)) {
		/* No, get IRQ from IO service */
		unsigned long irq_f;
		local_irq_save(irq_f);
		p->irq_cap = l4x_cap_alloc();
		if (l4_is_invalid_cap(p->irq_cap)
		    || l4io_request_irq(irq, p->irq_cap)) {
			/* "reset" handler ... */
			//irq_desc[irq].chip = &no_irq_type;
			/* ... and bail out  */
			LOG_printf("irq-startup: did not get irq %d\n", irq);
			local_irq_restore(irq_f);
			return 0;
		}
		local_irq_restore(irq_f);
	}

	l4x_irq_set_type_at_icu(irq, p->trigger);

	l4lx_irq_dev_enable(data);
	return 1;
}

void l4lx_irq_dev_shutdown(struct irq_data *data)
{
	unsigned irq = data->irq;
	struct l4x_irq_desc_private *p = irq_get_chip_data(irq);

	dd_printk("%s: %u\n", __func__, irq);
	l4lx_irq_dev_disable(data);

	if (l4_is_invalid_cap(l4x_have_irqcap(irq)))
		l4io_release_irq(irq, p->irq_cap);
}

void l4lx_irq_dev_enable(struct irq_data *data)
{
	struct irq_desc *desc = irq_to_desc(data->irq);
	struct l4x_irq_desc_private *p = irq_desc_get_chip_data(desc);

	dd_printk("%s: %u\n", __func__, data->irq);

	p->enabled = 1;
	attach_to_irq(desc);
	l4lx_irq_dev_eoi(data);
}

void l4lx_irq_dev_disable(struct irq_data *data)
{
	struct irq_desc *desc = irq_to_desc(data->irq);
	struct l4x_irq_desc_private *p = irq_desc_get_chip_data(desc);

	dd_printk("%s: %u\n", __func__, data->irq);

	p->enabled = 0;
	detach_from_interrupt(desc);
}

void l4lx_irq_dev_ack(struct irq_data *data)
{
	dd_printk("%s: %u\n", __func__, data->irq);
}

void l4lx_irq_dev_mask(struct irq_data *data)
{
	dd_printk("%s: %u\n", __func__, data->irq);
}

void l4lx_irq_dev_unmask(struct irq_data *data)
{
	dd_printk("%s: %u\n", __func__, data->irq);
}

void l4lx_irq_dev_eoi(struct irq_data *data)
{
	struct l4x_irq_desc_private *p = irq_get_chip_data(data->irq);
	unsigned long flags;

	dd_printk("%s: %u\n", __func__, data->irq);
	local_irq_save(flags);
	l4_irq_unmask(p->irq_cap);
	local_irq_restore(flags);
}

#ifdef CONFIG_SMP
static spinlock_t migrate_lock;

int l4lx_irq_dev_set_affinity(struct irq_data *data,
                              const struct cpumask *dest, bool force)
{
        unsigned target_cpu;
	unsigned long flags;
	struct irq_desc *desc = irq_to_desc(data->irq);
	struct l4x_irq_desc_private *p = irq_desc_get_chip_data(desc);

	if (!p->irq_cap)
		return 0;

	if (!cpumask_intersects(dest, cpu_online_mask))
                return 1;

        target_cpu = cpumask_any_and(dest, cpu_online_mask);

	if (target_cpu == p->cpu)
		return 0;

	spin_lock_irqsave(&migrate_lock, flags);
	detach_from_interrupt(desc);

	cpumask_copy(irq_desc_get_irq_data(desc)->affinity, dest);
	p->cpu = target_cpu;
	attach_to_irq(desc);
	if (p->enabled);
		l4_irq_unmask(p->irq_cap);
	spin_unlock_irqrestore(&migrate_lock, flags);

	return 0;
}
#endif
