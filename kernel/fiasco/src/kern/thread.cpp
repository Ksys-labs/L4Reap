INTERFACE:

#include <csetjmp>             // typedef jmp_buf
#include "kobject.h"
#include "l4_types.h"
#include "config.h"
#include "continuation.h"
#include "helping_lock.h"
#include "mem_layout.h"
#include "member_offs.h"
#include "receiver.h"
#include "ref_obj.h"
#include "sender.h"
#include "space.h"		// Space_index
#include "spin_lock.h"
#include "thread_lock.h"

class Return_frame;
class Syscall_frame;
class Vcpu_state;

typedef Context_ptr_base<Thread> Thread_ptr;


/** A thread.  This class is the driver class for most kernel functionality.
 */
class Thread :
  public Receiver,
  public Sender,
  public Kobject_iface,
  public Kobject
{
  FIASCO_DECLARE_KOBJ();
  MEMBER_OFFSET();

  friend class Jdb;
  friend class Jdb_bt;
  friend class Jdb_tcb;
  friend class Jdb_thread;
  friend class Jdb_thread_list;
  friend class Jdb_list_threads;
  friend class Jdb_list_timeouts;
  friend class Jdb_tbuf_show;

public:
  enum Context_mode_kernel { Kernel = 0 };
  enum Operation
  {
    Opcode_mask = 0xffff,
    Op_control = 0,
    Op_ex_regs = 1,
    Op_switch  = 2,
    Op_stats   = 3,
    Op_vcpu_resume = 4,
    Op_register_del_irq = 5,
    Op_modify_senders = 6,
    Op_gdt_x86 = 0x10,
  };

  enum Control_flags
  {
    Ctl_set_pager       = 0x0010000,
    Ctl_set_scheduler   = 0x0020000,
    Ctl_set_mcp         = 0x0040000,
    Ctl_set_prio        = 0x0080000,
    Ctl_set_quantum     = 0x0100000,
    Ctl_bind_task       = 0x0200000,
    Ctl_alien_thread    = 0x0400000,
    Ctl_ux_native       = 0x0800000,
    Ctl_set_exc_handler = 0x1000000,
    Ctl_vcpu_enabled    = 0x2000000,
  };

  enum Ex_regs_flags
  {
    Exr_cancel            = 0x10000,
    Exr_trigger_exception = 0x20000,
  };

  struct Remote_syscall
  {
    Thread *thread;
    L4_msg_tag result;
  };


  class Dbg_stack
  {
  public:
    enum { Stack_size = Config::PAGE_SIZE };
    void *stack_top;
    Dbg_stack();
  };

  static Per_cpu<Dbg_stack> dbg_stack;

public:

  typedef void (Utcb_copy_func)(Thread *sender, Thread *receiver);

  /**
   * Constructor.
   *
   * @param task the task the thread should reside in.
   * @param id user-visible thread ID of the sender.
   * @param init_prio initial priority.
   * @param mcp maximum controlled priority.
   *
   * @post state() != Thread_invalid.
   */
  Thread();


  int handle_page_fault (Address pfa, Mword error, Mword pc,
      Return_frame *regs);

  void sys_ipc();

private:

  struct Migration_helper_info
  {
    Migration_info inf;
    Thread *victim;
  };

  Thread(const Thread&);	///< Default copy constructor is undefined
  void *operator new(size_t);	///< Default new operator undefined

  bool handle_sigma0_page_fault (Address pfa);

  /**
   * Return to user.
   *
   * This function is the default routine run if a newly
   * initialized context is being switch_exec()'ed.
   */
  static void user_invoke();

public:
  static bool pagein_tcb_request(Return_frame *regs);

  inline Mword user_ip() const;
  inline void user_ip(Mword);

  inline Mword user_sp() const;
  inline void user_sp(Mword);

  inline Mword user_flags() const;

  /** nesting level in debugger (always critical) if >1 */
  static Per_cpu<unsigned long> nested_trap_recover;
  static void handle_remote_requests_irq() asm ("handle_remote_cpu_requests");
  static void handle_global_remote_requests_irq() asm ("ipi_remote_call");
protected:
  // implementation details follow...

  explicit Thread(Context_mode_kernel);

  // DATA

  // Another critical TCB cache line:
  Thread_lock  _thread_lock;

  // More ipc state
  Thread_ptr _pager;
  Thread_ptr _exc_handler;

public:
  jmp_buf *_recover_jmpbuf;	// setjmp buffer for page-fault recovery

protected:
  Ram_quota *_quota;
  Irq_base *_del_observer;

  // debugging stuff
  unsigned _magic;
  static const unsigned magic = 0xf001c001;
};

class Obj_cap : public L4_obj_ref
{
};


IMPLEMENTATION:

#include <cassert>
#include <cstdlib>		// panic()
#include <cstring>
#include "atomic.h"
#include "entry_frame.h"
#include "fpu_alloc.h"
#include "globals.h"
#include "kdb_ke.h"
#include "kmem_alloc.h"
#include "logdefs.h"
#include "map_util.h"
#include "ram_quota.h"
#include "sched_context.h"
#include "space.h"
#include "std_macros.h"
#include "task.h"
#include "thread_state.h"
#include "timeout.h"
#include "timer.h"

FIASCO_DEFINE_KOBJ(Thread);

Per_cpu<unsigned long> DEFINE_PER_CPU Thread::nested_trap_recover;


IMPLEMENT
Thread::Dbg_stack::Dbg_stack()
{
  stack_top = Kmem_alloc::allocator()->unaligned_alloc(Stack_size); 
  if (stack_top)
    stack_top = (char *)stack_top + Stack_size;
  //printf("JDB STACK start= %p - %p\n", (char *)stack_top - Stack_size, (char *)stack_top);
}


PUBLIC inline NEEDS[Thread::thread_lock]
void
Thread::kill_lock()
{
  thread_lock()->lock();
}


PUBLIC inline
void *
Thread::operator new(size_t, Ram_quota *q) throw ()
{
  void *t = Mapped_allocator::allocator()->q_unaligned_alloc(q, Config::thread_block_size);
  if (t)
    {
      memset(t, 0, sizeof(Thread));
      reinterpret_cast<Thread*>(t)->_quota = q;
    }
  return t;
}

