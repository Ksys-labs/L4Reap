#ifndef __ASM_L4__GENERIC__STACK_ID_H__
#define __ASM_L4__GENERIC__STACK_ID_H__

enum {
	L4X_UTCB_TCR_ID   = 0,
	L4X_UTCB_TCR_PRIO = 1,
};

#include <asm/thread_info.h>
#include <asm/generic/vcpu.h>

#ifndef CONFIG_L4_VCPU

struct l4x_stack_struct {
	l4_utcb_t *utcb;
};

extern l4_utcb_t *l4x_cpu_threads[NR_CPUS];

static inline
struct l4x_stack_struct * l4x_stack_struct_get(struct thread_info *ti)
{
	return (struct l4x_stack_struct *)(ti + 1);
}

static inline void l4x_stack_set(struct thread_info *ti, l4_utcb_t *u)
{
	struct l4x_stack_struct *s = l4x_stack_struct_get(ti);
	s->utcb = u;
}

static inline l4_utcb_t *l4x_utcb_current(void)
{
	return l4x_stack_struct_get(current_thread_info())->utcb;
}

#else

extern l4_utcb_t *l4x_cpu_threads[NR_CPUS];

static inline void l4x_stack_set(struct thread_info *ti, l4_utcb_t *u)
{
}

static inline l4_utcb_t *l4x_utcb_current(void)
{
	return l4x_cpu_threads[current_thread_info()->cpu];
}

static inline
l4_vcpu_state_t *l4x_vcpu_state_current(void)
{
	return l4x_vcpu_states[current_thread_info()->cpu];
}
#endif

static inline l4_cap_idx_t l4x_cap_current(void)
{
	return l4_utcb_tcr_u(l4x_utcb_current())->user[L4X_UTCB_TCR_ID];
}

static inline unsigned int l4x_prio_current(void)
{
	return l4_utcb_tcr_u(l4x_utcb_current())->user[L4X_UTCB_TCR_PRIO];
}

#endif /* ! __ASM_L4__GENERIC__STACK_ID_H__ */
