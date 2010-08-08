INTERFACE:

#include "vcpu.h"

EXTENSION class Context
{
protected:
  Space *_vcpu_user_space;

private:
  Vcpu_state *_vcpu_state;
};


IMPLEMENTATION:

PROTECTED inline
Vcpu_state *
Context::vcpu_state() const
{ return _vcpu_state; }

PROTECTED inline
void
Context::vcpu_state(void *s)
{ _vcpu_state = (Vcpu_state*)s; }


PUBLIC inline
Mword
Context::vcpu_disable_irqs()
{
  if (EXPECT_FALSE(state() & Thread_vcpu_enabled))
    {
      Mword s = vcpu_state()->state;
      vcpu_state()->state = s & ~Vcpu_state::F_irqs;
      return s & Vcpu_state::F_irqs;
    }
  return 0;
}

PUBLIC inline
void
Context::vcpu_restore_irqs(bool irqs)
{
  if (EXPECT_FALSE(state() & Thread_vcpu_enabled) && irqs)
    vcpu_state()->state |= Vcpu_state::F_irqs;
}

PUBLIC inline
void
Context::vcpu_save_state_and_upcall()
{
  extern char leave_by_vcpu_upcall[];
  _exc_cont.activate(regs(), leave_by_vcpu_upcall);
}

PUBLIC inline NEEDS["fpu.h"]
void
Context::vcpu_enter_kernel_mode()
{
  if (EXPECT_FALSE(state() & Thread_vcpu_enabled))
    {
      vcpu_state()->_saved_state = vcpu_state()->state;
      Mword flags = Vcpu_state::F_traps
	            | Vcpu_state::F_user_mode;
      vcpu_state()->state &= ~flags;
      if (state() & Thread_vcpu_user_mode)
	{
	  vcpu_state()->_sp = vcpu_state()->_entry_sp;
	  state_del_dirty(  Thread_vcpu_user_mode
                          | Thread_vcpu_fpu_disabled
                          | Thread_alien);

	  if (current() == this)
	    {
	      if (state() & Thread_fpu_owner)
		Fpu::enable();

	      space()->switchin_context(vcpu_user_space());
	    }
	}
      else
	vcpu_state()->_sp = regs()->sp();
    }
}



PUBLIC inline
bool
Context::vcpu_irqs_enabled() const
{
  return EXPECT_FALSE(state() & Thread_vcpu_enabled)
    && vcpu_state()->state & Vcpu_state::F_irqs;
}

PUBLIC inline
bool
Context::vcpu_pagefaults_enabled() const
{
  return EXPECT_FALSE(state() & Thread_vcpu_enabled)
    && vcpu_state()->state & Vcpu_state::F_page_faults;
}

PUBLIC inline
bool
Context::vcpu_exceptions_enabled() const
{
  return EXPECT_FALSE(state() & Thread_vcpu_enabled)
    && vcpu_state()->state & Vcpu_state::F_exceptions;
}

PUBLIC inline
void
Context::vcpu_set_irq_pending()
{
  if (EXPECT_FALSE(state() & Thread_vcpu_enabled))
    vcpu_state()->sticky_flags |= Vcpu_state::Sf_irq_pending;
}

PUBLIC inline
bool
Context::vcpu_irqs_pending() const
{
  if (EXPECT_FALSE(state() & Thread_vcpu_enabled))
    return vcpu_state()->sticky_flags & Vcpu_state::Sf_irq_pending;
  return false;
}

/** Return the space context.
    @return space context used for this execution context.
            Set with set_space_context().
 */
PUBLIC inline NEEDS["kdb_ke.h", "cpu_lock.h"]
Space *
Context::vcpu_user_space() const
{
  //assert_kdb (cpu_lock.test());
  return _vcpu_user_space;
}

PUBLIC inline NEEDS["space.h"]
void
Context::vcpu_set_user_space(Space *t)
{
  assert_kdb (current() == this);
  if (t)
    t->inc_ref();
  else
    state_del_dirty(Thread_vcpu_user_mode);

  Space *old = _vcpu_user_space;
  _vcpu_user_space = t;

  if (old)
    {
      if (!old->dec_ref())
	{
	  rcu_wait();
	  delete old;
	}
    }
}


// --------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Context
{
  static unsigned vcpu_log_fmt(Tb_entry *, int, char *)
  asm ("__context_vcpu_log_fmt");
};


// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "kobject_dbg.h"

IMPLEMENT
unsigned
Context::vcpu_log_fmt(Tb_entry *e, int maxlen, char *buf)
{
  Vcpu_log *l = e->payload<Vcpu_log>();

  switch (l->type)
    {
    case 0:
    case 4:
      return snprintf(buf, maxlen, "%sret pc=%lx sp=%lx state=%lx task=D:%lx",
	  l->type == 4 ? "f" : "", l->ip, l->sp, l->state, l->space);
    case 1:
      return snprintf(buf, maxlen, "ipc from D:%lx task=D:%lx sp=%lx",
	  Kobject_dbg::pointer_to_id((Kobject*)l->ip), l->state, l->sp);
    case 2:
      return snprintf(buf, maxlen, "exc #%x err=%lx pc=%lx sp=%lx state=%lx task=D:%lx",
	  (unsigned)l->trap, l->err, l->ip, l->sp, l->state, l->space);
    case 3:
      return snprintf(buf, maxlen, "pf  pc=%lx pfa=%lx state=%lx task=D:%lx",
	  l->ip, l->sp, l->state, l->space);
    default:
      return snprintf(buf, maxlen, "unknown");
    }
}

