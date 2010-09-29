INTERFACE[debug]:

#include "task.h"


INTERFACE:

#include "context.h"
#include "l4_error.h"
#include "member_offs.h"
#include "timeout.h"
#include "prio_list.h"
#include "ref_obj.h"

class Syscall_frame;
class Sender;


/** A receiver.  This is a role class, and real receiver's must inherit from 
    it.  The protected interface is intended for the receiver, and the public
    interface is intended for the sender.

    The only reason this class inherits from Context is to force a specific 
    layout for Thread.  Otherwise, Receiver could just embed or reference
    a Context.
 */
class Receiver : public Context,  public Ref_cnt_obj
{
  MEMBER_OFFSET();

public:
  enum Rcv_state
  {
    Rs_not_receiving = false,
    Rs_ipc_receive   = true,
    Rs_irq_receive   = true + 1
  };

  Rcv_state sender_ok(const Sender* sender) const;

  virtual ~Receiver() {}

private:
  // DATA
  Sender*           _partner;	// IPC partner I'm waiting for/involved with
  Syscall_frame    *_rcv_regs; // registers used for receive
  Mword _caller;
  Iteratable_prio_list         _sender_list;
};

typedef Context_ptr_base<Receiver> Receiver_ptr;

IMPLEMENTATION:

#include "l4_types.h"
#include <cassert>

#include "cpu_lock.h"
#include "globals.h"
#include "lock_guard.h"
#include "logdefs.h"
#include "sender.h"
#include "thread_lock.h"
#include "entry_frame.h"
#include "std_macros.h"
#include "thread_state.h"

// Interface for receivers

/** Constructor.
    @param thread_lock the lock used for synchronizing access to this receiver
    @param space_context the space context 
 */
PROTECTED inline
Receiver::Receiver(Thread_lock *thread_lock)
: Context(thread_lock), _caller(0)
{}

PUBLIC inline
Receiver *
Receiver::caller() const
{ return reinterpret_cast<Receiver*>(_caller & ~0x03UL); }

PUBLIC inline
unsigned char
Receiver::caller_rights() const
{ return _caller & 0x3; }


PUBLIC inline
void
Receiver::set_caller(Receiver *caller, unsigned char rights)
{
  register Mword nv = Mword(caller) | (rights & 0x3);
  reinterpret_cast<Mword volatile &>(_caller) = nv;
}
/** IPC partner (sender).
    @return sender of ongoing or previous IPC operation
 */
PROTECTED inline
Sender*
Receiver::partner() const
{
  return _partner;
}


/** Restore a saved IPC state to restart a suspended IPC.
    @param partner sender of suspended receive operation
    @param regs registers of suspended receive operation
 */
PROTECTED inline NEEDS[Receiver::set_partner, Receiver::set_rcv_regs]
void
Receiver::restore_receiver_state(Sender* partner, Syscall_frame* regs)
{
  set_partner(partner);
  set_rcv_regs(regs);
}

/** Save IPC state to allow later restart a suspended IPC.
    @param out_partner returns sender of suspended receive operation
    @param out_regs returns pointer to IPC regs of suspended receive operation
 */
PROTECTED inline
void
Receiver::save_receiver_state(Sender** out_partner, Syscall_frame** out_regs)
{
  *out_partner = _partner;
  *out_regs = _rcv_regs;
}


// Interface for senders

/** Return a reference to receiver's IPC registers.
    Senders call this function to poke values into the receiver's register set.
    @pre state() & Thread_ipc_receiving_mask
    @return pointer to receiver's IPC registers.
 */
PUBLIC inline NEEDS[<cassert>]
Syscall_frame*
Receiver::rcv_regs() const
{
  //assert (state () & Thread_ipc_receiving_mask);

  return _rcv_regs;
}

/** Head of sender list.
    @return a reference to the receiver's list of senders
 */
PUBLIC inline
Iteratable_prio_list *
Receiver::sender_list()
{
  return &_sender_list;
}

// MANIPULATORS

PROTECTED inline 
void
Receiver::set_rcv_regs(Syscall_frame* regs)
{
  _rcv_regs = regs;
}

PUBLIC inline
void
Receiver::set_timeout(Timeout *t)
{
  _timeout = t;
}

PUBLIC inline
void
Receiver::dequeue_timeout()
{
  if (_timeout)
    _timeout->dequeue(_timeout->has_hit());
}

PUBLIC inline
void
Receiver::enqueue_timeout_again()
{
  if (_timeout)
    _timeout->set_again(current_cpu());
}

PUBLIC inline
void
Receiver::reset_timeout()
{
  if (EXPECT_TRUE(!_timeout))
    return;

  _timeout->reset();
  _timeout = 0;
}

/** Initiates a receiving IPC and updates the ipc partner.
    @param sender the sender that wants to establish an IPC handshake
 */
PUBLIC inline NEEDS [Receiver::set_partner,
		     Receiver::rcv_regs,
		     "entry_frame.h", "sender.h", "l4_types.h"]
