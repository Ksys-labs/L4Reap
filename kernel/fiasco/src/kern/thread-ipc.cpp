INTERFACE:

#include "l4_error.h"

class Syscall_frame;

EXTENSION class Thread
{
protected:
  struct Log_pf_invalid
  {
    Mword pfa;
    Mword cap_idx;
    Mword err;
  };

  struct Log_exc_invalid
  {
    Mword cap_idx;
  };

  enum Check_sender_result
  {
    Ok = 0,
    Queued = 2,
    Receive_in_progress = 4,
    Failed = 1,
  };

  Syscall_frame *_snd_regs;
};

class Buf_utcb_saver
{
public:
  Buf_utcb_saver(Utcb const *u);
  void restore(Utcb *u);
private:
  L4_buf_desc buf_desc;
  Mword buf[2];
};

/**
 * Save critical contents of UTCB during nested IPC.
 */
class Pf_msg_utcb_saver : public Buf_utcb_saver
{
public:
  Pf_msg_utcb_saver(Utcb const *u);
  void restore(Utcb *u);
private:
  Mword msg[2];
};

// ------------------------------------------------------------------------
INTERFACE [debug]:

#include "tb_entry.h"

EXTENSION class Thread
{
protected:
  static unsigned log_fmt_pf_invalid(Tb_entry *, int max, char *buf) asm ("__fmt_page_fault_invalid_pager");
  static unsigned log_fmt_exc_invalid(Tb_entry *, int max, char *buf) asm ("__fmt_exception_invalid_handler");
};

// ------------------------------------------------------------------------
IMPLEMENTATION:

// IPC setup, and handling of ``short IPC'' and page-fault IPC

// IDEAS for enhancing this implementation: 

// Volkmar has suggested a possible optimization for
// short-flexpage-to-long-message-buffer transfers: Currently, we have
// to resort to long IPC in that case because the message buffer might
// contain a receive-flexpage option.  An easy optimization would be
// to cache the receive-flexpage option in the TCB for that case.
// This would save us the long-IPC setup because we wouldn't have to
// touch the receiver's user memory in that case.  Volkmar argues that
// cases like that are quite common -- for example, imagine a pager
// which at the same time is also a server for ``normal'' requests.

// The handling of cancel and timeout conditions could be improved as
// follows: Cancel and Timeout should not reset the ipc_in_progress
// flag.  Instead, they should just set and/or reset a flag of their
// own that is checked every time an (IPC) system call wants to go to
// sleep.  That would mean that IPCs that do not block are not
// cancelled or aborted.
//-

#include <cstdlib>		// panic()

#include "l4_types.h"
#include "l4_msg_item.h"
#include "l4_buf_iter.h"

#include "config.h"
#include "cpu_lock.h"
#include "ipc_timeout.h"
#include "lock_guard.h"
#include "logdefs.h"
#include "map_util.h"
#include "processor.h"
#include "kdb_ke.h"
#include "warn.h"

PUBLIC
virtual void
Thread::ipc_receiver_aborted()
{
  assert_kdb (receiver());

  sender_dequeue(receiver()->sender_list());
  receiver()->vcpu_update_state();
  set_receiver(0);

  if (!(state() & Thread_ipc_in_progress))
    return;

  state_add_dirty(Thread_ready);
  sched()->deblock(cpu());
}

/** Receiver-ready callback.  
    Receivers make sure to call this function on waiting senders when
    they get ready to receive a message from that sender. Senders need
    to overwrite this interface.

    Class Thread's implementation wakes up the sender if it is still in
    sender-wait state.
 */
PUBLIC virtual
bool
Thread::ipc_receiver_ready(Receiver *recv)
{
  if (cpu() == current_cpu())
    return ipc_local_receiver_ready(recv);
  else
    return ipc_remote_receiver_ready(recv);
}

PUBLIC virtual
void
Thread::modify_label(Mword const *todo, int cnt)
{
  assert_kdb (_snd_regs);
  Mword l = _snd_regs->from_spec();
  for (int i = 0; i < cnt*4; i += 4)
    {
      Mword const test_mask = todo[i];
      Mword const test      = todo[i+1];
      if ((l & test_mask) == test)
	{
	  Mword const del_mask = todo[i+2];
	  Mword const add_mask = todo[i+3];

	  l = (l & ~del_mask) | add_mask;
	  _snd_regs->from(l);
	  return;
	}
    }
}

PRIVATE inline
bool
Thread::ipc_local_receiver_ready(Receiver *recv)
{
  assert_kdb (receiver());
  assert_kdb (receiver() == recv);
  assert_kdb (receiver() == current());

  if (!(state() & Thread_ipc_in_progress))
    return false;

  if (!recv->sender_ok(this))
    return false;

  recv->ipc_init(this);

  state_add_dirty(Thread_ready | Thread_transfer_in_progress);

  sched()->deblock(cpu());
  sender_dequeue(recv->sender_list());
  recv->vcpu_update_state();

  // put receiver into sleep
  receiver()->state_del_dirty(Thread_ready);

  return true;
}

PRIVATE inline
void
Thread::snd_regs(Syscall_frame *r)
{ _snd_regs = r; }


/** Page fault handler.
    This handler suspends any ongoing IPC, then sets up page-fault IPC.
    Finally, the ongoing IPC's state (if any) is restored.
    @param pfa page-fault virtual address
    @param error_code page-fault error code.
 */
