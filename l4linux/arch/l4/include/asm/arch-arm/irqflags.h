#ifndef __ASM__L4__ARCH_ARM__IRQFLAGS_H__
#define __ASM__L4__ARCH_ARM__IRQFLAGS_H__

#ifdef __KERNEL__

#include <asm/ptrace.h>

/*
 * CPU interrupt mask handling.
 */
#ifndef  CONFIG_L4
#if __LINUX_ARM_ARCH__ >= 6

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

static inline void arch_local_irq_enable(void)
{
	asm volatile(
		"	cpsie i			@ arch_local_irq_enable"
		:
		:
		: "memory", "cc");
}

static inline void arch_local_irq_disable(void)
{
	asm volatile(
		"	cpsid i			@ arch_local_irq_disable"
		:
		:
		: "memory", "cc");
}

#define local_fiq_enable()  __asm__("cpsie f	@ __stf" : : : "memory", "cc")
#define local_fiq_disable() __asm__("cpsid f	@ __clf" : : : "memory", "cc")
#else

/*
 * Save the current interrupt enable state & disable IRQs
 */
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags, temp;

	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_save\n"
		"	orr	%1, %0, #128\n"
		"	msr	cpsr_c, %1"
		: "=r" (flags), "=r" (temp)
		:
		: "memory", "cc");
	return flags;
}

/*
 * Enable IRQs
 */
static inline void arch_local_irq_enable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_enable\n"
		"	bic	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Disable IRQs
 */
static inline void arch_local_irq_disable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_disable\n"
		"	orr	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Enable FIQs
 */
#define local_fiq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Disable FIQs
 */
#define local_fiq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

#endif
#else /* L4 */

#include <asm/generic/irq.h>
#include <asm/generic/vcpu.h>

extern void l4x_global_cli(void);
extern void l4x_global_sti(void);
extern unsigned long l4x_global_save_flags(void);
extern void l4x_global_restore_flags(unsigned long flags);

static inline unsigned long arch_local_save_flags(void)
{
	return l4x_global_save_flags();
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	l4x_global_restore_flags(flags);
}

static inline void arch_local_irq_disable(void)
{
	l4x_global_cli();
}

static inline void arch_local_irq_enable(void)
{
	l4x_global_sti();
}

#define local_fiq_enable()	do { } while (0)

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags = arch_local_save_flags();
	arch_local_irq_disable();
	return flags;
}

#endif /* L4 */

#ifndef CONFIG_L4
/*
 * Save the current interrupt enable state.
 */
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile(
		"	mrs	%0, cpsr	@ local_save_flags"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

/*
 * restore saved IRQ & FIQ state
 */
static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
		"	msr	cpsr_c, %0	@ local_irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return flags & PSR_I_BIT;
}

#else /* L4 */

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
#ifdef CONFIG_L4_VCPU
	return !(flags & L4_VCPU_F_IRQ);
#else
	return flags == L4_IRQ_DISABLED;
#endif
}
#endif /* L4 */

#endif /* __KERNEL__ */
#endif /* __ASM__L4__ARCH_ARM__IRQFLAGS_H__ */
