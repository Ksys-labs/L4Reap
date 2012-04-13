#ifndef ASMARM_ARCH_SMP_H
#define ASMARM_ARCH_SMP_H

#include <asm/generic/smp_ipi.h>
#include <asm/generic/smp_id.h>

#define hard_smp_processor_id() (l4x_cpu_cpu_get())

#endif
