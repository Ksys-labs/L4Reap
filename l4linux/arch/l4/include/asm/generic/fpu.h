#ifndef __ASM_L4__GENERIC__FPU_H__
#define __ASM_L4__GENERIC__FPU_H__

void l4x_fpu_set(int on_off);
struct l4x_arch_cpu_fpu_state *l4x_fpu_get(unsigned cpu);

#endif /* ! __ASM_L4__GENERIC__FPU_H__ */