PRIVATE
bool
Thread::handle_page_fault_pager(Thread_ptr const &_pager,
                                Address pfa, Mword error_code,
                                L4_msg_tag::Protocol protocol)
{
#ifndef NDEBUG
  // do not handle user space page faults from kernel mode if we're
  // already handling a request
  if (EXPECT_FALSE(!PF::is_usermode_error(error_code)
		   && thread_lock()->test() == Thread_lock::Locked))
    {
      kdb_ke("Fiasco BUG: page fault, under lock");
      panic("page fault in locked operation");
    }
#endif

  if (EXPECT_FALSE((state() & Thread_alien)
                   && !(state() & Thread_ipc_in_progress)))
    return false;

  Lock_guard<Cpu_lock> guard(&cpu_lock);

  unsigned char rights;
  Kobject_iface *pager = _pager.ptr(space(), &rights);

  if (!pager)
    {
      WARN ("CPU%d: Pager of %lx is invalid (pfa=" L4_PTR_FMT
	    ", errorcode=" L4_PTR_FMT ") to %lx (pc=%lx)\n",
	    current_cpu(), dbg_id(), pfa, error_code,
            _pager.raw(), regs()->ip());


      LOG_TRACE("Page fault invalid pager", "pf", this,
                __fmt_page_fault_invalid_pager,
                Log_pf_invalid *l = tbe->payload<Log_pf_invalid>();
                l->cap_idx = _pager.raw();
                l->err     = error_code;
                l->pfa     = pfa);

      pager = this; // block on ourselves
    }

  // set up a register block used as an IPC parameter block for the
  // page fault IPC
  Syscall_frame r;
  Utcb *utcb = access_utcb();

  // save the UTCB fields affected by PF IPC
  Pf_msg_utcb_saver saved_utcb_fields(utcb);


  utcb->buf_desc = L4_buf_desc(0,0,0,0,L4_buf_desc::Inherit_fpu);
  utcb->buffers[0] = L4_msg_item::map(0).raw();
  utcb->buffers[1] = L4_fpage::all_spaces().raw();

  utcb->values[0] = PF::addr_to_msgword0 (pfa, error_code);
  utcb->values[1] = regs()->ip(); //PF::pc_to_msgword1 (regs()->ip(), error_code));

  L4_timeout_pair timeout(L4_timeout::Never, L4_timeout::Never);
  
  // This might be a page fault in midst of a long-message IPC operation.
  // Save the current IPC state and restore it later.
  Sender *orig_partner;
  Syscall_frame *orig_rcv_regs;
  save_receiver_state (&orig_partner, &orig_rcv_regs);

  Receiver *orig_snd_partner = receiver();
  Timeout *orig_timeout = _timeout;
  if (orig_timeout)
    orig_timeout->reset();

  unsigned orig_ipc_state = state() & Thread_ipc_mask;

  state_del(orig_ipc_state);
  if (orig_ipc_state)
    timeout = utcb->xfer;	// in long IPC -- use pagefault timeout

  L4_msg_tag tag(2, 0, 0, protocol);

  r.timeout(timeout);
  r.tag(tag);
  r.from(0);
  r.ref(L4_obj_ref(_pager.raw() << L4_obj_ref::Cap_shift, L4_obj_ref::Ipc_call_ipc));
  pager->invoke(r.ref(), rights, &r, utcb);


  bool success = true;

  if (EXPECT_FALSE(r.tag().has_error()))
    {
      if (Config::conservative)
        {
          printf(" page fault %s error = 0x%lx\n",
                 utcb->error.snd_phase() ? "send" : "rcv",
                 utcb->error.raw());
	  kdb_ke("ipc to pager failed");
        }

      if (utcb->error.snd_phase()
          && (utcb->error.error() == L4_error::Not_existent)
          && PF::is_usermode_error(error_code)
	  && !(state() & Thread_cancel))
	{
	  success = false;
        }
    }
  else // no error
    {
      // If the pager rejects the mapping, it replies -1 in msg.w0
      if (EXPECT_FALSE (utcb->values[0] == Mword(-1)))
        success = false;
    }

  // restore previous IPC state

  saved_utcb_fields.restore(utcb);

  set_receiver(orig_snd_partner);
  restore_receiver_state(orig_partner, orig_rcv_regs);
  state_add(orig_ipc_state);

  if (orig_timeout)
    orig_timeout->set_again(cpu());

#if 0
  if (virqs && vcpu_irqs_pending())
    {
      vcpu_enter_kernel_mode();
      vcpu_state()->_saved_state |= Vcpu_state::F_irqs;
      do_ipc(L4_msg_tag(), 0, 0, true, 0,
	     L4_timeout_pair(L4_timeout::Zero, L4_timeout::Zero),
	     &vcpu_state()->_ipc_regs, 7);
      vcpu_state()->_ts.set_ipc_upcall();
      fast_return_to_user(vcpu_state()->_entry_ip, vcpu_state()->_sp);
    }
#endif

  return success;
}


/** L4 IPC system call.
    This is the `normal'' version of the IPC system call.  It usually only
    gets called if ipc_short_cut() has failed.
    @param regs system-call arguments.
 */
IMPLEMENT inline NOEXPORT ALWAYS_INLINE
void
Thread::sys_ipc()
{
  assert_kdb (!(state() & Thread_drq_ready));

  Syscall_frame *f = this->regs();

  Obj_cap obj = f->ref();
  Utcb *utcb = access_utcb();
  // printf("sys_invoke_object(f=%p, obj=%x)\n", f, f->obj_ref());
  unsigned char rights;
  Kobject_iface *o = obj.deref(&rights);
  L4_msg_tag e;
  if (EXPECT_TRUE(o!=0))
    o->invoke(obj, rights, f, utcb);
  else
    f->tag(commit_error(utcb, L4_error::Not_existent));
}

extern "C"
void
sys_ipc_wrapper()
{
  // Don't allow interrupts before we've got a call frame with a return
  // address on our stack, so that we can tweak the return address for
  // sysenter + sys_ex_regs to the iret path.

  current_thread()->sys_ipc();

  assert_kdb (!(current()->state() &
           (Thread_delayed_deadline | Thread_delayed_ipc)));

  // If we return with a modified return address, we must not be interrupted
  //  Proc::cli();
}


extern "C"
void
ipc_short_cut_wrapper()
{
  register Thread *const ct = current_thread();
  ct->sys_ipc();

  // If we return with a modified return address, we must not be interrupted
  //  Proc::cli();
}

PRIVATE inline
Mword
Thread::check_sender(Thread *sender, bool timeout)
{
  if (EXPECT_FALSE(is_invalid()))
    {
      sender->access_utcb()->error = L4_error::Not_existent;
      return Failed;
    }

  if (EXPECT_FALSE(!sender_ok(sender)))
    {
      if (!timeout)
	{
	  sender->access_utcb()->error = L4_error::Timeout;
	  return Failed;
	}

      sender->set_receiver(this);
      sender->sender_enqueue(sender_list(), sender->sched_context()->prio());
      vcpu_set_irq_pending();
      return Queued;
    }

  return Ok;
}


