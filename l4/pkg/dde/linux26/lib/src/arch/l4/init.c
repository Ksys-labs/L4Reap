#include "local.h"

#include <l4/dde/linux26/dde26.h>
#include <l4/dde/dde.h>

#define DEBUG_PCI(msg, ...)	ddekit_printf( "\033[33m"msg"\033[0m\n", ##__VA_ARGS__)

/* Didn't know where to put this. */
unsigned long __per_cpu_offset[NR_CPUS];

extern void driver_init(void);
extern int classes_init(void);

void __init __attribute__((used)) l4dde26_init(void)
{
	/* first, initialize DDEKit */
	ddekit_init();

	l4dde26_kmalloc_init();

	/* Init Linux driver framework before trying to add PCI devs to the bus */
	driver_init();

	printk("Initialized DDELinux 2.6\n");
}

#if 0
void l4dde26_do_initcalls(void)
{
	/* finally, let DDEKit perform all the initcalls */
	ddekit_do_initcalls();
}
#endif

dde_initcall(l4dde26_init);
