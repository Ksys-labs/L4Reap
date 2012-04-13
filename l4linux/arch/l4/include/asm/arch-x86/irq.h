#ifndef __ASM_L4__ARCH_X86__IRQ_H__
#define __ASM_L4__ARCH_X86__IRQ_H__
/*
 *	(C) 1992, 1993 Linus Torvalds, (C) 1997 Ingo Molnar
 *
 *	IRQ/IPI changes taken from work by Thomas Radke
 *	<tomsoft@informatik.tu-chemnitz.de>
 */

#include <asm/apicdef.h>
#include <asm/irq_vectors.h>

/* the defines are from irq_vectors out of the mach-default directory */
#define NR_IRQS_HW		64

static inline int irq_canonicalize(int irq)
{
	return ((irq == 2) ? 9 : irq);
}

#ifdef CONFIG_X86_32
extern void irq_ctx_init(int cpu);
#else
# define irq_ctx_init(cpu) do { } while (0)
#endif

#define __ARCH_HAS_DO_SOFTIRQ

#ifdef CONFIG_HOTPLUG_CPU
#include <linux/cpumask.h>
extern void fixup_irqs(void);
extern void irq_force_complete_move(int);
#endif

extern void (*x86_platform_ipi_callback)(void);
extern void native_init_IRQ(void);
extern bool handle_irq(unsigned irq, struct pt_regs *regs);

extern unsigned int do_IRQ(int irq, struct pt_regs *regs);

/* Interrupt vector management */
extern DECLARE_BITMAP(used_vectors, NR_VECTORS);
extern int vector_used_by_percpu_irq(unsigned int vector);

extern void init_ISA_irqs(void);

#endif /* __ASM_L4__ARCH_X86__IRQ_H__ */