PRIVATE inline
void Thread::goto_sleep(L4_timeout const &t, Sender *sender, Utcb *utcb)
{
  if (EXPECT_FALSE
     ((state() & (Thread_receiving | Thread_ipc_in_progress | Thread_cancel))
      != (Thread_receiving | Thread_ipc_in_progress)))
    return;

  IPC_timeout timeout;

  if (EXPECT_FALSE(t.is_finite() && !_timeout))
    {

      state_del_dirty(Thread_ready);

      Unsigned64 tval = t.microsecs(Timer::system_clock(), utcb);

      if (EXPECT_TRUE((tval != 0)))
	{
	  set_timeout(&timeout);
	  timeout.set(tval, cpu());
	}
      else // timeout already hit
	state_change_dirty(~Thread_ipc_in_progress, Thread_ready);

    }
  else
    {
      if (EXPECT_TRUE(t.is_never()))
	state_del_dirty(Thread_ready);
      else
	state_change_dirty(~Thread_ipc_in_progress, Thread_ready);
    }

  if (sender == this)
    switch_sched(sched());

  schedule();

  if (EXPECT_FALSE((long)_timeout))
    {
      timeout.reset();
      set_timeout(0);
    }

  assert_kdb (state() & Thread_ready);  
}




/** 
 * @pre cpu_lock must be held
 */
PRIVATE inline NEEDS["logdefs.h"]
unsigned
Thread::handshake_receiver(Thread *partner, L4_timeout snd_t)
{
  assert_kdb(cpu_lock.test());

  switch (__builtin_expect(partner->check_sender(this, !snd_t.is_zero()), Ok))
    {
    case Failed:
      return Failed;
    case Queued:
      state_add_dirty(Thread_send_in_progress | Thread_ipc_in_progress);
      return Queued;
    default:
      return Ok;
    }
}


PRIVATE inline
void
Thread::wake_receiver(Thread *receiver)
{
  // If neither IPC partner is delayed, just update the receiver's state
  if (1) // rt:EXPECT_TRUE(!((state() | receiver->state()) & Thread_delayed_ipc)))
    {
      receiver->state_change_dirty(~(Thread_ipc_receiving_mask
                                     | Thread_ipc_in_progress),
                                   Thread_ready);
      return;
    }

  // Critical section if either IPC partner is delayed until its next period
  assert_kdb (cpu_lock.test());
#if 0 // rt ext
  // Sender has no receive phase and deadline timeout already hit
  if ( (state() & (Thread_receiving |
		   Thread_delayed_deadline | Thread_delayed_ipc)) ==
      Thread_delayed_ipc)
    {
      state_change_dirty (~Thread_delayed_ipc, 0);
      switch_sched (sched_context()->next());
      _deadline_timeout.set (Timer::system_clock() + period(), cpu());
    }

  // Receiver's deadline timeout already hit
  if ( (receiver->state() & (Thread_delayed_deadline |
                             Thread_delayed_ipc) ==
                             Thread_delayed_ipc))
    {
      receiver->state_change_dirty (~Thread_delayed_ipc, 0);
      receiver->switch_sched (receiver->sched_context()->next());
      receiver->_deadline_timeout.set (Timer::system_clock() +
                                       receiver->period(), receiver->cpu());
    }
#endif
  receiver->state_change_dirty(~(Thread_ipc_mask | Thread_delayed_ipc), Thread_ready);
}

PRIVATE inline
void
Thread::set_ipc_error(L4_error const &e, Thread *rcv)
{
  access_utcb()->error = e;
  rcv->access_utcb()->error = L4_error(e, L4_error::Rcv);
}

PRIVATE inline NEEDS [Thread::do_send_wait]
bool
Thread::do_ipc_send(L4_msg_tag const &tag, Thread *partner,
                    bool have_receive,
                    L4_timeout_pair t, Syscall_frame *regs,
                    bool *dont_switch, unsigned char rights)
{
  unsigned result;

  state_add_dirty(Thread_send_in_progress);
  set_ipc_send_rights(rights);

  if (EXPECT_FALSE(partner->cpu() != current_cpu()) ||
      ((result = handshake_receiver(partner, t.snd)) == Failed
       && partner->drq_pending()))
    {
      *dont_switch = true;
      result = remote_handshake_receiver(tag, partner, have_receive, t.snd,
                                         regs, rights);
    }

  if (EXPECT_FALSE(result & Queued))
    {
      L4_timeout snd_t;
      if (result & Receive_in_progress)
	snd_t = L4_timeout::Never;
      else
	snd_t = t.snd;

      // set _snd_regs, we may become a remote IPC while waiting
      snd_regs(regs);

      if (!do_send_wait(partner, snd_t))
	return false;
    }
  else if (EXPECT_FALSE(result == Failed))
    {
      state_del_dirty(Thread_ipc_sending_mask
                      | Thread_transfer_in_progress
                      | Thread_ipc_in_progress);
      return false;
    }

  // Case 1: The handshake told us it was Ok
  // Case 2: The send_wait told us it had finished w/o error

  // in The X-CPU IPC case the IPC has been already finished here
  if (EXPECT_FALSE(partner->cpu() != current_cpu()
                   || (!(state() & Thread_send_in_progress))))
    {
      state_del_dirty(Thread_ipc_sending_mask | Thread_transfer_in_progress);
      return true;
    }

  assert_kdb (!(state() & Thread_polling));

  partner->ipc_init(this);

  // mmh, we can reset the receivers timeout
  // ping pong with timeouts will profit from it, because
  // it will require much less sorting overhead
  // if we dont reset the timeout, the possibility is very high
  // that the receiver timeout is in the timeout queue
  partner->reset_timeout();

  bool success = transfer_msg(tag, partner, regs, rights);

  if (success && this->partner() == partner)
    partner->set_caller(this, rights);

  if (!tag.do_switch() || partner->state() & Thread_suspended)
    *dont_switch = true;

  // partner locked, i.e. lazy locking (not locked) or we own the lock
  assert_kdb (!partner->thread_lock()->test()
              || partner->thread_lock()->lock_owner() == this);


  if (EXPECT_FALSE(!success || !have_receive))
    {
      // make the ipc partner ready if still engaged in ipc with us
      if (partner->in_ipc(this))
	{
	  wake_receiver(partner);
	  if (!dont_switch)
	    partner->thread_lock()->set_switch_hint(SWITCH_ACTIVATE_LOCKEE);
	}

      partner->thread_lock()->clear_dirty();

      state_del(Thread_ipc_sending_mask
                | Thread_transfer_in_progress
                | Thread_ipc_in_progress);

      return success;
    }

  partner->thread_lock()->clear_dirty_dont_switch();
  // possible preemption point

  if (EXPECT_TRUE(!partner->in_ipc(this)))
    {
      state_del(Thread_ipc_sending_mask
                | Thread_transfer_in_progress
                | Thread_ipc_in_progress);
      sender_dequeue(partner->sender_list());
      partner->vcpu_update_state();
      access_utcb()->error = L4_error::Aborted;
      return false;
    }

  wake_receiver(partner);
  prepare_receive_dirty_2();
  return true;
}