/** Class-specific allocator.
    This allocator ensures that threads are allocated at a fixed virtual
    address computed from their thread ID.
    @param id thread ID
    @return address of new thread control block
 */
PRIVATE inline
void *
Thread::operator new(size_t, Thread *t) throw ()
{
  // Allocate TCB in TCB space.  Actually, do not allocate anything,
  // just return the address.  Allocation happens on the fly in
  // Thread::handle_page_fault().
  return t;
}

/** Deallocator.  This function currently does nothing: We do not free up
    space allocated to thread-control blocks.
 */
PUBLIC
void
Thread::operator delete(void *_t)
{
  Thread * const t = static_cast<Thread*>(_t);
  Ram_quota * const q = t->_quota;
  Mapped_allocator::allocator()->q_unaligned_free(q, Config::thread_block_size, t);

  LOG_TRACE("Kobject delete", "del", current(), __fmt_kobj_destroy,
      Log_destroy *l = tbe->payload<Log_destroy>();
      l->id = t->dbg_id();
      l->obj = t;
      l->type = "Thread";
      l->ram = q->current());
}


PUBLIC inline NEEDS["space.h"]
bool
Thread::bind(Space *t, void *_utcb)
{
  // _utcb == 0 for all kernel threads
  assert_kdb (!_utcb || t->is_utcb_valid(_utcb));

    {
      Lock_guard<Spin_lock> guard(&_space);
      if (_space.get_unused())
	return false;

      _space.set_unused(t);
      space()->inc_ref();
    }

  utcb(t->kernel_utcb(_utcb));
  local_id(Address(_utcb));
  _utcb_handler = 0;

  return true;
}


PUBLIC inline NEEDS["kdb_ke.h", "cpu_lock.h", "space.h"]
bool
Thread::unbind()
{
  Space *old;

    {
      Lock_guard<Spin_lock> guard(&_space);

      if (!_space.get_unused())
	return true;

      old = _space.get_unused();
      _space.set_unused(0);

      Mem_space *oms = old->mem_space();

      if (old->dec_ref())
	old = 0;

      // switch to a safe page table
      if (Mem_space::current_mem_space(current_cpu()) == oms)
	Mem_space::kernel_space()->switchin_context(oms);
    }

  if (old)
    {
      current()->rcu_wait();
      delete old;
    }

  return true;
}

/** Cut-down version of Thread constructor; only for kernel threads
    Do only what's necessary to get a kernel thread started --
    skip all fancy stuff, no locking is necessary.
    @param task the address space
    @param id user-visible thread ID of the sender
 */
IMPLEMENT inline
Thread::Thread(Context_mode_kernel)
  : Receiver(&_thread_lock), Sender(), _del_observer(0), _magic(magic)
{
  *reinterpret_cast<void(**)()>(--_kernel_sp) = user_invoke;

  inc_ref();

  if (Config::stack_depth)
    std::memset((char*)this + sizeof(Thread), '5',
		Config::thread_block_size-sizeof(Thread)-64);
}


/** Destructor.  Reestablish the Context constructor's precondition.
    @pre current() == thread_lock()->lock_owner()
         && state() == Thread_dead
    @pre lock_cnt() == 0
    @post (_kernel_sp == 0)  &&  (* (stack end) == 0)  &&  !exists()
 */
PUBLIC virtual
Thread::~Thread()		// To be called in locked state.
{

  unsigned long *init_sp = reinterpret_cast<unsigned long*>
    (reinterpret_cast<unsigned long>(this) + size - sizeof(Entry_frame));


  _kernel_sp = 0;
  *--init_sp = 0;
  Fpu_alloc::free_state(fpu_state());
  state_change(0, Thread_invalid);
}

PUBLIC
void
Thread::destroy(Kobject ***rl)
{
  Kobject::destroy(rl);
  check_kdb(kill());
#if 0
  assert_kdb(state() == Thread_dead);
#endif
  assert_kdb(_magic == magic);

}


// IPC-gate deletion stuff ------------------------------------

PUBLIC inline
void
Thread::ipc_gate_deleted(Mword id)
{
  (void) id;
  Lock_guard<Cpu_lock> g(&cpu_lock);
  if (_del_observer)
    _del_observer->hit();
}

class Del_irq_pin : public Irq_pin_dummy
{
};

PUBLIC inline
Del_irq_pin::Del_irq_pin(Thread *o)
{ payload()[0] = (Address)o; }

PUBLIC inline
Thread *
Del_irq_pin::thread() const
{ return (Thread*)payload()[0]; }

PUBLIC inline
void
Del_irq_pin::unbind_irq()
{
  thread()->remove_delete_irq(); 
}

PUBLIC inline
Del_irq_pin::~Del_irq_pin()
{
  unbind_irq();
}

PUBLIC
void
Thread::register_delete_irq(Irq_base *irq)
{
  irq->pin()->unbind_irq();
  irq->pin()->replace<Del_irq_pin>(this);
  _del_observer = irq;
}

PUBLIC
void
Thread::remove_delete_irq()
{
  if (!_del_observer)
    return;

  Irq_base *tmp = _del_observer;
  _del_observer = 0;
  tmp->pin()->unbind_irq();
}

// end of: IPC-gate deletion stuff -------------------------------

PUBLIC virtual
bool
Thread::put()
{ return dec_ref() == 0; }




/** Lookup function: Find Thread instance that owns a given Context.
    @param c a context
    @return the thread that owns the context
 */
PUBLIC static inline
Thread*
Thread::lookup (Context* c)
{
  return reinterpret_cast<Thread*>(c);
}

PUBLIC static inline
Thread const *
Thread::lookup (Context const * c)
{
  return reinterpret_cast<Thread const *>(c);
}

/** Currently executing thread.
    @return currently executing thread.
 */
inline
Thread*
current_thread()
{
  return Thread::lookup(current());
}

PUBLIC inline
bool
Thread::exception_triggered() const
{ return _exc_cont.valid(); }

//
// state requests/manipulation
//


/** Thread lock.
    Overwrite Context's version of thread_lock() with a semantically
    equivalent, but more efficient version.
    @return lock used to synchronize accesses to the thread.
 */
