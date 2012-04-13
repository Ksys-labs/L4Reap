/*
 * linux/include/asm-l4/arch-arm/arch/system.h
 */
#ifndef __ASM_L4__ARCH_ARM__ARCH__SYSTEM_H__
#define __ASM_L4__ARCH_ARM__ARCH__SYSTEM_H__

#include <linux/kernel.h>
#include <asm/generic/setup.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	local_irq_disable();
	l4x_exit_l4linux();
}
#endif /* ! __ASM_L4__ARCH_ARM__ARCH__SYSTEM_H__ */
