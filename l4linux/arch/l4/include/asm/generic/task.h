#ifndef __ASM_L4__GENERIC__TASK_H__
#define __ASM_L4__GENERIC__TASK_H__

#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

#include <l4/sys/types.h>

#include <asm/api/config.h>

#define COPY_THREAD_STACK_SIZE___FLAG_INKERNEL 0     // because 0 in fork.c
#define COPY_THREAD_STACK_SIZE___FLAG_USER     0x49

/* Send SIGKILL to current */
void l4x_sig_current_kill(void);

#ifdef CONFIG_SMP
#include <asm/generic/smp.h>
#define l4x_idle_task(cpu) l4x_cpu_idle_get(cpu)
#else
#define l4x_idle_task(cpu) (&init_task)
#endif

DECLARE_PER_CPU(struct thread_info *, l4x_current_ti);

void l4x_exit_thread(void);

#endif /* ! __ASM_L4__GENERIC__TASK_H__ */
