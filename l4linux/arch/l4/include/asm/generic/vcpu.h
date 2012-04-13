#ifndef __ASM_L4__GENERIC__VCPU_H__
#define __ASM_L4__GENERIC__VCPU_H__

#include <linux/threads.h>
#include <l4/sys/vcpu.h>

extern l4_vcpu_state_t *l4x_vcpu_states[NR_CPUS];

static inline
l4_vcpu_state_t *l4x_vcpu_state(int cpu)
{
	return l4x_vcpu_states[cpu];
}

#ifdef CONFIG_L4_VCPU

#include <linux/irqflags.h>
#include <linux/threads.h>
#include <linux/linkage.h>
#include <asm/ptrace.h>

#define  L4XV_V(n) unsigned long n
#define  L4XV_L(n) local_irq_save(n)
#define  L4XV_U(n) local_irq_restore(n)

void l4x_vcpu_handle_irq(l4_vcpu_state_t *t, struct pt_regs *regs);
void l4x_vcpu_handle_ipi(struct pt_regs *regs);
asmlinkage void l4x_vcpu_entry(void);

static inline void l4x_vcpu_init(l4_vcpu_state_t *v)
{
	v->state     = L4_VCPU_F_EXCEPTIONS;
	v->entry_ip  = (l4_addr_t)&l4x_vcpu_entry;
	v->user_task = L4_INVALID_CAP;
}

#else

#define  L4XV_V(n)
#define  L4XV_L(n) do {} while (0)
#define  L4XV_U(n) do {} while (0)

#endif

#endif /* ! __ASM_L4__GENERIC__VCPU_H__ */