PRIVATE inline NOEXPORT
void
Thread::handle_abnormal_termination(Syscall_frame *regs)
{
  if (EXPECT_TRUE (!(state() & Thread_ipc_receiving_mask)))
    return;

  Utcb *utcb = access_utcb();
  // the IPC has not been finished.  could be timeout or cancel
  // XXX should only modify the error-code part of the status code

  if (EXPECT_FALSE((state() & Thread_busy)))
    regs->tag(commit_error(utcb, L4_error::R_aborted, regs->tag()));
  else if (EXPECT_FALSE(state() & Thread_cancel))
    {
      // we've presumably been reset!
      if (state() & Thread_transfer_in_progress)
	regs->tag(commit_error(utcb, L4_error::R_aborted, regs->tag()));
      else
	regs->tag(commit_error(utcb, L4_error::R_canceled, regs->tag()));
    }
  else
    regs->tag(commit_error(utcb, L4_error::R_timeout, regs->tag()));
}


/**
 * Send an IPC message.
 *        Block until we can send the message or the timeout hits.
 * @param partner the receiver of our message
 * @param t a timeout specifier
 * @param regs sender's IPC registers
 * @pre cpu_lock must be held
 * @return sender's IPC error code
 */
PUBLIC
void
Thread::do_ipc(L4_msg_tag const &tag, bool have_send, Thread *partner,
               bool have_receive, Sender *sender,
               L4_timeout_pair t, Syscall_frame *regs,
               unsigned char rights)
{
  assert_kdb (cpu_lock.test());
  assert_kdb (this == current());

  bool dont_switch = false;
  //LOG_MSG_3VAL(this, "ipc", (Mword) partner, (Mword) sender, cpu());
  assert_kdb (!(state() & Thread_ipc_sending_mask));

  prepare_receive_dirty_1(sender, have_receive ? regs : 0);

  if (have_send)
    {
      assert_kdb(!in_sender_list());
      bool ok = do_ipc_send(tag, partner, have_receive, t, regs, &dont_switch, rights);
      if (EXPECT_FALSE(!ok))
        {
          regs->tag(L4_msg_tag(0, 0, L4_msg_tag::Error, 0));
          assert_kdb (!in_sender_list());
          return;
        }

      if (!have_receive)
        {
          regs->tag(L4_msg_tag(0,0,0,0));
          assert_kdb (!in_sender_list());
          return;
        }
    }
  else
    {
      assert_kdb (have_receive);
      prepare_receive_dirty_2();
    }

  assert_kdb (!in_sender_list());
  assert_kdb (!(state() & Thread_ipc_sending_mask));

  while (EXPECT_TRUE
         ((state() & (Thread_receiving | Thread_ipc_in_progress | Thread_cancel))
          == (Thread_receiving | Thread_ipc_in_progress)) ) 
    {
      Sender *next = 0;

      if (EXPECT_FALSE((long)sender_list()->head()))
	{
	  if (sender) // closed wait
	    {
	      if (sender->in_sender_list()
		  && this == sender->receiver()
		  && sender->ipc_receiver_ready(this))
		next = sender;
	    }
	  else // open wait
	    {

	      next = Sender::cast(sender_list()->head());

              assert_kdb (next->in_sender_list());

	      if (!next->ipc_receiver_ready(this)) 
		{
		  next->sender_dequeue_head(sender_list());
		  vcpu_update_state();
		  Proc::preemption_point();
		  continue;
		}
	    }
	}

      assert_kdb (cpu_lock.test());

      // XXX: I'm not sure that EXPECT_FALSE ist the right here
      if (EXPECT_FALSE((long) next))
	{

	  assert_kdb (!(state() & Thread_ipc_in_progress)
		 || !(state() & Thread_ready));

	  // maybe switch_exec should return an bool to avoid testing the 
	  // state twice
	  if (have_send) 
	    {
	      assert_kdb (partner);
	      assert_kdb (partner->sched());
	    }
	  /* dont_switch == true for xCPU */
	  if (EXPECT_TRUE(have_send && !dont_switch
			  && (partner->state() & Thread_ready)
			  && (next->sender_prio() <= partner->sched()->prio())))
	    switch_exec_schedule_locked(partner,  Context::Not_Helping);
	  else
	    {
	      if (have_send && partner->cpu() == cpu()
                  && (partner->state() & Thread_ready))
	        partner->sched()->deblock(cpu());
	      schedule();
	    }

	  assert_kdb (state() & Thread_ready);
	}
      else 
	{
	  if (EXPECT_TRUE(have_send && partner->cpu() == cpu()
	                  && (partner->state() & Thread_ready)))
	    {
	      have_send = false;
	      if (!dont_switch)
		{
		  switch_exec_locked(partner,  Context::Not_Helping);
		  // We have to retry if there are possible senders in our
		  // sender queue, because a sender from a remote CPU may
		  // have been enqueued in handle_drq, in switch_exec_locked
		  continue;
		}
	      else
	        partner->sched()->deblock(cpu());
	    }

	  goto_sleep(t.rcv, sender, access_utcb());
	  have_send = false;
	  // LOG_MSG_3VAL(this, "ipcrw", Mword(sender), state(), 0);
	}
    }

  assert_kdb (!(state() & Thread_ipc_sending_mask));

  // if the receive operation was canceled/finished before we 
  // switched to the old receiver, finish the send
  if (have_send && partner->cpu() == cpu()
      && (partner->state() & Thread_ready))
    {
      if (!dont_switch && EXPECT_TRUE(partner != this))
        switch_exec_schedule_locked(partner,  Context::Not_Helping);
      else
        partner->sched()->deblock(cpu());
    }

  // fast out if ipc is already finished
  if (EXPECT_TRUE((state() & ~(Thread_transfer_in_progress | Thread_fpu_owner|Thread_cancel)) == Thread_ready))
    {
      state_del(Thread_transfer_in_progress);
      return;
    }
  assert_kdb (!(state() & (Thread_ipc_sending_mask)));

  // abnormal termination?
  handle_abnormal_termination(regs);

  state_del(Thread_ipc_mask);
}