PUBLIC inline
Thread_lock *
Thread::thread_lock()
{
  return &_thread_lock;
}


PUBLIC inline NEEDS ["config.h", "timeout.h"]
void
Thread::handle_timer_interrupt()
{
  unsigned _cpu = cpu(true);
  // XXX: This assumes periodic timers (i.e. bogus in one-shot mode)
  if (!Config::fine_grained_cputime)
    consume_time(Config::scheduler_granularity);

  bool resched = Rcu::do_pending_work(_cpu);

  // Check if we need to reschedule due to timeouts or wakeups
  if ((Timeout_q::timeout_queue.cpu(_cpu).do_timeouts() || resched)
      && !schedule_in_progress())
    {
      schedule();
      assert (timeslice_timeout.cpu(cpu(true))->is_set());	// Coma check
    }
}


PUBLIC
void
Thread::halt()
{
  // Cancel must be cleared on all kernel entry paths. See slowtraps for
  // why we delay doing it until here.
  state_del (Thread_cancel);

  // we haven't been re-initialized (cancel was not set) -- so sleep
  if (state_change_safely (~Thread_ready, Thread_cancel | Thread_dead))
    while (! (state() & Thread_ready))
      schedule();
}

PUBLIC static
void
Thread::halt_current ()
{
  for (;;)
    {
      current_thread()->halt();
      kdb_ke("Thread not halted");
    }
}

PRIVATE static inline
void
Thread::user_invoke_generic()
{
  Context *const c = current();
  assert_kdb (c->state() & Thread_ready_mask);

  if (c->handle_drq() && !c->schedule_in_progress())
    c->schedule();

  // release CPU lock explicitly, because
  // * the context that switched to us holds the CPU lock
  // * we run on a newly-created stack without a CPU lock guard
  cpu_lock.clear();
}


PRIVATE static void
Thread::leave_and_kill_myself()
{
  current_thread()->do_kill();
#ifdef CONFIG_JDB
  WARN("dead thread scheduled: %lx\n", current_thread()->dbg_id());
#endif
  kdb_ke("DEAD SCHED");
}

PUBLIC static
unsigned
Thread::handle_kill_helper(Drq *src, Context *, void *)
{
  delete nonull_static_cast<Thread*>(src->context());
  return Drq::No_answer | Drq::Need_resched;
}



PRIVATE
bool
Thread::do_kill()
{
  Lock_guard<Thread_lock> guard(thread_lock());

  if (state() == Thread_invalid)
    return false;


  unset_utcb_ptr();


  //
  // Kill this thread
  //

  // But first prevent it from being woken up by asynchronous events

  {
    Lock_guard <Cpu_lock> guard (&cpu_lock);

    // if IPC timeout active, reset it
    if (_timeout)
      _timeout->reset();

    // Switch to time-sharing mode
    set_mode (Sched_mode (0));

    // Switch to time-sharing scheduling context
    if (sched() != sched_context())
      switch_sched(sched_context());

    if (current_sched()->context() == this)
      set_current_sched(current()->sched());
  }

  // possibly dequeue from a wait queue
  wait_queue_kill();

  // if other threads want to send me IPC messages, abort these
  // operations
  {
    Lock_guard <Cpu_lock> guard (&cpu_lock);
    while (Sender *s = Sender::cast(sender_list()->head()))
      {
	s->ipc_receiver_aborted();
	Proc::preemption_point();
      }
  }

  // if engaged in IPC operation, stop it
  if (receiver())
    sender_dequeue (receiver()->sender_list());

  Context::do_kill();

  vcpu_update_state();

  unbind();
  vcpu_set_user_space(0);

  cpu_lock.lock();

  state_change_dirty (0, Thread_dead);

  // dequeue from system queues
  ready_dequeue();

  if (_del_observer)
    {
      _del_observer->pin()->unbind_irq();
      _del_observer = 0;
    }

  if (dec_ref())
    while (1)
      {
	state_del_dirty(Thread_ready_mask);
	schedule();
	WARN("woken up dead thread %lx\n", dbg_id());
	kdb_ke("X");
      }

  rcu_wait();

  state_del_dirty(Thread_ready_mask);

  ready_dequeue();

  kernel_context_drq(handle_kill_helper, 0);
  kdb_ke("Im dead");
  return true;
}

PRIVATE static
unsigned
Thread::handle_remote_kill(Drq *, Context *self, void *)
{
  Thread *c = nonull_static_cast<Thread*>(self);
  c->state_add_dirty(Thread_cancel | Thread_ready);
  c->_exc_cont.restore(c->regs());
  c->do_trigger_exception(c->regs(), (void*)&Thread::leave_and_kill_myself);
  return 0;
}


PRIVATE
bool
Thread::kill()
{
  Lock_guard<Cpu_lock> guard(&cpu_lock);
  inc_ref();


  if (cpu() == current_cpu())
    {
      state_add_dirty(Thread_cancel | Thread_ready);
      sched()->deblock(cpu());
      _exc_cont.restore(regs()); // overwrite an already triggered exception
      do_trigger_exception(regs(), (void*)&Thread::leave_and_kill_myself);
//          current()->switch_exec (this, Helping);
      return true;
    }

  drq(Thread::handle_remote_kill, 0, 0, Drq::Any_ctxt);

  return true;
#if 0
    drq(Thread::handle_migration, reinterpret_cast<void*>(current_cpu()));

  assert_kdb(cpu() == current_cpu());

  return do_kill();
#endif
}


PUBLIC
void
Thread::set_sched_params(unsigned prio, Unsigned64 quantum)
{
  Sched_context *sc = sched_context();
  bool const change = prio != sc->prio()
                   || quantum != sc->quantum();
  bool const ready_queued = in_ready_list();

  if (!change && (ready_queued || this == current()))
    return;

  ready_dequeue();

  sc->set_prio(prio);
  sc->set_quantum(quantum);
  sc->replenish();

  if (sc == current_sched())
    set_current_sched(sc);

  if (state() & Thread_ready_mask)
    {
      if (this != current())
        ready_enqueue();
      else
        schedule();
    }
}

