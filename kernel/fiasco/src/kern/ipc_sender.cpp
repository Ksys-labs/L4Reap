INTERFACE:

#include "sender.h"
#include "receiver.h"

class Ipc_sender_base : public Sender
{
};

template< typename Derived >
class Ipc_sender : public Ipc_sender_base
{
private:
  Derived *derived() { return static_cast<Derived*>(this); }
  static bool dequeue_sender() { return true; }
  static bool requeue_sender() { return false; }
};

extern "C" void fast_ret_from_irq(void);

IMPLEMENTATION:

#include "config.h"
#include "entry_frame.h"
#include "globals.h"
#include "kdb_ke.h"
#include "thread_state.h"
#include <cassert>

PUBLIC
virtual void
Ipc_sender_base::ipc_receiver_aborted()
{
  assert (receiver());

  sender_dequeue(receiver()->sender_list());
  receiver()->vcpu_update_state();
  set_receiver(0);
}

/** Sender-activation function called when receiver gets ready.
    Irq::hit() actually ensures that this method is always called
    when an interrupt occurs, even when the receiver was already
    waiting.
 */
PUBLIC template< typename Derived >
virtual bool
Ipc_sender<Derived>::ipc_receiver_ready(Receiver *recv)
{
  // we are running with ints off
  assert_kdb(current()->state() & Thread_ready);
  assert_kdb(current() == recv);

  if(!recv->sender_ok(this))
    return false;

  recv->vcpu_disable_irqs();

  recv->ipc_init(this);

  derived()->transfer_msg(recv);

  recv->state_change(~(Thread_receiving
                       | Thread_transfer_in_progress
                       | Thread_ipc_in_progress),
                     Thread_ready);

  if (derived()->dequeue_sender())    // last interrupt in queue?
    {
      sender_dequeue(recv->sender_list());
      recv->vcpu_update_state();
    }

  // else remain queued if more interrupts are left
  return true;
}

PROTECTED inline NEEDS["config.h", "globals.h", "thread_state.h"]
bool
Ipc_sender_base::handle_shortcut(Syscall_frame *dst_regs,
                                 Receiver *receiver)
{
  if (EXPECT_TRUE
      ((current() != receiver
        && receiver->sched()->deblock(current_cpu(), current()->sched(), true)
        // avoid race in do_ipc() after Thread_send_in_progress
        // flag was deleted from receiver's thread state
        // also: no shortcut for alien threads, they need to see the
        // after-syscall exception
        && !(receiver->state()
          & (Thread_ready_mask | Thread_delayed_deadline | Thread_alien))
        && !current()->schedule_in_progress()))) // no schedule in progress
    {
      // we don't need to manipulate the state in a safe way
      // because we are still running with interrupts turned off
      receiver->state_add_dirty(Thread_ready);

      if (!Config::Irq_shortcut)
        {
          // no shortcut: switch to the interrupt thread which will
          // calls Irq::ipc_receiver_ready
          current()->switch_to_locked(receiver);
          return true;
        }

      // The following shortcut optimization does not work if PROFILE
      // is defined because fast_ret_from_irq does not handle the
      // different implementation of the kernel lock in profiling mode

      // At this point we are sure that the connected interrupt
      // thread is waiting for the next interrupt and that its 
      // thread priority is higher than the current one. So we
      // choose a short cut: Instead of doing the full ipc handshake
      // we simply build up the return stack frame and go out as 
      // quick as possible.
      //
      // XXX We must own the kernel lock for this optimization!
      //

      Mword *esp = reinterpret_cast<Mword*>(dst_regs);

      // set return address of irq_thread
      *--esp = reinterpret_cast<Mword>(fast_ret_from_irq);

      // XXX set stack pointer of irq_thread
      receiver->set_kernel_sp(esp);

      // directly switch to the interrupt thread context and go out
      // fast using fast_ret_from_irq (implemented in assembler).
      // kernel-unlock is done in switch_exec() (on switchee's side).

      // no shortcut if profiling: switch to the interrupt thread
      current()->switch_to_locked (receiver);
      return true;
    }
  return false;
}


PROTECTED template< typename Derived >
inline  NEEDS["config.h","globals.h", "thread_state.h",
              Ipc_sender_base::handle_shortcut]
void
Ipc_sender<Derived>::send_msg(Receiver *receiver)
{
  set_receiver(receiver);

  if (!Config::Irq_shortcut)
    {
      // in profile mode, don't optimize
      // in non-profile mode, enqueue _after_ shortcut if still necessary
      sender_enqueue(receiver->sender_list(), 255);
      receiver->vcpu_set_irq_pending();
    }

  // if the thread is waiting for this interrupt, make it ready;
  // this will cause it to run irq->receiver_ready(), which
  // handles the rest

  // XXX careful!  This code may run in midst of an do_ipc()
  // operation (or similar)!
  if (Receiver::Rcv_state s = receiver->sender_ok(this))
    {
      Syscall_frame *dst_regs = derived()->transfer_msg(receiver);

      if (derived()->requeue_sender())
	{
	  sender_enqueue(receiver->sender_list(), 255);
	  receiver->vcpu_set_irq_pending();
	}

      // ipc completed
      receiver->state_change_dirty(~Thread_ipc_mask, 0);

      // in case a timeout was set
      receiver->reset_timeout();

      if (s == Receiver::Rs_ipc_receive)
	{
	  if (handle_shortcut(dst_regs, receiver))
	    return;
	}
      // we don't need to manipulate the state in a safe way
      // because we are still running with interrupts turned off
      receiver->state_add_dirty(Thread_ready);
      receiver->sched()->deblock(receiver->cpu());
      return;
    }

  if (Config::Irq_shortcut)
    {
      // in profile mode, don't optimize
      // in non-profile mode, enqueue after shortcut if still necessary
      sender_enqueue(receiver->sender_list(), 255);
      receiver->vcpu_set_irq_pending();
    }
}