PRIVATE inline NEEDS ["map_util.h", Thread::copy_utcb_to, 
		      Thread::unlock_receiver]
bool
Thread::transfer_msg(L4_msg_tag tag, Thread *receiver,
                     Syscall_frame *sender_regs, unsigned char rights)
{
  Syscall_frame* dst_regs = receiver->rcv_regs();

  bool success = copy_utcb_to(tag, receiver, rights);
  tag.set_error(!success);
  dst_regs->tag(tag);
  dst_regs->from(sender_regs->from_spec());
  return success;
}


/** Unlock the Receiver locked with ipc_try_lock().
    If the sender goes to wait for a registered message enable LIPC.
    @param receiver receiver to unlock
    @param sender_regs dummy
 */
PRIVATE inline NEEDS ["entry_frame.h"]
void
Thread::unlock_receiver(Receiver *receiver, const Syscall_frame*)
{
  receiver->ipc_unlock();
}


IMPLEMENT inline
Buf_utcb_saver::Buf_utcb_saver(const Utcb *u)
{
  buf_desc = u->buf_desc;
  buf[0] = u->buffers[0];
  buf[1] = u->buffers[1];
}

IMPLEMENT inline
void
Buf_utcb_saver::restore(Utcb *u)
{
  u->buf_desc = buf_desc;
  u->buffers[0] = buf[0];
  u->buffers[1] = buf[1];
}

IMPLEMENT inline
Pf_msg_utcb_saver::Pf_msg_utcb_saver(Utcb const *u) : Buf_utcb_saver(u)
{
  msg[0] = u->values[0];
  msg[1] = u->values[1];
}

IMPLEMENT inline
void
Pf_msg_utcb_saver::restore(Utcb *u)
{
  Buf_utcb_saver::restore(u);
  u->values[0] = msg[0];
  u->values[1] = msg[1];
}


/**
 * \pre must run with local IRQs disabled (CPU lock held)
 * to ensure that handler does not dissapear meanwhile.
 */
PRIVATE
bool
Thread::exception(Kobject_iface *handler, Trap_state *ts, Mword rights)
{
  Syscall_frame r;
  L4_timeout_pair timeout(L4_timeout::Never, L4_timeout::Never);

  CNT_EXC_IPC;

  void *old_utcb_handler = _utcb_handler;
  _utcb_handler = ts;

  // fill registers for IPC
  Utcb *utcb = access_utcb();
  Buf_utcb_saver saved_state(utcb);

  utcb->buf_desc = L4_buf_desc(0,0,0,0,L4_buf_desc::Inherit_fpu);
  utcb->buffers[0] = L4_msg_item::map(0).raw();
  utcb->buffers[1] = L4_fpage::all_spaces().raw();

  // clear regs
  L4_msg_tag tag(L4_exception_ipc::Msg_size, 0, L4_msg_tag::Transfer_fpu,
                 L4_msg_tag::Label_exception);

  r.tag(tag);
  r.timeout(timeout);
  r.from(0);
  r.ref(L4_obj_ref(_exc_handler.raw() << L4_obj_ref::Cap_shift, L4_obj_ref::Ipc_call_ipc));
  spill_user_state();
  handler->invoke(r.ref(), rights, &r, utcb);
  fill_user_state();

  saved_state.restore(utcb);

  if (EXPECT_FALSE(r.tag().has_error()))
    {
      if (Config::conservative)
        {
          printf(" exception fault %s error = 0x%lx\n",
                 utcb->error.snd_phase() ? "send" : "rcv",
                 utcb->error.raw());
	  kdb_ke("ipc to pager failed");
        }

      state_del(Thread_in_exception);
    }
   else if (r.tag().proto() == L4_msg_tag::Label_allow_syscall)
     state_add(Thread_dis_alien);

  // restore original utcb_handler
  _utcb_handler = old_utcb_handler;

  // FIXME: handle not existing pager properly
  // for now, just ignore any errors
  return 1;
}

/* return 1 if exception could be handled
 * return 0 if not for send_exception and halt thread
 */
PUBLIC inline NEEDS["task.h", "trap_state.h",
                    Thread::fast_return_to_user,
                    Thread::save_fpu_state_to_utcb]
int
Thread::send_exception(Trap_state *ts)
{
  assert(cpu_lock.test());

  if (vcpu_exceptions_enabled())
    {
      // no not reflect debug exceptions to the VCPU but handle them in
      // Fiasco
      if (EXPECT_FALSE(ts->is_debug_exception()
                       && !(vcpu_state()->state & Vcpu_state::F_debug_exc)))
        return 0;

      if (_exc_cont.valid())
	return 1;
      vcpu_enter_kernel_mode();
      spill_user_state();
      LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
	  Vcpu_log *l = tbe->payload<Vcpu_log>();
	  l->type = 2;
	  l->state = vcpu_state()->_saved_state;
	  l->ip = ts->ip();
	  l->sp = ts->sp();
	  l->trap = ts->trapno();
	  l->err = ts->error();
	  l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
	  );
      memcpy(&vcpu_state()->_ts, ts, sizeof(Trap_state));
      save_fpu_state_to_utcb(ts, access_utcb());
      fast_return_to_user(vcpu_state()->_entry_ip, vcpu_state()->_sp);
    }

  // local IRQs must be disabled because we dereference a Thread_ptr
  if (EXPECT_FALSE(_exc_handler.is_kernel()))
    return 0;

  if (!send_exception_arch(ts))
    return 0; // do not send exception

  unsigned char rights = 0;
  Kobject_iface *pager = _exc_handler.ptr(space(), &rights);

  if (EXPECT_FALSE(!pager))
    {
      /* no pager (anymore), just ignore the exception, return success */
      LOG_TRACE("Exception invalid handler", "exc", this,
                __fmt_exception_invalid_handler,
                Log_exc_invalid *l = tbe->payload<Log_exc_invalid>();
                l->cap_idx = _exc_handler.raw());
      if (EXPECT_FALSE(space() == sigma0_task))
	{
	  WARNX(Error, "Sigma0 raised an exception --> HALT\n");
	  panic("...");
	}

      pager = this; // block on ourselves
    }

  state_change(~Thread_cancel, Thread_in_exception);

  return exception(pager, ts, rights);
}