PUBLIC
long
Thread::control(Thread_ptr const &pager, Thread_ptr const &exc_handler,
                Space *task, void *user_utcb,
                bool utcb_vcpu_flag = false, bool utcb_vcpu_val = false)
{
  bool new_vcpu_state = state() & Thread_vcpu_enabled;
  if (utcb_vcpu_flag)
    new_vcpu_state = utcb_vcpu_val;

  if (task)
    {
      if (EXPECT_FALSE(!task->is_utcb_valid(user_utcb, 1 + new_vcpu_state)))
        return -L4_err::EInval;

      if (EXPECT_FALSE(!bind(task, user_utcb)))
        return -L4_err::EInval; // unbind first !!

      if (new_vcpu_state)
        vcpu_state(utcb() + 1);
    }

  if (new_vcpu_state)
    {
      if (space())
        {
	  if (!space()->is_utcb_valid((void *)local_id(), 2))
            return -L4_err::EInval;
          vcpu_state(utcb() + 1);
        }

      state_add_dirty(Thread_vcpu_enabled);
    }
  else
    // we're not clearing the vcpu_state pointer, it's not used if vcpu mode
    // is off
    state_del_dirty(Thread_vcpu_enabled);

  if (pager.is_valid())
    _pager = pager;

  if (exc_handler.is_valid())
    _exc_handler = exc_handler;

  return 0;
}


/** Clears the utcb pointer of the Thread
 *  Reason: To avoid a stale pointer after unmapping and deallocating
 *  the UTCB. Without this the Thread_lock::clear will access the UTCB
 *  after the unmapping the UTCB -> POOFFF.
 */
PUBLIC inline
void
Thread::unset_utcb_ptr()
{
  utcb(0);
  local_id(0);
}


PRIVATE static inline
bool FIASCO_WARN_RESULT
Thread::copy_utcb_to_utcb(L4_msg_tag const &tag, Thread *snd, Thread *rcv,
                          unsigned char rights)
{
  assert (cpu_lock.test());

  Utcb *snd_utcb = snd->access_utcb();
  Utcb *rcv_utcb = rcv->access_utcb();
  Mword s = tag.words();
  Mword r = Utcb::Max_words;

  Mem::memcpy_mwords (rcv_utcb->values, snd_utcb->values, r < s ? r : s);

  bool success = true;
  if (tag.items())
    success = transfer_msg_items(tag, snd, snd_utcb, rcv, rcv_utcb, rights);

  if (tag.transfer_fpu() && rcv_utcb->inherit_fpu() && (rights & L4_fpage::W))
    snd->transfer_fpu(rcv);

  return success;
}


PUBLIC inline NEEDS[Thread::copy_utcb_to_ts, Thread::copy_utcb_to_utcb,
                    Thread::copy_ts_to_utcb]
bool FIASCO_WARN_RESULT
Thread::copy_utcb_to(L4_msg_tag const &tag, Thread* receiver,
                     unsigned char rights)
{
  // we cannot copy trap state to trap state!
  assert_kdb (!this->_utcb_handler || !receiver->_utcb_handler);
  if (EXPECT_FALSE(this->_utcb_handler != 0))
    return copy_ts_to_utcb(tag, this, receiver, rights);
  else if (EXPECT_FALSE(receiver->_utcb_handler != 0))
    return copy_utcb_to_ts(tag, this, receiver, rights);
  else
    return copy_utcb_to_utcb(tag, this, receiver, rights);
}


PUBLIC inline
void
Thread::recover_jmp_buf(jmp_buf *b)
{ _recover_jmpbuf = b; }


PUBLIC static inline
bool
Thread::is_tcb_address(Address a)
{
  a &= ~(Config::thread_block_size - 1);
  return reinterpret_cast<Thread *>(a)->_magic == magic;
}

PUBLIC static inline
void
Thread::assert_irq_entry()
{
  assert_kdb(current_thread()->schedule_in_progress()
             || current_thread()->state() & (Thread_ready_mask | Thread_drq_wait | Thread_waiting));
}


// ---------------------------------------------------------------------------

PUBLIC inline
Obj_cap::Obj_cap(L4_obj_ref const &o) : L4_obj_ref(o) {}

PUBLIC inline NEEDS["kobject.h"]
Kobject_iface *
Obj_cap::deref(unsigned char *rights = 0, bool dbg = false)
{
  Thread *current = current_thread();
  if (flags() & L4_obj_ref::Ipc_reply)
    {
      if (rights) *rights = current->caller_rights();
      Thread *ca = static_cast<Thread*>(current->caller());
      if (!dbg)
	current->set_caller(0,0);
      return ca;
    }

  if (EXPECT_FALSE(invalid()))
    {
      if (!self())
	return 0;

      if (rights) *rights = L4_fpage::RWX;
      return current_thread();
    }

  return current->space()->obj_space()->lookup_local(cap(), rights);
}

PUBLIC inline NEEDS["kobject.h"]
bool
Obj_cap::revalidate(Kobject_iface *o)
{
  return deref() == o;
}


// ---------------------------------------------------------------------------

PUBLIC inline
bool
Thread::check_sys_ipc(unsigned flags, Thread **partner, Thread **sender,
                      bool *have_recv) const
{
  if (flags & L4_obj_ref::Ipc_recv)
    {
      *sender = flags & L4_obj_ref::Ipc_open_wait ? 0 : const_cast<Thread*>(this);
      *have_recv = true;
    }

  if (flags & L4_obj_ref::Ipc_send)
    *partner = const_cast<Thread*>(this);

  // FIXME: shall be removed flags == 0 is no-op
  if (!flags)
    {
      *sender = const_cast<Thread*>(this);
      *partner = const_cast<Thread*>(this);
      *have_recv = true;
    }

  return *have_recv || ((flags & L4_obj_ref::Ipc_send) && *partner);
}



