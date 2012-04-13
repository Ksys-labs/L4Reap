#ifndef __ASM_L4__GENERIC__SMP_H__
#define __ASM_L4__GENERIC__SMP_H__

#include <l4/sys/types.h>
#include <asm/l4lxapi/thread.h>

#ifdef CONFIG_SMP

#include <linux/sched.h>
#include <linux/bitops.h>

#include <asm/generic/smp_ipi.h>

#define L4X_TIMER_VECTOR	9

extern unsigned int l4x_nr_cpus;

void l4x_smp_process_IPI(int vector, struct pt_regs *regs);

void l4x_cpu_spawn(int cpu, struct task_struct *idle);
void l4x_cpu_release(int cpu);
struct task_struct *l4x_cpu_idle_get(int cpu);

void l4x_migrate_thread(l4_cap_idx_t thread, unsigned from_cpu, unsigned to_cpu);

#ifdef CONFIG_HOTPLUG_CPU
void l4x_cpu_dead(void);
void l4x_destroy_ugate(unsigned cpu);
void l4x_shutdown_cpu(unsigned cpu);
#ifdef ARCH_x86
unsigned l4x_utcb_get_orig_segment(void);
#endif
#endif

#ifdef ARCH_x86
void l4x_load_percpu_gdt_descriptor(struct desc_struct *gdt);
#endif

#ifdef ARCH_arm
void l4x_raise_softirq(const struct cpumask *mask, unsigned ipi);
#endif

#else
/* UP Systems */

//#include <asm/generic/kthreads.h>

static inline int l4x_IPI_pending_tac(int cpu)
{
	return 0;
}

static inline int l4x_IPI_is_ipi_message(l4_umword_t d0)
{
	return 0;
}

static inline void l4x_smp_broadcast_timer(void)
{
}
#endif

unsigned l4x_cpu_physmap_get_id(unsigned lcpu);

#endif /* ! __ASM_L4__GENERIC__SMP_H__ */