PRIVATE static
bool
Thread::try_transfer_local_id(L4_buf_iter::Item const *const buf,
                              L4_fpage sfp, Mword *rcv_word, Thread* snd,
                              Thread *rcv)
{
  if (buf->b.is_rcv_id())
    {
      if (snd->space() == rcv->space())
	{
	  rcv_word[-2] |= 6;
	  rcv_word[-1] = sfp.raw();
	  return true;
	}
      else
	{
	  unsigned char rights = 0;
	  Obj_space::Capability cap = snd->space()->obj_space()->lookup(sfp.obj_index());
	  Kobject_iface *o = cap.obj();
	  rights = cap.rights();
	  if (EXPECT_TRUE(o && o->is_local(rcv->space())))
	    {
	      rcv_word[-2] |= 4;
	      rcv_word[-1] = o->obj_id() | Mword(rights);
	      return true;
	    }
	}
    }
  return false;
}


PRIVATE static
bool
Thread::transfer_msg_items(L4_msg_tag const &tag, Thread* snd, Utcb *snd_utcb,
                           Thread *rcv, Utcb *rcv_utcb,
                           unsigned char rights)
{
  // LOG_MSG_3VAL(current(), "map bd=", rcv_utcb->buf_desc.raw(), 0, 0);
  L4_buf_iter mem_buffer(rcv_utcb, rcv_utcb->buf_desc.mem());
  L4_buf_iter io_buffer(rcv_utcb, rcv_utcb->buf_desc.io());
  L4_buf_iter obj_buffer(rcv_utcb, rcv_utcb->buf_desc.obj());
  L4_snd_item_iter snd_item(snd_utcb, tag.words());
  register int items = tag.items();
  Mword *rcv_word = rcv_utcb->values + tag.words();

  // XXX: damn X-CPU state modification
  // snd->prepare_long_ipc(rcv);
  Reap_list rl;

  for (;items > 0 && snd_item.more();)
    {
      if (EXPECT_FALSE(!snd_item.next()))
	{
	  snd->set_ipc_error(L4_error::Overflow, rcv);
	  return false;
	}

      L4_snd_item_iter::Item const *const item = snd_item.get();

      if (item->b.is_void())
	{ // XXX: not sure if void fpages are needed
	  // skip send item and current rcv_buffer
	  --items;
	  continue;
	}

      L4_buf_iter *buf_iter = 0;

      switch (item->b.type())
	{
	case L4_msg_item::Map:
	  switch (L4_fpage(item->d).type())
	    {
	    case L4_fpage::Memory: buf_iter = &mem_buffer; break;
	    case L4_fpage::Io:     buf_iter = &io_buffer; break;
	    case L4_fpage::Obj:    buf_iter = &obj_buffer; break;
	    default: break;
	    }
	  break;
	default:
	  break;
	}

      if (EXPECT_FALSE(!buf_iter))
	{
	  // LOG_MSG_3VAL(snd, "lIPCm0", 0, 0, 0);
	  snd->set_ipc_error(L4_error::Overflow, rcv);
	  return false;
	}

      L4_buf_iter::Item const *const buf = buf_iter->get();

      if (EXPECT_FALSE(buf->b.is_void() || buf->b.type() != item->b.type()))
	{
	  // LOG_MSG_3VAL(snd, "lIPCm1", buf->b.raw(), item->b.raw(), 0);
	  snd->set_ipc_error(L4_error::Overflow, rcv);
	  return false;
	}

	{
	  assert_kdb (item->b.type() == L4_msg_item::Map);
	  L4_fpage sfp(item->d);
	  *rcv_word = (item->b.raw() & ~0x0ff7) | (sfp.raw() & 0x0ff0);

	  rcv_word += 2;

	  if (!try_transfer_local_id(buf, sfp, rcv_word, snd, rcv))
	    {
	      // we need to do a real mapping¿

	      // diminish when sending via restricted ipc gates
	      if (sfp.type() == L4_fpage::Obj)
		sfp.mask_rights(L4_fpage::Rights(rights | L4_fpage::RX));

	      L4_error err = fpage_map(snd->space(), sfp,
		  rcv->space(), L4_fpage(buf->d), item->b.raw(), &rl);

	      if (EXPECT_FALSE(!err.ok()))
		{
		  snd->set_ipc_error(err, rcv);
		  return false;
		}
	    }
	}

      --items;

      if (!item->b.compund())
	buf_iter->next();
    }

  if (EXPECT_FALSE(items))
    {
      snd->set_ipc_error(L4_error::Overflow, rcv);
      return false;
    }

  return true;
}


/**
 * \pre Runs on the sender CPU
 */
PRIVATE inline NEEDS[Thread::do_remote_abort_send]
bool
Thread::abort_send(L4_error const &e, Thread *partner)
{
  state_del_dirty(Thread_send_in_progress | Thread_polling | Thread_ipc_in_progress
                  | Thread_transfer_in_progress);

  if (_timeout && _timeout->is_set())
    _timeout->reset();

  set_timeout(0);

  if (partner->cpu() == current_cpu())
    {
      if (in_sender_list())
	{
	  sender_dequeue(partner->sender_list());
	  partner->vcpu_update_state();
	}

      access_utcb()->error = e;
      return true;
    }

  return do_remote_abort_send(e, partner);
}



/**
 * \pre Runs on the sender CPU
 */