PUBLIC
void
Thread::invoke(L4_obj_ref /*self*/, Mword rights, Syscall_frame *f, Utcb *utcb)
{
  register unsigned flags = f->ref().flags();
  if (((flags != 0) && !(flags & L4_obj_ref::Ipc_send))
      || (flags & L4_obj_ref::Ipc_reply)
      || f->tag().proto() != L4_msg_tag::Label_thread)
    {
      /* we do IPC */
      Thread *ct = current_thread();
      Thread *sender = 0;
      Thread *partner = 0;
      bool have_rcv = false;

      if (EXPECT_FALSE(!check_sys_ipc(flags, &partner, &sender, &have_rcv)))
	{
	  utcb->error = L4_error::Not_existent;
	  return;
	}

      ct->do_ipc(f->tag(), partner, partner, have_rcv, sender,
                 f->timeout(), f, rights);
      return;
    }

  switch (utcb->values[0] & Opcode_mask)
    {
    case Op_control:
      f->tag(sys_control(rights, f->tag(), utcb));
      return;
    case Op_ex_regs:
      f->tag(sys_ex_regs(f->tag(), utcb));
      return;
    case Op_switch:
      f->tag(sys_thread_switch(f->tag(), utcb));
      return;
    case Op_stats:
      f->tag(sys_thread_stats(f->tag(), utcb));
      return;
    case Op_vcpu_resume:
      f->tag(sys_vcpu_resume(f->tag(), utcb));
      return;
    case Op_register_del_irq:
      f->tag(sys_register_delete_irq(f->tag(), utcb, utcb));
      return;
    case Op_modify_senders:
      f->tag(sys_modify_senders(f->tag(), utcb, utcb));
      return;
    default:
      L4_msg_tag tag = f->tag();
      if (invoke_arch(tag, utcb))
	f->tag(tag);
      else
        f->tag(commit_result(-L4_err::ENosys));
      return;
    }
}

