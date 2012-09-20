#include <linux/export.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/clk.h>

#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>

#include <asm/l4lxapi/irq.h>
#include <asm/api/config.h>
#include <asm/generic/irq.h>
#include <asm/generic/devs.h>
#include <asm/generic/setup.h>
#include <asm/generic/timer.h>

#include <l4/sys/cache.h>

static void __init l4x_mach_map_io(void)
{
}

static void __init l4x_mach_fixup(struct tag *tags, char **cmdline, struct meminfo *mi)
{
}

#ifdef CONFIG_DEBUG_LL
#include <l4/sys/kdebug.h>
void printascii(const char *buf)
{
	outstring(buf);
}
#endif

void __init l4x_setup_irq(unsigned int irq)
{
	irq_set_chip_and_handler(irq, &l4x_irq_dev_chip, handle_simple_irq);
	set_irq_flags(irq, IRQF_VALID);
	l4x_alloc_irq_desc_data(irq);
}

static void __init l4x_mach_init_irq(void)
{
	int i;

	/* Call our generic IRQ handling code */
	l4lx_irq_init();

	for (i = 0; i < NR_IRQS; i++)
		l4x_setup_irq(i);
}

#ifdef CONFIG_L4_CLK_NOOP
#if 0
int clk_enable(struct clk *clk)
{
	printk("%s %d\n", __func__, __LINE__);
        return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	printk("%s %d\n", __func__, __LINE__);
}
EXPORT_SYMBOL(clk_disable);
#endif
#endif

int dma_needs_bounce(struct device *d, dma_addr_t a, size_t s)
{
	return 1;
}


#ifdef CONFIG_OUTER_CACHE
static void l4x_outer_cache_inv_range(unsigned long start, unsigned long end)
{
	l4_cache_l2_inv(start, end);
}

static void l4x_outer_cache_clean_range(unsigned long start, unsigned long end)
{
	l4_cache_l2_clean(start, end);
}

static void l4x_outer_cache_flush_range(unsigned long start, unsigned long end)
{
	l4_cache_l2_flush(start, end);
}

static void l4x_outer_cache_flush_all(void)
{
	printk("%s called by %p\n", __func__, __builtin_return_address(0));
}

static void l4x_outer_cache_inv_all(void)
{
	printk("%s called by %p\n", __func__, __builtin_return_address(0));
}

static void l4x_outer_cache_disable(void)
{
	printk("%s called by %p\n", __func__, __builtin_return_address(0));
}

static void l4x_outer_cache_set_debug(unsigned long val)
{
	printk("%s called by %p\n", __func__, __builtin_return_address(0));
}

static void __init l4x_setup_outer_cache(void)
{
	outer_cache.inv_range   = l4x_outer_cache_inv_range;
	outer_cache.clean_range = l4x_outer_cache_clean_range;
	outer_cache.flush_range = l4x_outer_cache_flush_range;
	outer_cache.flush_all   = l4x_outer_cache_flush_all;
	outer_cache.inv_all     = l4x_outer_cache_inv_all;
	outer_cache.disable     = l4x_outer_cache_disable;
	outer_cache.set_debug   = l4x_outer_cache_set_debug;
}
#endif

static void __init l4x_mach_init(void)
{
#ifdef CONFIG_OUTER_CACHE
	l4x_setup_outer_cache();
#endif


	l4x_arm_devices_early_init();
	l4x_arm_devices_init();
}

static inline void l4x_stop(char mode, const char *cmd)
{
	local_irq_disable();
	l4x_exit_l4linux();
}


struct sys_timer l4x_mach_timer = {
	.init		= l4x_timer_init,
};

MACHINE_START(L4, "L4")
	.atag_offset    = 0,
	.fixup		= l4x_mach_fixup,
	.map_io		= l4x_mach_map_io,
	.init_irq	= l4x_mach_init_irq,
	.timer		= &l4x_mach_timer,
	.init_machine	= l4x_mach_init,
	.restart	= l4x_stop,
MACHINE_END

/*
 * We only have one machine description for now, so keep lookup_machine_type
 * simple.
 */
const struct machine_desc *lookup_machine_type(unsigned int x)
{
	return &__mach_desc_L4;
}

#ifdef CONFIG_SMP
#include <asm/generic/smp_ipi.h>

void l4x_raise_softirq(const struct cpumask *mask, unsigned ipi)
{
	int cpu;

	for_each_cpu(cpu, mask) {
		l4x_cpu_ipi_enqueue_vector(cpu, ipi);
		l4x_cpu_ipi_trigger(cpu);
	}
}
#endif
