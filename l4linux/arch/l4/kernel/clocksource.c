#include <linux/clocksource.h>

#include <l4/re/env.h>

static cycle_t l4x_clk_read(struct clocksource *cs)
{
	return l4re_kip()->clock;
}

static struct clocksource l4x_kipclk_cs = {
	.name           = "l4kipclk",
	.rating         = 100,
	.read           = l4x_clk_read,
	.mask           = CLOCKSOURCE_MASK(64),
	.mult           = 1000,
	.flags          = CLOCK_SOURCE_IS_CONTINUOUS,
};

static int __init init_cs(void)
{
	return clocksource_register(&l4x_kipclk_cs);
}

arch_initcall(init_cs);
