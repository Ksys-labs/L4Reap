/*
 *  linux/arch/l4/kernel/reboot.c
 */

#include <linux/module.h>

#include <asm/generic/memory.h>
#include <asm/generic/setup.h>

#include <asm/reboot.h>

/*
 * Power off function, if any
 */
void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

int reboot_force;

bool port_cf9_safe = false;

void machine_halt(void)
{
	local_irq_disable();
	l4x_exit_l4linux();
}

void machine_emergency_restart(void)
{
	machine_halt();
}

void machine_restart(char *__unused)
{
	machine_halt();
}

void machine_power_off(void)
{
	machine_halt();
}

struct machine_ops machine_ops = {
	.power_off = machine_power_off,
	.shutdown =  machine_halt,
	.emergency_restart = machine_emergency_restart,
	.restart = machine_restart,
	.halt = machine_halt,
};
