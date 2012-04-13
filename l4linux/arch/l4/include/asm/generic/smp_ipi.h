#ifndef __ASM_L4__GENERIC__SMP_IPI_H__
#define __ASM_L4__GENERIC__SMP_IPI_H__

void l4x_send_IPI_mask_bitmask(unsigned long, int);
void l4x_cpu_ipi_setup(unsigned cpu);
void l4x_cpu_ipi_enqueue_vector(unsigned cpu, unsigned vector);
void l4x_cpu_ipi_trigger(unsigned cpu);
void l4x_smp_broadcast_timer(void);

#ifdef CONFIG_HOTPLUG_CPU
void l4x_cpu_ipi_stop(unsigned cpu);
#endif

#endif /* __ASM_L4__GENERIC__SMP_IPI_H__ */
