#ifndef __ASM_L4__L4X_ARM__EXCEPTION_H__
#define __ASM_L4__L4X_ARM__EXCEPTION_H__

#include <asm/ptrace.h>

#include <l4/sys/utcb.h>

enum l4x_cpu_modes {
	L4X_MODE_KERNEL = SYSTEM_MODE,
	L4X_MODE_USER   = USR_MODE,
};

static inline void l4x_set_cpu_mode(struct pt_regs *r, enum l4x_cpu_modes mode)
{
	r->ARM_cpsr = (r->ARM_cpsr & ~MODE_MASK) | (mode & MODE_MASK);
}

static inline void l4x_set_user_mode(struct pt_regs *r)
{
	l4x_set_cpu_mode(r, USR_MODE);
}

static inline void l4x_set_kernel_mode(struct pt_regs *r)
{
	l4x_set_cpu_mode(r, SYSTEM_MODE);
}

static inline unsigned long l4x_get_cpu_mode(struct pt_regs *r)
{
	return processor_mode(r);
}

static inline void l4x_make_up_kernel_regs(struct pt_regs *r)
{
	register unsigned long sp asm("sp");
	raw_local_save_flags(r->ARM_cpsr);
	l4x_set_cpu_mode(r, L4X_MODE_KERNEL);
	r->ARM_pc = (unsigned long)__builtin_return_address(0);
	r->ARM_sp = sp;
}

#ifdef CONFIG_L4_VCPU
static inline void vcpu_to_ptregs(l4_vcpu_state_t *v,
                                  struct pt_regs *regs)
{
	regs->uregs[0]  = v->r.r[0];
	regs->uregs[1]  = v->r.r[1];
	regs->uregs[2]  = v->r.r[2];
	regs->uregs[3]  = v->r.r[3];
	regs->uregs[4]  = v->r.r[4];
	regs->uregs[5]  = v->r.r[5];
	regs->uregs[6]  = v->r.r[6];
	regs->uregs[7]  = v->r.r[7];
	regs->uregs[8]  = v->r.r[8];
	regs->uregs[9]  = v->r.r[9];
	regs->uregs[10] = v->r.r[10];
	regs->uregs[11] = v->r.r[11];
	regs->uregs[12] = v->r.r[12];
	regs->ARM_sp    = v->r.sp;
	regs->ARM_lr    = v->r.lr;
	regs->ARM_pc    = v->r.ip;
	if (v->saved_state & L4_VCPU_F_IRQ)
		regs->ARM_cpsr = v->r.flags & ~PSR_I_BIT;
	else
		regs->ARM_cpsr = v->r.flags | PSR_I_BIT;
	// disable IRQ and FIQ, for valid_user_regs(regsp) + mode
	if (v->saved_state & L4_VCPU_F_USER_MODE)
		regs->ARM_cpsr &= ~(PSR_F_BIT | PSR_I_BIT | (MODE_MASK ^ USR_MODE));
	else
		regs->ARM_cpsr = (regs->ARM_cpsr & ~MODE_MASK) | SVC_MODE;
}

static inline void ptregs_to_vcpu(l4_vcpu_state_t *v,
                                  struct pt_regs *regs)
{
	v->r.r[0]  = regs->uregs[0];
	v->r.r[1]  = regs->uregs[1];
	v->r.r[2]  = regs->uregs[2];
	v->r.r[3]  = regs->uregs[3];
	v->r.r[4]  = regs->uregs[4];
	v->r.r[5]  = regs->uregs[5];
	v->r.r[6]  = regs->uregs[6];
	v->r.r[7]  = regs->uregs[7];
	v->r.r[8]  = regs->uregs[8];
	v->r.r[9]  = regs->uregs[9];
	v->r.r[10] = regs->uregs[10];
	v->r.r[11] = regs->uregs[11];
	v->r.r[12] = regs->uregs[12];
	v->r.sp    = regs->ARM_sp;
	v->r.lr    = regs->ARM_lr;
	v->r.ip    = regs->ARM_pc;
	v->r.flags = regs->ARM_cpsr;
	v->saved_state &= ~(L4_VCPU_F_IRQ | L4_VCPU_F_USER_MODE);
	if (!(regs->ARM_cpsr & PSR_I_BIT))
		v->saved_state |= L4_VCPU_F_IRQ;
	if ((regs->ARM_cpsr & MODE_MASK) == USR_MODE)
		v->saved_state |= L4_VCPU_F_USER_MODE;
}
#endif

static inline void utcb_exc_to_ptregs(l4_exc_regs_t *exc, struct pt_regs *ptregs)
{
	/* TODO: compactify this to a memcpy */
	ptregs->uregs[0]  = exc->r[0];
	ptregs->uregs[1]  = exc->r[1];
	ptregs->uregs[2]  = exc->r[2];
	ptregs->uregs[3]  = exc->r[3];
	ptregs->uregs[4]  = exc->r[4];
	ptregs->uregs[5]  = exc->r[5];
	ptregs->uregs[6]  = exc->r[6];
	ptregs->uregs[7]  = exc->r[7];
	ptregs->uregs[8]  = exc->r[8];
	ptregs->uregs[9]  = exc->r[9];
	ptregs->uregs[10] = exc->r[10];
	ptregs->uregs[11] = exc->r[11];
	ptregs->uregs[12] = exc->r[12];
	ptregs->ARM_sp    = exc->sp;
	ptregs->ARM_lr    = exc->ulr;
	ptregs->ARM_pc    = exc->pc;
	// disable IRQ and FIQ, for valid_user_regs(regsp) + mode
	ptregs->ARM_cpsr  = exc->cpsr & ~(PSR_F_BIT | PSR_I_BIT | (MODE_MASK ^ USR_MODE));
}

static inline void ptregs_to_utcb_exc(struct pt_regs *ptregs, l4_exc_regs_t *exc)
{
	/* TODO: compactify this to a memcpy */
	exc->r[0]  = ptregs->uregs[0];
	exc->r[1]  = ptregs->uregs[1];
	exc->r[2]  = ptregs->uregs[2];
	exc->r[3]  = ptregs->uregs[3];
	exc->r[4]  = ptregs->uregs[4];
	exc->r[5]  = ptregs->uregs[5];
	exc->r[6]  = ptregs->uregs[6];
	exc->r[7]  = ptregs->uregs[7];
	exc->r[8]  = ptregs->uregs[8];
	exc->r[9]  = ptregs->uregs[9];
	exc->r[10] = ptregs->uregs[10];
	exc->r[11] = ptregs->uregs[11];
	exc->r[12] = ptregs->uregs[12];
	exc->sp    = ptregs->ARM_sp;
	exc->ulr   = ptregs->ARM_lr;
	exc->pc    = ptregs->ARM_pc;
	exc->cpsr  = ptregs->ARM_cpsr;
}

#endif /* ! __ASM_L4__L4X_ARM__EXCEPTION_H__ */