void
Receiver::ipc_init(Sender* sender)
{
  set_partner(sender);
  //rcv_regs()->from (sender->id());
  state_add_dirty(Thread_transfer_in_progress);
}

PROTECTED inline
void Receiver::prepare_receive_dirty_1(Sender *partner,
                                       Syscall_frame *regs)
{
  // cpu lock required, or weird things will happen
  assert (cpu_lock.test());

  set_rcv_regs(regs);  // message should be poked in here
  set_partner(partner);
}

PROTECTED inline
bool Receiver::prepared() const
{ return _rcv_regs; }


PROTECTED inline
void Receiver::prepare_receive_dirty_2()
{
  // cpu lock required, or weird things will happen
  assert (cpu_lock.test());
  assert (_rcv_regs);

  state_change_dirty(~(Thread_ipc_sending_mask | Thread_transfer_in_progress),
                     Thread_receiving | Thread_ipc_in_progress);
}

PUBLIC inline
bool
Receiver::in_ipc(Sender *sender)
{
  Mword ipc_state = (state() & (Thread_ipc_in_progress
                               | Thread_cancel
                               | Thread_transfer_in_progress));

  if (EXPECT_TRUE
      ((ipc_state == (Thread_transfer_in_progress | Thread_ipc_in_progress))
       &&  _partner == sender))
    return true;

  return false;

}

/** Set the IPC partner (sender).
    @param partner IPC partner
 */
PROTECTED inline 
void
Receiver::set_partner(Sender* partner)
{
  _partner = partner;
}

/** Unlock a receiver locked with ipc_try_lock(). */
PUBLIC inline NEEDS ["thread_lock.h", "globals.h"]
void
Receiver::ipc_unlock()
{
  //assert (thread_lock()->lock_owner() == current());

  thread_lock()->clear();
}



/** Return whether the receiver is ready to accept a message from the
    given sender.
    @param sender thread that wants to send a message to this receiver
    @return true if receiver is in correct state to accept a message 
                 right now (open wait, or closed wait and waiting for sender).
 */
IMPLEMENT inline NEEDS["std_macros.h", "thread_state.h", Receiver::partner,
                       Receiver::vcpu_async_ipc]
Receiver::Rcv_state
Receiver::sender_ok(const Sender *sender) const
{
  unsigned ipc_state = state() & (Thread_receiving |
                                  //                 Thread_send_in_progress |
                                  Thread_ipc_in_progress);
  // If Thread_send_in_progress is still set, we're still in the send phase
  if (EXPECT_FALSE(ipc_state != (Thread_receiving | Thread_ipc_in_progress)))
    return vcpu_async_ipc(sender);

  // Check open wait; test if this sender is really the first in queue
  if (EXPECT_TRUE(!partner()
                  && (!_sender_list.head() 
		    || sender->is_head_of(&_sender_list))))
    return Rs_ipc_receive;

  // Check closed wait; test if this sender is really who we specified
  if (EXPECT_TRUE(sender == partner()))
    return Rs_ipc_receive;

  return Rs_not_receiving;
}

/** Set up a receiving IPC.  
    @param sender 0 means any sender OK, otherwise only specified sender
                  may send
    @param regs   register set that should be used as IPC registers
    @return state bits that should be added to receiver's state word.
    @post (sender == 0 || partner() == sender) && (receive_regs() == regs)
          && (retval == Thread_receiving)
 */
PROTECTED inline NEEDS["thread_state.h", Receiver::set_partner, 
		       Receiver::set_rcv_regs]
unsigned
Receiver::setup_receiver_state(Sender* sender, Syscall_frame* regs,
                               bool = false)
{
  set_rcv_regs(regs);	// message should be poked in here
  set_partner(sender);
  return Thread_receiving;
}

//-----------------------------------------------------------------------------
// VCPU code:

PRIVATE inline
Receiver::Rcv_state
Receiver::vcpu_async_ipc(Sender const *sender) const
{
  if (EXPECT_FALSE(!vcpu_irqs_enabled()))
    return Rs_not_receiving;
    
  Receiver *self = const_cast<Receiver*>(this);
  
  if (this == current())
    self->spill_user_state();

  self->vcpu_enter_kernel_mode();

  LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
      Vcpu_log *l = tbe->payload<Vcpu_log>();
      l->type = 1;
      l->state = vcpu_state()->_saved_state;
      l->ip = Mword(sender);
      l->sp = regs()->sp();
      l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
      );

  self->_rcv_regs = &vcpu_state()->_ipc_regs;
  self->vcpu_state()->_ts.set_ipc_upcall();
  self->set_partner(const_cast<Sender*>(sender));
  self->state_add_dirty(Thread_receiving | Thread_ipc_in_progress);
  self->vcpu_save_state_and_upcall();
  return Rs_irq_receive;
}


PUBLIC inline
void
Receiver::vcpu_update_state()
{
  if (EXPECT_TRUE(!(state() & Thread_vcpu_enabled)))
    return;

  if (!sender_list()->head())
    vcpu_state()->sticky_flags &= ~Vcpu_state::Sf_irq_pending;
}