PRIVATE inline
bool
Thread::do_send_wait(Thread *partner, L4_timeout snd_t)
{
  state_add_dirty(Thread_polling);

  IPC_timeout timeout;

  if (EXPECT_FALSE(snd_t.is_finite()))
    {
      Unsigned64 tval = snd_t.microsecs(Timer::system_clock(), access_utcb());
      // Zero timeout or timeout expired already -- give up
      if (tval == 0)
	return abort_send(L4_error::Timeout, partner);

      set_timeout(&timeout);
      timeout.set(tval, cpu());
    }

  while (1)
    {
      if ((state() & (Thread_ipc_in_progress | Thread_polling
                      | Thread_cancel | Thread_transfer_in_progress))
           == (Thread_ipc_in_progress | Thread_polling))
        {
          state_del_dirty(Thread_ready);
          schedule();
        }

      // ipc handshake bit is set
      if ((state() & (Thread_transfer_in_progress | Thread_receiving
	              | Thread_ipc_in_progress))
	  != Thread_ipc_in_progress)
	break;

      if (EXPECT_FALSE(state() & Thread_cancel))
	return abort_send(L4_error::Canceled, partner);

      // FIXME: existence check
#if 0
      if (EXPECT_FALSE(0 && partner->is_invalid()))
	{
	  state_del_dirty(Thread_send_in_progress | Thread_polling
	      | Thread_ipc_in_progress | Thread_transfer_in_progress);

	  if (_timeout && _timeout->is_set())
	    _timeout->reset();

	  set_timeout(0);

          access_utcb()->error = L4_error::Not_existent;
	  return false;
	}
#endif

      // Make sure we're really still in IPC
      assert_kdb (state() & Thread_ipc_in_progress);

      state_add_dirty(Thread_polling);
    }

  state_del_dirty(Thread_polling);

  if (EXPECT_FALSE((state() & (Thread_send_in_progress | Thread_cancel))
        == (Thread_send_in_progress | Thread_cancel)))
    return abort_send(L4_error::Canceled, partner);

  // reset is only an simple dequeing operation from an double
  // linked list, so we dont need an extra preemption point for this

  if (EXPECT_FALSE(timeout.has_hit() && (state() & (Thread_send_in_progress
                               | Thread_ipc_in_progress)) ==
      Thread_send_in_progress))
    return abort_send(L4_error::Timeout, partner);

  timeout.reset();
  set_timeout(0);

  return true;
}


//---------------------------------------------------------------------
IMPLEMENTATION [!mp]:

PRIVATE inline
void
Thread::set_ipc_send_rights(unsigned char)
{}

PRIVATE inline NEEDS ["l4_types.h"]
unsigned
Thread::remote_handshake_receiver(L4_msg_tag const &, Thread *,
                                  bool, L4_timeout, Syscall_frame *, unsigned char)
{
  kdb_ke("Remote IPC in UP kernel");
  return Failed;
}

PRIVATE inline
bool
Thread::ipc_remote_receiver_ready(Receiver *)
{ kdb_ke("Remote IPC in UP kernel"); return false; }


PRIVATE inline
bool
Thread::do_remote_abort_send(L4_error const &, Thread *)
{ kdb_ke("Remote abort send on UP kernel"); return false; }

//---------------------------------------------------------------------
INTERFACE [mp]:

EXTENSION class Thread
{
private:
  unsigned char _ipc_send_rights;
};

struct Ipc_remote_request;

struct Ipc_remote_request
{
  L4_msg_tag tag;
  Thread *partner;
  Syscall_frame *regs;
  unsigned char rights;
  bool timeout;
  bool have_rcv;

  unsigned result;
};

struct Ready_queue_request;

struct Ready_queue_request
{
  Thread *thread;
  Mword state_add;
  Mword state_del;

  enum Result { Done, Wrong_cpu, Not_existent };
  Result result;
};

//---------------------------------------------------------------------
IMPLEMENTATION [mp]:


PRIVATE inline
void
Thread::set_ipc_send_rights(unsigned char c)
{
  _ipc_send_rights = c;
}

PRIVATE inline
void
Thread::schedule_if(bool s)
{
  if (!s || current()->schedule_in_progress())
    return;

  current()->schedule();
}

PRIVATE inline NEEDS[Thread::schedule_if]
bool
Thread::do_remote_abort_send(L4_error const &e, Thread *partner)
{
  Ipc_remote_request rq;
  rq.partner = partner;
  partner->drq(handle_remote_abort_send, &rq);
  if (rq.tag.has_error())
    access_utcb()->error = e;
  schedule_if(handle_drq());
  return !rq.tag.has_error();
}

/**
 *
 * Runs on the receiver CPU in the context of recv.
 * The 'this' pointer is the sender.
 */
PRIVATE inline NEEDS[Thread::schedule_if]
bool
Thread::ipc_remote_receiver_ready(Receiver *recv)
{
  //printf(" remote ready: %x.%x \n", id().task(), id().lthread());
  //LOG_MSG_3VAL(this, "recvr", Mword(recv), 0, 0);
  assert_kdb (recv->cpu() == current_cpu());

  recv->ipc_init(this);

  Syscall_frame *regs = _snd_regs;

  recv->vcpu_disable_irqs();
  //printf("  transfer to %p\n", recv);
  bool success = transfer_msg(regs->tag(), nonull_static_cast<Thread*>(recv), regs, _ipc_send_rights);
  //printf("  done\n");
  regs->tag(L4_msg_tag(regs->tag(), success ? 0 : L4_msg_tag::Error));
  if (success && partner() == nonull_static_cast<Thread*>(recv))
    nonull_static_cast<Thread*>(recv)->set_caller(this, _ipc_send_rights);


  recv->state_del_dirty(Thread_ipc_receiving_mask | Thread_ipc_in_progress);

  // dequeue sender from receiver's sending queue
  sender_dequeue(recv->sender_list());
  recv->vcpu_update_state();

  Ready_queue_request rq;
  rq.thread = this;
  rq.state_add = Thread_transfer_in_progress;
  if (Receiver::prepared())
    { // same as in Receiver::prepare_receive_dirty_2
      rq.state_del = Thread_ipc_sending_mask;
      rq.state_add |= Thread_receiving;
    }
  else
    rq.state_del = 0;

  drq(handle_remote_ready_enqueue, &rq);
  schedule_if(current()->handle_drq());
  //printf("  wakeup sender done\n");

  return true;
}