PRIVATE inline NOEXPORT
L4_msg_tag
Thread::sys_modify_senders(L4_msg_tag tag, Utcb const *in, Utcb * /*out*/)
{
  if (sender_list()->cursor())
    return Kobject_iface::commit_result(-L4_err::EBusy);

  if (0)
    printf("MODIFY ID (%08lx:%08lx->%08lx:%08lx\n",
           in->values[1], in->values[2],
           in->values[3], in->values[4]);


  int elems = tag.words();

  if (elems < 5)
    return Kobject_iface::commit_result(0);

  --elems;

  elems = elems / 4;

  ::Prio_list_elem *c = sender_list()->head();
  while (c)
    {
      // this is kind of arbitrary
      for (int cnt = 50; c && cnt > 0; --cnt)
	{
	  Sender *s = Sender::cast(c);
	  s->modify_label(&in->values[1], elems);
	  c = c->next();
	}

      if (!c)
	return Kobject_iface::commit_result(0);

      sender_list()->cursor(c);
      Proc::preemption_point();
      c = sender_list()->cursor();
    }
  return Kobject_iface::commit_result(0);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Thread::sys_register_delete_irq(L4_msg_tag tag, Utcb const *in, Utcb * /*out*/)
{
  L4_snd_item_iter snd_items(in, tag.words());

  if (!tag.items() || !snd_items.next())
    return Kobject_iface::commit_result(-L4_err::EInval);

  L4_fpage bind_irq(snd_items.get()->d);
  if (EXPECT_FALSE(!bind_irq.is_objpage()))
    return Kobject_iface::commit_error(in, L4_error::Overflow);

  register Context *const c_thread = ::current();
  register Space *const c_space = c_thread->space();
  register Obj_space *const o_space = c_space->obj_space();
  unsigned char irq_rights = 0;
  Irq_base *irq
    = Irq_base::dcast(o_space->lookup_local(bind_irq.obj_index(), &irq_rights));

  if (!irq)
    return Kobject_iface::commit_result(-L4_err::EInval);

  if (EXPECT_FALSE(!(irq_rights & L4_fpage::X)))
    return Kobject_iface::commit_result(-L4_err::EPerm);

  register_delete_irq(irq);
  return Kobject_iface::commit_result(0);
}


PRIVATE inline NOEXPORT
L4_msg_tag
Thread::sys_control(unsigned char rights, L4_msg_tag const &tag, Utcb *utcb)
{
  if (EXPECT_FALSE(!(rights & L4_fpage::W)))
    return commit_result(-L4_err::EPerm);

  if (EXPECT_FALSE(tag.words() < 6))
    return commit_result(-L4_err::EInval);

  Context *curr = current();
  Obj_space *s = curr->space()->obj_space();
  L4_snd_item_iter snd_items(utcb, tag.words());
  Task *task = 0;
  void *utcb_addr = 0;

  Mword flags = utcb->values[0];

  Mword _old_pager = _pager.raw() << L4_obj_ref::Cap_shift;
  Mword _old_exc_handler = _exc_handler.raw() << L4_obj_ref::Cap_shift;

  Thread_ptr _new_pager(~0UL);
  Thread_ptr _new_exc_handler(~0UL);

  if (flags & Ctl_set_pager)
    _new_pager = Thread_ptr(utcb->values[1] >> L4_obj_ref::Cap_shift);

  if (flags & Ctl_set_exc_handler)
    _new_exc_handler = Thread_ptr(utcb->values[2] >> L4_obj_ref::Cap_shift);

  if (flags & Ctl_bind_task)
    {
      if (EXPECT_FALSE(!tag.items() || !snd_items.next()))
	return commit_result(-L4_err::EInval);

      L4_fpage bind_task(snd_items.get()->d);

      if (EXPECT_FALSE(!bind_task.is_objpage()))
	return commit_result(-L4_err::EInval);

      unsigned char task_rights = 0;
      task = Kobject::dcast<Task*>(s->lookup_local(bind_task.obj_index(), &task_rights));

      if (EXPECT_FALSE(!(task_rights & L4_fpage::W)))
	return commit_result(-L4_err::EPerm);

      if (!task)
	return commit_result(-L4_err::EInval);

      utcb_addr = (void*)utcb->values[5];
    }

  long res = control(_new_pager, _new_exc_handler,
                     task, utcb_addr, flags & Ctl_vcpu_enabled,
                     utcb->values[4] & Ctl_vcpu_enabled);

  if (res < 0)
    return commit_result(res);

  if ((res = sys_control_arch(utcb)) < 0)
    return commit_result(res);

    {
      // FIXME: must be done xcpu safe, may be some parts above too
      Lock_guard<Cpu_lock> guard(&cpu_lock);
      if (flags & Ctl_alien_thread)
        {
	  if (utcb->values[4] & Ctl_alien_thread)
	    state_change_dirty (~Thread_dis_alien, Thread_alien, false);
	  else
	    state_del_dirty(Thread_alien, false);
	}
    }

  utcb->values[1] = _old_pager;
  utcb->values[2] = _old_exc_handler;

  return commit_result(0, 3);
}

// -------------------------------------------------------------------
// Thread::ex_regs class system calls

PUBLIC
bool
Thread::ex_regs(Address ip, Address sp,
                Address *o_ip = 0, Address *o_sp = 0, Mword *o_flags = 0,
                Mword ops = 0)
{
  if (state(false) == Thread_invalid || !space())
    return false;

  if (current() == this)
    spill_user_state();

  if (o_sp) *o_sp = user_sp();
  if (o_ip) *o_ip = user_ip();
  if (o_flags) *o_flags = user_flags();

  // Changing the run state is only possible when the thread is not in
  // an exception.
  if (!(ops & Exr_cancel) && (state(false) & Thread_in_exception))
    // XXX Maybe we should return false here.  Previously, we actually
    // did so, but we also actually didn't do any state modification.
    // If you change this value, make sure the logic in
    // sys_thread_ex_regs still works (in particular,
    // ex_regs_cap_handler and friends should still be called).
    return true;

  if (state(false) & Thread_dead)	// resurrect thread
    state_change_dirty (~Thread_dead, Thread_ready, false);

  else if (ops & Exr_cancel)
    // cancel ongoing IPC or other activity
    state_change_dirty (~(Thread_ipc_in_progress | Thread_delayed_deadline |
                        Thread_delayed_ipc), Thread_cancel | Thread_ready, false);

  if (ops & Exr_trigger_exception)
    {
      extern char leave_by_trigger_exception[];
      do_trigger_exception(regs(), leave_by_trigger_exception);
    }

  if (ip != ~0UL)
    user_ip(ip);

  if (sp != ~0UL)
    user_sp (sp);

  if (current() == this)
    fill_user_state();

  return true;
}

PUBLIC inline
L4_msg_tag
Thread::ex_regs(Utcb *utcb)
{
  Address ip = utcb->values[1];
  Address sp = utcb->values[2];
  Mword flags;
  Mword ops = utcb->values[0];

  LOG_TRACE("Ex-regs", "exr", current(), __fmt_thread_exregs,
      Log_thread_exregs *l = tbe->payload<Log_thread_exregs>();
      l->id = dbg_id();
      l->ip = ip; l->sp = sp; l->op = ops;);

  if (!ex_regs(ip, sp, &ip, &sp, &flags, ops))
    return commit_result(-L4_err::EInval);

  utcb->values[0] = flags;
  utcb->values[1] = ip;
  utcb->values[2] = sp;

  return commit_result(0, 3);
}

PRIVATE static
unsigned
Thread::handle_remote_ex_regs(Drq *, Context *self, void *p)
{
  Remote_syscall *params = reinterpret_cast<Remote_syscall*>(p);
  params->result = nonull_static_cast<Thread*>(self)->ex_regs(params->thread->access_utcb());
  return params->result.proto() == 0 ? Drq::Need_resched : 0;
}

PRIVATE inline NOEXPORT
L4_msg_tag
Thread::sys_ex_regs(L4_msg_tag const &tag, Utcb * /*utcb*/)
{
  if (tag.words() != 3)
    return commit_result(-L4_err::EInval);

  Remote_syscall params;
  params.thread = current_thread();

  drq(handle_remote_ex_regs, &params, 0, Drq::Any_ctxt);
  return params.result;
}

PRIVATE inline NOEXPORT NEEDS["timer.h"]
L4_msg_tag
Thread::sys_thread_switch(L4_msg_tag const &/*tag*/, Utcb *utcb)
{
  Context *curr = current();

  if (curr == this)
    return commit_result(0);

  if (current_cpu() != cpu())
    return commit_result(0);

#ifdef FIXME
  Sched_context * const cs = current_sched();
#endif

  if (curr != this
      && ((state() & (Thread_ready | Thread_suspended)) == Thread_ready))
    {
      curr->switch_exec_schedule_locked (this, Not_Helping);
      *(Unsigned64*)((void*)(utcb->values)) = 0; // Assume timeslice was used up
      return commit_result(0, 2);
    }

#if 0 // FIXME: provide API for multiple sched contexts
      // Compute remaining quantum length of timeslice
      regs->left (timeslice_timeout.cpu(cpu())->get_timeout(Timer::system_clock()));

      // Yield current global timeslice
      cs->owner()->switch_sched (cs->id() ? cs->next() : cs);
#endif
  *(Unsigned64*)((void*)(utcb->values)) = timeslice_timeout.cpu(current_cpu())->get_timeout(Timer::system_clock());
  curr->schedule();

  return commit_result(0,2);
}



// -------------------------------------------------------------------
// Gather statistics information about thread execution

PRIVATE
unsigned
Thread::sys_thread_stats_remote(void *data)
{
  update_consumed_time();
  *(Clock::Time *)data = consumed_time();
  return 0;
}

PRIVATE static
unsigned
Thread::handle_sys_thread_stats_remote(Drq *, Context *self, void *data)
{
  return nonull_static_cast<Thread*>(self)->sys_thread_stats_remote(data);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Thread::sys_thread_stats(L4_msg_tag const &/*tag*/, Utcb *utcb)
{
  Clock::Time value;

  if (cpu() != current_cpu())
    drq(handle_sys_thread_stats_remote, &value, 0, Drq::Any_ctxt);
  else
    {
      // Respect the fact that the consumed time is only updated on context switch
      if (this == current())
        update_consumed_time();
      value = consumed_time();
    }

  *(Cpu_time *)((void*)(utcb->values)) = value;

  return commit_result(0, sizeof(Cpu_time) / sizeof(Mword));
}


PUBLIC static
unsigned
Thread::handle_migration_helper(Drq *, Context *, void *p)
{
  Migration_helper_info const *inf = (Migration_helper_info const *)p;
  return inf->victim->migration_helper(&inf->inf);
}


PRIVATE
void
Thread::do_migration()
{
  assert_kdb(cpu_lock.test());
  assert_kdb(current_cpu() == cpu(true));

  Migration_helper_info inf;

    {
      Lock_guard<Spin_lock> g(affinity_lock());
      inf.inf = _migration_rq.inf;
      _migration_rq.pending = false;
      _migration_rq.in_progress = true;
    }

  unsigned on_cpu = cpu();

  if (inf.inf.cpu == ~0U)
    {
      state_add_dirty(Thread_suspended);
      set_sched_params(0, 0);
      _migration_rq.in_progress = false;
      return;
    }

  state_del_dirty(Thread_suspended);

  if (inf.inf.cpu == on_cpu)
    {
      // stay here
      set_sched_params(inf.inf.prio, inf.inf.quantum);
      _migration_rq.in_progress = false;
      return;
    }

  // spill FPU state into memory before migration
  if (state() & Thread_fpu_owner)
    {
      if (current() != this)
	Fpu::enable();

      spill_fpu();
      Fpu::set_owner(on_cpu, 0);
      Fpu::disable();
    }


  // if we are in the middle of the scheduler, leave it now
  if (schedule_in_progress() == this)
    reset_schedule_in_progress();

  inf.victim = this;

  if (current() == this && Config::Max_num_cpus > 1)
    kernel_context_drq(handle_migration_helper, &inf);
  else
    migration_helper(&inf.inf);
}

PUBLIC
void
Thread::initiate_migration()
{ do_migration(); }

PUBLIC
void
Thread::finish_migration()
{ enqueue_timeout_again(); }


PUBLIC
void
Thread::migrate(Migration_info const &info)
{
  assert_kdb (cpu_lock.test());

  LOG_TRACE("Thread migration", "mig", this, __thread_migration_log_fmt,
      Migration_log *l = tbe->payload<Migration_log>();
      l->state = state();
      l->src_cpu = cpu();
      l->target_cpu = info.cpu;
      l->user_ip = regs()->ip();
  );

    {
      Lock_guard<Spin_lock> g(affinity_lock());
      _migration_rq.inf = info;
      _migration_rq.pending = true;
    }

  unsigned cpu = this->cpu();

  if (current_cpu() == cpu)
    {
      do_migration();
      return;
    }

  migrate_xcpu(cpu);
}


//---------------------------------------------------------------------------
IMPLEMENTATION [fpu && !ux]:

#include "fpu.h"
#include "fpu_alloc.h"
#include "fpu_state.h"

PUBLIC inline NEEDS ["fpu.h"]
void
Thread::spill_fpu()
{
  // If we own the FPU, we should never be getting an "FPU unavailable" trap
  assert_kdb (Fpu::owner(cpu()) == this);
  assert_kdb (state() & Thread_fpu_owner);
  assert_kdb (fpu_state());

  // Save the FPU state of the previous FPU owner (lazy) if applicable
  Fpu::save_state (fpu_state());
  state_del_dirty (Thread_fpu_owner);
}


/*
 * Handle FPU trap for this context. Assumes disabled interrupts
 */
PUBLIC inline NEEDS [Thread::spill_fpu, "fpu_alloc.h","fpu_state.h"]
int
Thread::switchin_fpu(bool alloc_new_fpu = true)
{
  unsigned cpu = this->cpu(true);

  if (state() & Thread_vcpu_fpu_disabled)
    return 0;

  // If we own the FPU, we should never be getting an "FPU unavailable" trap
  assert_kdb (Fpu::owner(cpu) != this);

  // Allocate FPU state slab if we didn't already have one
  if (!fpu_state()->state_buffer()
      && (EXPECT_FALSE((!alloc_new_fpu
                        || (state() & Thread_alien))
                       || !Fpu_alloc::alloc_state (_quota, fpu_state()))))
    return 0;

  // Enable the FPU before accessing it, otherwise recursive trap
  Fpu::enable();

  // Save the FPU state of the previous FPU owner (lazy) if applicable
  if (Fpu::owner(cpu))
    nonull_static_cast<Thread*>(Fpu::owner(cpu))->spill_fpu();

  // Become FPU owner and restore own FPU state
  Fpu::restore_state (fpu_state());

  state_add_dirty (Thread_fpu_owner);
  Fpu::set_owner (cpu, this);
  return 1;
}

PUBLIC inline NEEDS["fpu.h", "fpu_alloc.h"]
void
Thread::transfer_fpu(Thread *to)
{
  unsigned cpu = this->cpu();
  if (cpu != to->cpu())
    return;

  if (to->fpu_state()->state_buffer())
    Fpu_alloc::free_state(to->fpu_state());

  to->fpu_state()->state_buffer(fpu_state()->state_buffer());
  fpu_state()->state_buffer(0);

  assert (current() == this || current() == to);

  Fpu::disable(); // it will be reanabled in switch_fpu

  if (EXPECT_FALSE(Fpu::owner(cpu) == to))
    {
      assert_kdb (to->state() & Thread_fpu_owner);

      Fpu::set_owner(cpu, 0);
      to->state_del_dirty (Thread_fpu_owner);
    }
  else if (Fpu::owner(cpu) == this)
    {
      assert_kdb (state() & Thread_fpu_owner);

      state_del_dirty (Thread_fpu_owner);

      to->state_add_dirty (Thread_fpu_owner);
      Fpu::set_owner(cpu, to);
      if (EXPECT_FALSE(current() == to))
        Fpu::enable();
    }
}

//---------------------------------------------------------------------------
IMPLEMENTATION [!fpu]:

PUBLIC inline
int
Thread::switchin_fpu(bool alloc_new_fpu = true)
{ (void)alloc_new_fpu;
  return 0;
}

PUBLIC inline
void
Thread::spill_fpu()
{}

//---------------------------------------------------------------------------
IMPLEMENTATION [!fpu || ux]:

PUBLIC inline
void
Thread::transfer_fpu(Thread *)
{}

//---------------------------------------------------------------------------
IMPLEMENTATION [!log]:

PUBLIC inline
unsigned Thread::sys_ipc_log(Syscall_frame *)
{ return 0; }

PUBLIC inline
unsigned Thread::sys_ipc_trace(Syscall_frame *)
{ return 0; }

static inline
void Thread::page_fault_log(Address, unsigned, unsigned)
{}

PUBLIC static inline
int Thread::log_page_fault()
{ return 0; }

PUBLIC inline
unsigned Thread::sys_fpage_unmap_log(Syscall_frame *)
{ return 0; }

//---------------------------------------------------------------------------
IMPLEMENTATION [!io]:

PUBLIC inline
bool
Thread::has_privileged_iopl()
{
  return false;
}


// ----------------------------------------------------------------------------
IMPLEMENTATION [!mp]:


PRIVATE inline
unsigned
Thread::migration_helper(Migration_info const *inf)
{
  unsigned cpu = inf->cpu;
  //  LOG_MSG_3VAL(this, "MGi ", Mword(current()), (current_cpu() << 16) | cpu(), Context::current_sched());
  if (_timeout)
    _timeout->reset();
  ready_dequeue();

    {
      // Not sure if this can ever happen
      Sched_context *csc = Context::current_sched();
      if (!csc || csc->context() == this)
	Context::set_current_sched(current()->sched());
    }

  Sched_context *sc = sched_context();
  sc->set_prio(inf->prio);
  sc->set_quantum(inf->quantum);
  sc->replenish();
  set_sched(sc);

  if (drq_pending())
    state_add_dirty(Thread_drq_ready);

  set_cpu_of(this, cpu);
  return  Drq::No_answer | Drq::Need_resched;
}

PRIVATE inline
void
Thread::migrate_xcpu(unsigned cpu)
{
  (void)cpu;
  assert_kdb (false);
}


//----------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Thread
{
protected:
  struct Migration_log
  {
    Mword    state;
    Address  user_ip;
    unsigned src_cpu;
    unsigned target_cpu;

    static unsigned fmt(Tb_entry *, int, char *)
    asm ("__thread_migration_log_fmt");
  };
};


// ----------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include "ipi.h"

IMPLEMENT
void
Thread::handle_remote_requests_irq()
{ assert_kdb (cpu_lock.test());
  // printf("CPU[%2u]: > RQ IPI (current=%p)\n", current_cpu(), current());
  Ipi::eoi(Ipi::Request);
  Context *const c = current();
  //LOG_MSG_3VAL(c, "ipi", c->cpu(), (Mword)c, c->drq_pending());
  Context *migration_q = 0;
  bool resched = _pending_rqq.cpu(c->cpu()).handle_requests(&migration_q);

  resched |= Rcu::do_pending_work(c->cpu());

  if (migration_q)
    static_cast<Thread*>(migration_q)->do_migration();

  if ((resched || c->handle_drq()) && !c->schedule_in_progress())
    {
      //LOG_MSG_3VAL(c, "ipis", 0, 0, 0);
      // printf("CPU[%2u]: RQ IPI sched %p\n", current_cpu(), current());
      c->schedule();
    }
  // printf("CPU[%2u]: < RQ IPI (current=%p)\n", current_cpu(), current());
}

IMPLEMENT
void
Thread::handle_global_remote_requests_irq()
{ assert_kdb (cpu_lock.test());
  // printf("CPU[%2u]: > RQ IPI (current=%p)\n", current_cpu(), current());
  Ipi::eoi(Ipi::Global_request);
  Context::handle_global_requests();
}

PRIVATE inline
unsigned
Thread::migration_helper(Migration_info const *inf)
{
  // LOG_MSG_3VAL(this, "MGi ", Mword(current()), (current_cpu() << 16) | cpu(), 0);
  assert_kdb (cpu() == current_cpu());
  assert_kdb (current() != this);
  assert_kdb (cpu_lock.test());

  if (_timeout)
    _timeout->reset();
  ready_dequeue();

    {
      // Not sure if this can ever happen
      Sched_context *csc = Context::current_sched();
      if (!csc || csc->context() == this)
	Context::set_current_sched(current()->sched());
    }

  unsigned cpu = inf->cpu;

    {
      Queue &q = _pending_rqq.cpu(current_cpu());
      // The queue lock of the current CPU protects the cpu number in
      // the thread
      Lock_guard<Pending_rqq::Inner_lock> g(q.q_lock());

      // potentailly dequeue from our local queue
      if (_pending_rq.queued())
	check_kdb (q.dequeue(&_pending_rq, Queue_item::Ok));

      Sched_context *sc = sched_context();
      sc->set_prio(inf->prio);
      sc->set_quantum(inf->quantum);
      sc->replenish();
      set_sched(sc);

      if (drq_pending())
	state_add_dirty(Thread_drq_ready);

      Mem::mp_wmb();

      assert_kdb (!in_ready_list());

      set_cpu_of(this, cpu);
      // now we are migrated away fom current_cpu
    }

  bool ipi = true;

    {
      Queue &q = _pending_rqq.cpu(cpu);
      Lock_guard<Pending_rqq::Inner_lock> g(q.q_lock());

      // migrated meanwhile
      if (this->cpu() != cpu || _pending_rq.queued())
	return  Drq::No_answer | Drq::Need_resched;

      if (q.first())
	ipi = false;

      q.enqueue(&_pending_rq);
    }

  if (ipi)
    {
      //LOG_MSG_3VAL(this, "sipi", current_cpu(), cpu(), (Mword)current());
      Ipi::send(cpu, Ipi::Request);
    }

  return  Drq::No_answer | Drq::Need_resched;
}

PRIVATE inline
void
Thread::migrate_xcpu(unsigned cpu)
{
  bool ipi = true;

    {
      Queue &q = Context::_pending_rqq.cpu(cpu);
      Lock_guard<Pending_rqq::Inner_lock> g(q.q_lock());

      // already migrated
      if (cpu != this->cpu())
	return;

      if (q.first())
	ipi = false;

      if (!_pending_rq.queued())
	q.enqueue(&_pending_rq);
      else
	ipi = false;
    }

  if (ipi)
    Ipi::send(cpu, Ipi::Request);
}

//----------------------------------------------------------------------------
IMPLEMENTATION [debug]:

IMPLEMENT
unsigned
Thread::Migration_log::fmt(Tb_entry *e, int maxlen, char *buf)
{
  Migration_log *l = e->payload<Migration_log>();
  return snprintf(buf, maxlen, "migrate from %u to %u (state=%lx user ip=%lx)",
      l->src_cpu, l->target_cpu, l->state, l->user_ip);
}

