INTERFACE:

extern "C" void vcpu_resume(Trap_state *, Return_frame *sp)
   FIASCO_FASTCALL FIASCO_NORETURN;


// --------------------------------------------------------------------------
IMPLEMENTATION:

#include "logdefs.h"
#include "vcpu.h"

PUBLIC inline NEEDS["logdefs.h", "vcpu.h"]
bool
Thread::vcpu_pagefault(Address pfa, Mword err, Mword ip)
{
  (void)ip;
  if (vcpu_pagefaults_enabled())
    {
      spill_user_state();
      vcpu_enter_kernel_mode();
      LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
	  Vcpu_log *l = tbe->payload<Vcpu_log>();
	  l->type = 3;
	  l->state = vcpu_state()->_saved_state;
	  l->ip = ip;
	  l->sp = pfa;
	  l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
	  );
      vcpu_state()->_ts.set_pagefault(pfa, err);
      vcpu_save_state_and_upcall();
      return true;
    }

  return false;
}


PRIVATE inline NEEDS[Thread::fast_return_to_user]
L4_msg_tag
Thread::sys_vcpu_resume(L4_msg_tag const &tag, Utcb *utcb)
{
  if (this != current() || !(state() & Thread_vcpu_enabled))
    return commit_result(-L4_err::EInval);

  Obj_space *s = space()->obj_space();

  L4_obj_ref user_task = vcpu_state()->user_task;
  if (user_task.valid())
    {
      unsigned char task_rights = 0;
      Task *task = Kobject::dcast<Task*>(s->lookup_local(user_task.cap(),
                                                         &task_rights));

      if (EXPECT_FALSE(task && !(task_rights & L4_fpage::W)))
        return commit_result(-L4_err::EPerm);

      if (task != vcpu_user_space())
        vcpu_set_user_space(task);

      vcpu_state()->user_task = L4_obj_ref();
    }
  else if (user_task.flags() == L4_obj_ref::Ipc_reply)
    vcpu_set_user_space(0);

  L4_snd_item_iter snd_items(utcb, tag.words());
  int items = tag.items();
  for (; items && snd_items.more(); --items)
    {
      if (EXPECT_FALSE(!snd_items.next()))
        break;

      cpu_lock.clear();

      L4_snd_item_iter::Item const *const item = snd_items.get();
      L4_fpage sfp(item->d);

      Reap_list rl;
      L4_error err = fpage_map(space(), sfp,
                               vcpu_user_space(), L4_fpage::all_spaces(),
                               item->b.raw(), &rl);
      rl.del();

      cpu_lock.lock();

      if (EXPECT_FALSE(!err.ok()))
        return commit_error(utcb, err);
    }

  if ((vcpu_state()->_saved_state & Vcpu_state::F_irqs) && vcpu_irqs_pending())
    {
      assert_kdb(cpu_lock.test());
      do_ipc(L4_msg_tag(), 0, 0, true, 0,
	     L4_timeout_pair(L4_timeout::Zero, L4_timeout::Zero),
	     &vcpu_state()->_ipc_regs, 7);
      if (EXPECT_TRUE(!vcpu_state()->_ipc_regs.tag().has_error()))
	{
	  vcpu_state()->_ts.set_ipc_upcall();

	  Address sp;

	  if (vcpu_state()->_saved_state & Vcpu_state::F_user_mode)
	    sp = vcpu_state()->_entry_sp;
	  else
	    sp = vcpu_state()->_ts.sp();

	  LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
	      Vcpu_log *l = tbe->payload<Vcpu_log>();
	      l->type = 4;
	      l->state = vcpu_state()->state;
	      l->ip = vcpu_state()->_entry_ip;
	      l->sp = sp;
	      l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
	      );

	  fast_return_to_user(vcpu_state()->_entry_ip, sp, false);
	}
    }

  vcpu_state()->state = vcpu_state()->_saved_state;
  Trap_state ts;
  memcpy(&ts, &vcpu_state()->_ts, sizeof(Trap_state));


  assert_kdb(cpu_lock.test());

  ts.set_ipc_upcall();

  ts.sanitize_user_state();

  if (vcpu_state()->state & Vcpu_state::F_user_mode)
    {
      if (!vcpu_user_space())
	return commit_result(-L4_err::EInval);

      vcpu_state()->state |= Vcpu_state::F_traps | Vcpu_state::F_exceptions
                             | Vcpu_state::F_debug_exc;
      state_add_dirty(Thread_vcpu_user_mode | Thread_alien);

      if (!(vcpu_state()->state & Vcpu_state::F_fpu_enabled))
	{
	  state_add_dirty(Thread_vcpu_fpu_disabled);
	  Fpu::disable();
	}
      else
        state_del_dirty(Thread_vcpu_fpu_disabled);

      vcpu_resume_user_arch();

      vcpu_user_space()->switchin_context(space());
    }

  LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
      Vcpu_log *l = tbe->payload<Vcpu_log>();
      l->type = 0;
      l->state = vcpu_state()->state;
      l->ip = ts.ip();
      l->sp = ts.sp();
      l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
      );

  vcpu_resume(&ts, regs());
}