PRIVATE inline NOEXPORT
bool
Thread::remote_ipc_send(Context *src, Ipc_remote_request *rq)
{
  (void)src;
  //LOG_MSG_3VAL(this, "rse", current_cpu(), (Mword)src, 0);
#if 0
  LOG_MSG_3VAL(this, "rsend", (Mword)src, 0, 0);
  printf("CPU[%u]: remote IPC send ...\n"
         "  partner=%p [%u]\n"
	 "  sender =%p [%u] regs=%p\n"
	 "  timeout=%u\n",
	 current_cpu(),
	 rq->partner, rq->partner->cpu(),
	 src, src->cpu(),
	 rq->regs,
	 rq->timeout);
#endif
  rq->result = Ok;

  switch (__builtin_expect(rq->partner->check_sender(this, rq->timeout), Ok))
    {
    case Failed:
      rq->result = Failed;
      return false;
    case Queued:
      rq->result = Queued;
      return false;
    default:
      break;
    }

  // trigger remote_ipc_receiver_ready path, because we may need to grab locks
  // and this is forbidden in a DRQ handler. So transfer the IPC in usual
  // thread code. However, this induces a overhead of two extra IPIs.
  if (rq->tag.items())
    {
      set_receiver(rq->partner);
      sender_enqueue(rq->partner->sender_list(), sched_context()->prio());
      rq->partner->vcpu_set_irq_pending();

      //LOG_MSG_3VAL(rq->partner, "pull", dbg_id(), 0, 0);
      rq->result = Queued | Receive_in_progress;
      rq->partner->state_add_dirty(Thread_ready);
      rq->partner->sched()->deblock(current_cpu());
      return true;
    }
  rq->partner->vcpu_disable_irqs();
  bool success = transfer_msg(rq->tag, rq->partner, rq->regs, _ipc_send_rights);
  rq->result = success ? Ok : Failed;

  if (success && partner() == rq->partner)
    rq->partner->set_caller(this, _ipc_send_rights);

  rq->partner->state_change_dirty(~(Thread_ipc_receiving_mask | Thread_ipc_in_progress), Thread_ready);
  // hm, should be done by lazy queueing: rq->partner->ready_enqueue();
  return true;
}

PRIVATE static
unsigned
Thread::handle_remote_ipc_send(Drq *src, Context *, void *_rq)
{
  Ipc_remote_request *rq = (Ipc_remote_request*)_rq;
  bool r = nonull_static_cast<Thread*>(src->context())->remote_ipc_send(src->context(), rq);
  //LOG_MSG_3VAL(src, "rse<", current_cpu(), (Mword)src, r);
  return r ? Drq::Need_resched : 0;
}

PRIVATE static
unsigned
Thread::handle_remote_abort_send(Drq *src, Context *, void *_rq)
{
  Ipc_remote_request *rq = (Ipc_remote_request*)_rq;
  Thread *sender = nonull_static_cast<Thread*>(src->context());
  if (sender->in_sender_list())
    {
      // really cancled IPC
      rq->tag.set_error(true);
      sender->sender_dequeue(rq->partner->sender_list());
      rq->partner->vcpu_update_state();
      return 0;
    }
  else
    {
      // IPC already done
      return 0;
    }
}


PRIVATE static
unsigned
Thread::handle_remote_ready_enqueue(Drq *, Context *self, void *_rq)
{
  Ready_queue_request *rq = (Ready_queue_request*)_rq;
  Context *c = self;
  //LOG_MSG_3VAL(current(), "rre", rq->state_add, rq->state_del, c->state());

  c->state_add_dirty(rq->state_add);
  c->state_del_dirty(rq->state_del);
  rq->result = Ready_queue_request::Done;

  if (EXPECT_FALSE(c->state() & Thread_ready))
    return Drq::Need_resched;

  c->state_add_dirty(Thread_ready);
  // hm, should be done by our lazy queueing: c->ready_enqueue();
  return Drq::Need_resched;
}




/**
 * \pre Runs on the sender CPU
 */
PRIVATE //inline NEEDS ["mp_request.h"]
unsigned
Thread::remote_handshake_receiver(L4_msg_tag const &tag, Thread *partner,
                                  bool have_receive,
                                  L4_timeout snd_t, Syscall_frame *regs,
                                  unsigned char rights)
{
  // Flag that there must be no switch in the receive path.
  // This flag also prevents the receive path from accessing
  // the thread state of a remote sender.
  Ipc_remote_request rq;
  rq.tag = tag;
  rq.have_rcv = have_receive;
  rq.partner = partner;
  rq.timeout = !snd_t.is_zero();
  rq.regs = regs;
  rq.rights = rights;
  _snd_regs = regs;

  set_receiver(partner);

  state_add_dirty(Thread_send_in_progress | Thread_ipc_in_progress);

  partner->drq(handle_remote_ipc_send, &rq,
               remote_prepare_receive);


  return rq.result;
}

PRIVATE static
unsigned
Thread::remote_prepare_receive(Drq *src, Context *, void *arg)
{
  Context *c = src->context();
  Ipc_remote_request *rq = (Ipc_remote_request*)arg;
  //printf("CPU[%2u:%p]: remote_prepare_receive (err=%x)\n", current_cpu(), c, rq->err.error());

  if (EXPECT_FALSE(rq->result & Queued))
    return 0;

  c->state_del(Thread_send_in_progress);
  if (EXPECT_FALSE((rq->result & Failed) || !rq->have_rcv))
    return 0;

  Thread *t = nonull_static_cast<Thread*>(c);
  t->prepare_receive_dirty_2();
  return 0;
}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:

IMPLEMENT
unsigned
Thread::log_fmt_pf_invalid(Tb_entry *e, int max, char *buf)
{
  Log_pf_invalid *l = e->payload<Log_pf_invalid>();
  return snprintf(buf, max, "InvCap C:%lx pfa=%lx err=%lx", l->cap_idx, l->pfa, l->err);
}

IMPLEMENT
unsigned
Thread::log_fmt_exc_invalid(Tb_entry *e, int max, char *buf)
{
  Log_exc_invalid *l = e->payload<Log_exc_invalid>();
  return snprintf(buf, max, "InvCap C:%lx", l->cap_idx);
}
