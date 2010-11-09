INTERFACE:

#include <csetjmp>             // typedef jmp_buf
#include "types.h"
#include "clock.h"
#include "config.h"
#include "continuation.h"
#include "dlist.h"
#include "fpu_state.h"
#include "globals.h"
#include "l4_types.h"
#include "member_offs.h"
#include "per_cpu_data.h"
#include "queue.h"
#include "queue_item.h"
#include "rcupdate.h"
#include "sched_context.h"
#include "spin_lock.h"
#include "timeout.h"

class Entry_frame;
class Mem_space;
class Space;
class Thread_lock;
class Context;
class Kobject_iface;

class Context_ptr
{
public:
  explicit Context_ptr(unsigned long id) : _t(id) {}
  Context_ptr() {}
  Context_ptr(Context_ptr const &o) : _t(o._t) {}
  Context_ptr const &operator = (Context_ptr const &o)
  { _t = o._t; return *this; }

  Kobject_iface *ptr(Space *, unsigned char *) const;

  bool is_kernel() const { return false; }
  bool is_valid() const { return _t != ~0UL; }

  // only for debugging use
  Mword raw() const { return _t;}

private:
  Mword _t;

};

template< typename T >
class Context_ptr_base : public Context_ptr
{
public:
  enum Invalid_type { Invalid };
  explicit Context_ptr_base(Invalid_type) : Context_ptr(0) {}
  explicit Context_ptr_base(unsigned long id) : Context_ptr(id) {}
  Context_ptr_base() {}
  Context_ptr_base(Context_ptr_base<T> const &o) : Context_ptr(o) {}
  template< typename X >
  Context_ptr_base(Context_ptr_base<X> const &o) : Context_ptr(o)
  { X*x = 0; T*t = x; (void)t; }

  Context_ptr_base<T> const &operator = (Context_ptr_base<T> const &o)
  { Context_ptr::operator = (o); return *this; }

  template< typename X >
  Context_ptr_base<T> const &operator = (Context_ptr_base<X> const &o)
  { X*x=0; T*t=x; (void)t; Context_ptr::operator = (o); return *this; }

  //T *ptr(Space *s) const { return static_cast<T*>(Context_ptr::ptr(s)); }
};




class Present_list_item : public D_list_item
{
protected:
  static Spin_lock _plist_lock;
  static Present_list_item *head;
};




/** An execution context.  A context is a runnable, schedulable activity.
    It carries along some state used by other subsystems: A lock count,
    and stack-element forward/next pointers.
 */
class Context :
  public Global_context_data,
  private Present_list_item,
  protected Rcu_item
{
  MEMBER_OFFSET();
  friend class Jdb_thread_list;
  friend class Context_ptr;
  friend class Jdb_utcb;

protected:
  virtual void finish_migration() = 0;
  virtual void initiate_migration() = 0;

  struct State_request
  {
    Mword add;
    Mword del;
  };

public:
  /**
   * \brief Encapsulate an aggregate of Context.
   *
   * Allow to get a back reference to the aggregating Context object.
   */
  class Context_member
  {
  public:

    /**
     * \brief Get the aggregating Context object.
     */
    Context *context();
  };

  /**
   * \brief Deffered Request.
   *
   * Represents a request that can be queued for each Context
   * and is executed by the target context just after switching to the
   * target context.
   */
  class Drq : public Queue_item, public Context_member
  {
  public:
    typedef unsigned (Request_func)(Drq *, Context *target, void *);
    enum { Need_resched = 1, No_answer = 2 };
    enum Wait_mode { No_wait = 0, Wait = 1 };
    enum Exec_mode { Target_ctxt = 0, Any_ctxt = 1 };
    // enum State { Idle = 0, Handled = 1, Reply_handled = 2 };

    Request_func *func;
    Request_func *reply;
    void *arg;
    // State state;
  };

  /**
   * \brief Queue for deffered requests (Drq).
   *
   * A FIFO queue each Context aggregates to queue incomming Drq's
   * that have to be executed directly after switching to a context.
   */
  class Drq_q : public Queue, public Context_member
  {
  public:
    enum Drop_mode { Drop = true, No_drop = false };
    void enq(Drq *rq);
    bool handle_requests(Drop_mode drop = No_drop);
    bool execute_request(Drq *r, Drop_mode drop, bool local);
  };

  struct Migration_info
  {
    Mword quantum;
    unsigned cpu;
    unsigned short prio;
  };


public:

  /**
   * Definition of different scheduling modes
   */
  enum Sched_mode
  {
    Periodic	= 0x1,	///< 0 = Conventional, 1 = Periodic
    Nonstrict	= 0x2,	///< 0 = Strictly Periodic, 1 = Non-strictly periodic
  };

  /**
   * Definition of different helping modes
   */
  enum Helping_mode {
    Helping,
    Not_Helping,
    Ignore_Helping
  };

  // FIXME: remove this function!
  Mword is_tcb_mapped() const;

  /**
   * Size of a Context (TCB + kernel stack)
   */
  static const size_t size = Config::thread_block_size;

  /**
   * Return consumed CPU time.
   * @return Consumed CPU time in usecs
   */
  Cpu_time consumed_time();

  /**
   * Get the kernel UTCB pointer.
   * @return UTCB pointer, or 0 if there is no UTCB
   */
  Utcb* utcb() const;
  
  /**
   * Get the local ID of the context.
   */
  Local_id local_id() const;

  /**
   * Set the local ID of the context.
   * Does not touch the kernel UTCB pointer, since
   * we would need space() to do the address translation.
   *
   * After setting the local ID and mapping the UTCB area, use
   * Thread::utcb_init() to set the kernel UTCB pointer and initialize the
   * UTCB.
   */
  void local_id (Local_id id);

  virtual bool kill() = 0;

  void spill_user_state();
  void fill_user_state();

protected:
  /**
   * Update consumed CPU time during each context switch and when
   *        reading out the current thread's consumed CPU time.
   */
  void update_consumed_time();
  
  /**
   * Set the kernel UTCB pointer.
   * Does NOT keep the value of _local_id in sync.
   * @see local_id (Local_id id);
   */
  void utcb (Utcb *u);

  Mword   *		_kernel_sp;
  void *_utcb_handler;

private:
  friend class Jdb;
  friend class Jdb_tcb;

  /// low level page table switching stuff
  void switchin_context(Context *) asm ("switchin_context_label") FIASCO_FASTCALL;

  /// low level fpu switching stuff
  void switch_fpu (Context *t);

  /// low level cpu switching stuff
  void switch_cpu (Context *t);

protected:
  Spin_lock_coloc<Space *> _space;

private:
  Context *		_donatee;
  Context *		_helper;

  // Lock state
  // how many locks does this thread hold on other threads
  // incremented in Thread::lock, decremented in Thread::clear
  // Thread::kill needs to know
  int			_lock_cnt;
  Thread_lock * const	_thread_lock;
  
  Local_id _local_id;
  Utcb *_utcb;

  // The scheduling parameters.  We would only need to keep an
  // anonymous reference to them as we do not need them ourselves, but
  // we aggregate them for performance reasons.
  Sched_context		_sched_context;
  Sched_context *	_sched;
  Unsigned64		_period;
  Sched_mode		_mode;

  // Pointer to floating point register state
  Fpu_state		_fpu_state;
  // Implementation-specific consumed CPU time (TSC ticks or usecs)
  Clock::Time           _consumed_time;

  bool _drq_active;
  Drq _drq;
  Drq_q _drq_q;

protected:
  // for trigger_exception
  Continuation _exc_cont;

  jmp_buf *_recover_jmpbuf;     // setjmp buffer for page-fault recovery

  struct Migration_rq
  {
    Migration_info inf;
    bool pending;
    bool in_progress;

    Migration_rq() : pending(false), in_progress(false) {}
  } _migration_rq;

protected:
  // XXX Timeout for both, sender and receiver! In normal case we would have
  // to define own timeouts in Receiver and Sender but because only one
  // timeout can be set at one time we use the same timeout. The timeout
  // has to be defined here because Dirq::hit has to be able to reset the
  // timeout (Irq::_irq_thread is of type Receiver).
  Timeout *          _timeout;
  Spin_lock _affinity_lock;

private:
  static Per_cpu<Clock> _clock;
  static Per_cpu<Context *> _kernel_ctxt;
};


INTERFACE [debug]:

#include "tb_entry.h"

EXTENSION class Context
{
public:
  struct Drq_log
  {
    void *func;
    void *reply;
    Context *thread;
    unsigned target_cpu;
    char const *type;
    bool wait;
  };


  struct Vcpu_log
  {
    Mword state;
    Mword ip;
    Mword sp;
    Mword space;
    Mword err;
    unsigned char type;
    unsigned char trap;
  };

  static unsigned drq_log_fmt(Tb_entry *, int, char *)
  asm ("__context_drq_log_fmt");

};

// --------------------------------------------------------------------------
IMPLEMENTATION:

#include <cassert>
#include "atomic.h"
#include "cpu.h"
#include "cpu_lock.h"
#include "entry_frame.h"
#include "fpu.h"
#include "globals.h"		// current()
#include "kdb_ke.h"
#include "lock_guard.h"
#include "logdefs.h"
#include "mem.h"
#include "mem_layout.h"
#include "processor.h"
#include "space.h"
#include "std_macros.h"
#include "thread_state.h"
#include "timer.h"
#include "timeout.h"

Per_cpu<Clock> DEFINE_PER_CPU Context::_clock(true);
Per_cpu<Context *> DEFINE_PER_CPU Context::_kernel_ctxt;

Spin_lock Present_list_item::_plist_lock INIT_PRIORITY(EARLY_INIT_PRIO);
Present_list_item *Present_list_item::head;

IMPLEMENT inline NEEDS["kdb_ke.h"]
Kobject_iface *
Context_ptr::ptr(Space *s, unsigned char *rights) const
{
  assert_kdb (cpu_lock.test());

  return s->obj_space()->lookup_local(_t, rights);
}



#include <cstdio>

/** Initialize a context.  After setup, a switch_exec to this context results
    in a return to user code using the return registers at regs().  The
    return registers are not initialized however; neither is the space_context
    to be used in thread switching (use set_space_context() for that).
    @pre (_kernel_sp == 0)  &&  (* (stack end) == 0)
    @param thread_lock pointer to lock used to lock this context
    @param space_context the space context
 */
PUBLIC inline NEEDS ["atomic.h", "entry_frame.h", <cstdio>]
Context::Context(Thread_lock *thread_lock)
: _kernel_sp(reinterpret_cast<Mword*>(regs())),
  _helper(this),
  _thread_lock(thread_lock),
  _sched_context(),
  _sched(&_sched_context),
  _mode(Sched_mode (0))
{
  // NOTE: We do not have to synchronize the initialization of
  // _space_context because it is constant for all concurrent
  // invocations of this constructor.  When two threads concurrently
  // try to create a new task, they already synchronize in
  // sys_task_new() and avoid calling us twice with different
  // space_context arguments.

  set_cpu_of(this, current_cpu());

  Lock_guard<Spin_lock> guard(&Present_list_item::_plist_lock);
  if (Present_list_item::head)
    Present_list_item::head->Present_list_item::enqueue(this);
  else
    Present_list_item::head = this;
}


PUBLIC inline
Spin_lock *
Context::affinity_lock()
{ return &_affinity_lock; }

PUBLIC inline
void
Context::do_kill()
{
  // If this context owned the FPU, noone owns it now
  if (Fpu::is_owner(cpu(), this))
    {
      Fpu::set_owner(cpu(), 0);
      Fpu::disable();
    }
}

/** Destroy context.
 */
PUBLIC virtual
Context::~Context()
{
  Lock_guard<Spin_lock> guard(&Present_list_item::_plist_lock);
  if (this == Present_list_item::head)
    {
      if (Present_list_item::next() != this)
	Present_list_item::head = static_cast<Present_list_item*>(Present_list_item::next());
      else
	{
	  Present_list_item::head = 0;
	  return;
	}
    }

  Present_list_item::dequeue();
  
}


PUBLIC inline
Mword
Context::state(bool check = true) const
{
  (void)check;
  assert_2_kdb(!check || cpu() == current_cpu());
  return _state;
}

IMPLEMENT inline
Mword
Context::is_tcb_mapped() const
{ return true; }


PUBLIC static inline
Context*
Context::kernel_context(unsigned cpu)
{ return _kernel_ctxt.cpu(cpu); }

PROTECTED static inline
void
Context::kernel_context(unsigned cpu, Context *ctxt)
{ _kernel_ctxt.cpu(cpu) = ctxt; }


/** @name State manipulation */
//@{
//-


/**
 * Does the context exist? .
 * @return true if this context has been initialized.
 */
PUBLIC inline NEEDS ["thread_state.h"]
Mword
Context::exists() const
{
  return state() != Thread_invalid;
}

/**
 * Is the context about to be deleted.
 * @return true if this context is in deletion.
 */
PUBLIC inline NEEDS ["thread_state.h"]
bool
Context::is_invalid() const
{ return state() == Thread_invalid; }

/**
 * Atomically add bits to state flags.
 * @param bits bits to be added to state flags
 * @return 1 if none of the bits that were added had been set before
 */
PUBLIC inline NEEDS ["atomic.h"]
void
Context::state_add (Mword const bits)
{
  assert_2_kdb(cpu() == current_cpu());
  atomic_or (&_state, bits);
}

/**
 * Add bits in state flags. Unsafe (non-atomic) and
 *        fast version -- you must hold the kernel lock when you use it.
 * @pre cpu_lock.test() == true
 * @param bits bits to be added to state flags
 */ 
PUBLIC inline
void
Context::state_add_dirty (Mword bits)
{ 
  assert_2_kdb(cpu() == current_cpu());
  _state |=bits;
}

/**
 * Atomically delete bits from state flags.
 * @param bits bits to be removed from state flags
 * @return 1 if all of the bits that were removed had previously been set
 */
PUBLIC inline NEEDS ["atomic.h"]
void
Context::state_del (Mword const bits)
{
  assert_2_kdb (current_cpu() == cpu());
  atomic_and (&_state, ~bits);
}

/**
 * Delete bits in state flags. Unsafe (non-atomic) and
 *        fast version -- you must hold the kernel lock when you use it.
 * @pre cpu_lock.test() == true
 * @param bits bits to be removed from state flags
 */
PUBLIC inline
void
Context::state_del_dirty (Mword bits, bool check = true)
{
  (void)check;
  assert_2_kdb(!check || cpu() == current_cpu());
  _state &=~bits;
}

/**
 * Atomically delete and add bits in state flags, provided the
 *        following rules apply (otherwise state is not changed at all):
 *        - Bits that are to be set must be clear in state or clear in mask
 *        - Bits that are to be cleared must be set in state
 * @param mask Bits not set in mask shall be deleted from state flags
 * @param bits Bits to be added to state flags
 * @return 1 if state was changed, 0 otherwise
 */
PUBLIC inline NEEDS ["atomic.h"]
Mword
Context::state_change_safely (Mword const mask, Mword const bits)
{
  assert_2_kdb (current_cpu() == cpu());
  Mword old;

  do
    {
      old = _state;
      if (old & bits & mask | ~old & ~mask)
        return 0;
    }
  while (!cas (&_state, old, old & mask | bits));

  return 1;
}

/**
 * Atomically delete and add bits in state flags.
 * @param mask bits not set in mask shall be deleted from state flags
 * @param bits bits to be added to state flags
 */
PUBLIC inline NEEDS ["atomic.h"]
Mword
Context::state_change (Mword const mask, Mword const bits)
{
  assert_2_kdb (current_cpu() == cpu());
  return atomic_change (&_state, mask, bits);
}

/**
 * Delete and add bits in state flags. Unsafe (non-atomic) and
 *        fast version -- you must hold the kernel lock when you use it.
 * @pre cpu_lock.test() == true
 * @param mask Bits not set in mask shall be deleted from state flags
 * @param bits Bits to be added to state flags
 */
PUBLIC inline
void
Context::state_change_dirty (Mword const mask, Mword const bits, bool check = true)
{
  (void)check;
  assert_2_kdb(!check || cpu() == current_cpu());
  _state &= mask;
  _state |= bits;
}

//@}
//-

/** Return the space context.
    @return space context used for this execution context.
            Set with set_space_context().
 */
PUBLIC inline NEEDS["kdb_ke.h", "cpu_lock.h"]
Space *
Context::space() const
{
  //assert_kdb (cpu_lock.test());
  return _space.get_unused();
}

PUBLIC inline NEEDS[Context::space, Context::vcpu_user_space]
Space *
Context::vcpu_aware_space() const
{
  if (EXPECT_FALSE(state() & Thread_vcpu_user_mode))
    return vcpu_user_space();
  else
    return space();
}

/** Convenience function: Return memory space. */
PUBLIC inline NEEDS["space.h"]
Mem_space*
Context::mem_space() const
{
  return space()->mem_space();
}

/** Thread lock.
    @return the thread lock used to lock this context.
 */
PUBLIC inline
Thread_lock *
Context::thread_lock() const
{
  return _thread_lock;
}


/** Registers used when iret'ing to user mode.
    @return return registers
 */
PUBLIC inline NEEDS["cpu.h", "entry_frame.h"]
Entry_frame *
Context::regs() const
{
  return reinterpret_cast<Entry_frame *>
    (Cpu::stack_align(reinterpret_cast<Mword>(this) + size)) - 1;
}

/** @name Lock counting
    These functions count the number of locks
    this context holds.  A context must not be deleted if its lock
    count is nonzero. */
//@{
//-

/** Increment lock count.
    @post lock_cnt() > 0 */
PUBLIC inline
void
Context::inc_lock_cnt()
{
  _lock_cnt++;
}

/** Decrement lock count.
    @pre lock_cnt() > 0
 */
PUBLIC inline
void
Context::dec_lock_cnt()
{
  _lock_cnt--;
}

/** Lock count.
    @return lock count
 */
PUBLIC inline
int
Context::lock_cnt() const
{
  return _lock_cnt;
}

//@}

/**
 * Switch active timeslice of this Context.
 * @param next Sched_context to switch to
 */
PUBLIC
void
Context::switch_sched(Sched_context * const next)
{
  // Ensure CPU lock protection
  assert_kdb (cpu_lock.test());

  // If we're leaving the global timeslice, invalidate it
  // This causes schedule() to select a new timeslice via set_current_sched()
  if (sched() == current_sched())
    invalidate_sched();
#if 0
  // Ensure the new timeslice has a full quantum
  assert_kdb (next->left() == next->quantum());
#endif
  if (in_ready_list())
    ready_dequeue();

  set_sched(next);
  ready_enqueue();
}

/**
 * Select a different context for running and activate it.
 */
PUBLIC
void
Context::schedule()
{
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  CNT_SCHEDULE;

  // Ensure only the current thread calls schedule
  assert_kdb (this == current());
  assert_kdb (!_drq_active);

  unsigned current_cpu = ~0U;
  Sched_context::Ready_queue *rq = 0;

  // Enqueue current thread into ready-list to schedule correctly
  update_ready_list();

  // Select a thread for scheduling.
  Context *next_to_run;

  do
    {
      // I may've been migrated during the switch_exec_locked in the while
      // statement below. So cxheck out if I've to use a new ready queue.
      if (cpu() != current_cpu)
	{
	  current_cpu = cpu();
	  rq = &Sched_context::rq(current_cpu);
	  if (rq->schedule_in_progress)
	    return;
	  // Nested invocations of schedule() are bugs
	  assert_kdb (!rq->schedule_in_progress);
	}

      for (;;)
	{
	  next_to_run = rq->next_to_run()->context();

	  // Ensure ready-list sanity
	  assert_kdb (next_to_run);
      
	  if (EXPECT_TRUE (next_to_run->state() & Thread_ready_mask))
	    break;

	  next_to_run->ready_dequeue();
 
	  rq->schedule_in_progress = this;

	  cpu_lock.clear();
	  Proc::irq_chance();
	  cpu_lock.lock();

	  // check if we've been migrated meanwhile
	  if (EXPECT_FALSE(current_cpu != cpu()))
	    {
	      current_cpu = cpu();
	      rq = &Sched_context::rq(current_cpu);
	      if (rq->schedule_in_progress)
		return;
	    }
	  else
	    rq->schedule_in_progress = 0;
	}
    }
  while (EXPECT_FALSE(schedule_switch_to_locked (next_to_run)));
}

/**
 * Return if there is currently a schedule() in progress
 */
PUBLIC inline
Context *
Context::schedule_in_progress()
{
  return sched()->schedule_in_progress(cpu());
}


PROTECTED inline
void
Context::reset_schedule_in_progress()
{ sched()->reset_schedule_in_progress(cpu()); }

#if 0
/**
 * Return true if s can preempt the current scheduling context, false otherwise
 */
PUBLIC static inline NEEDS ["globals.h"]
bool
Context::can_preempt_current (Sched_context const *s)
{
  assert_kdb (current_cpu() == s->owner()->cpu());
  return current()->sched()->can_preempt_current(s);
}
#endif
/**
 * Return currently active global Sched_context.
 */
PUBLIC static inline
Sched_context *
Context::current_sched()
{
  return Sched_context::rq(current_cpu()).current_sched();
}

/**
 * Set currently active global Sched_context.
 */
PROTECTED
void
Context::set_current_sched(Sched_context *sched)
{
  assert_kdb (sched);
  // Save remainder of previous timeslice or refresh it, unless it had
  // been invalidated
  unsigned cpu = this->cpu();
  Sched_context::Ready_queue &rq = Sched_context::rq(cpu);

  Timeout * const tt = timeslice_timeout.cpu(cpu);
  Unsigned64 clock = Timer::system_clock();
  if (Sched_context *s = rq.current_sched())
    {
      Signed64 left = tt->get_timeout(clock);
      if (left > 0)
	s->set_left(left);
      else
	s->replenish();

      LOG_SCHED_SAVE(s);
    }

  // Program new end-of-timeslice timeout
  tt->reset();
  tt->set(clock + sched->left(), cpu);

  // Make this timeslice current
  rq.activate(sched);

  LOG_SCHED_LOAD(sched);
}

/**
 * Invalidate (expire) currently active global Sched_context.
 */
PROTECTED inline NEEDS["logdefs.h","timeout.h"]
void
Context::invalidate_sched()
{
  //LOG_SCHED_INVALIDATE;
  sched()->invalidate_sched(cpu());
}

/**
 * Return Context's Sched_context with id 'id'; return time slice 0 as default.
 * @return Sched_context with id 'id' or 0
 */
PUBLIC inline
Sched_context *
Context::sched_context(unsigned short const id = 0) const
{
  if (EXPECT_TRUE (!id))
    return const_cast<Sched_context*>(&_sched_context);
#if 0
  for (Sched_context *tmp = _sched_context.next();
      tmp != &_sched_context; tmp = tmp->next())
    if (tmp->id() == id)
      return tmp;
#endif
  return 0;
}

/**
 * Return Context's currently active Sched_context.
 * @return Active Sched_context
 */
PUBLIC inline
Sched_context *
Context::sched() const
{
  return _sched;
}

/**
 * Set Context's currently active Sched_context.
 * @param sched Sched_context to be activated
 */
PROTECTED inline
void
Context::set_sched (Sched_context * const sched)
{
  _sched = sched;
}

/**
 * Return Context's real-time period length.
 * @return Period length in usecs
 */
PUBLIC inline
Unsigned64
Context::period() const
{
  return _period;
}

/**
 * Set Context's real-time period length.
 * @param period New period length in usecs
 */
PROTECTED inline
void
Context::set_period (Unsigned64 const period)
{
  _period = period;
}

/**
 * Return Context's scheduling mode.
 * @return Scheduling mode
 */
PUBLIC inline
Context::Sched_mode
Context::mode() const
{
  return _mode;
}

/**
 * Set Context's scheduling mode.
 * @param mode New scheduling mode
 */
PUBLIC inline
void
Context::set_mode (Context::Sched_mode const mode)
{
  _mode = mode;
}

// queue operations

// XXX for now, synchronize with global kernel lock
//-

/**
 * Enqueue current() if ready to fix up ready-list invariant.
 */
PRIVATE inline NOEXPORT
void
Context::update_ready_list()
{
  assert_kdb (this == current());

  if (state() & Thread_ready_mask)
    ready_enqueue();
}

/**
 * Check if Context is in ready-list.
 * @return 1 if thread is in ready-list, 0 otherwise
 */
PUBLIC inline
Mword
Context::in_ready_list() const
{
  return sched()->in_ready_list();
}

/**
 * Enqueue context in ready-list.
 */
PUBLIC
void
Context::ready_enqueue()
{
  assert_kdb(current_cpu() == cpu());
  //Lock_guard <Cpu_lock> guard (&cpu_lock);

  // Don't enqueue threads that are not ready or have no own time
  if (EXPECT_FALSE (!(state() & Thread_ready_mask) || !sched()->left()))
    return;

  sched()->ready_enqueue(cpu());
}


/**
 * \brief Activate a newly created thread.
 *
 * This function sets a new thread onto the ready list and switches to
 * the thread if it can preempt the currently running thread.
 */
PUBLIC
bool
Context::activate()
{
  Lock_guard <Cpu_lock> guard (&cpu_lock);
  if (cpu() == current_cpu())
    {
      state_add_dirty(Thread_ready);
      if (sched()->deblock(cpu(), current()->sched(), true))
	{
	  current()->switch_to_locked(this);
	  return true;
	}
    }
  else
    remote_ready_enqueue();

  return false;
}

/**
 * Remove context from ready-list.
 */
PUBLIC inline NEEDS ["cpu_lock.h", "lock_guard.h", "std_macros.h"]
void
Context::ready_dequeue()
{
  assert_kdb(current_cpu() == cpu());
  sched()->ready_dequeue();
}

/** Helper.  Context that helps us by donating its time to us. It is
    set by switch_exec() if the calling thread says so.
    @return context that helps us and should be activated after freeing a lock.
*/
PUBLIC inline
Context *
Context::helper() const
{
  return _helper;
}

PUBLIC inline
void
Context::set_helper (enum Helping_mode const mode)
{
  switch (mode)
    {
    case Helping:
      _helper = current();
      break;
    case Not_Helping:
      _helper = this;
      break;
    case Ignore_Helping:
      // don't change _helper value
      break;
    }
}

/** Donatee.  Context that receives our time slices, for example
    because it has locked us.
    @return context that should be activated instead of us when we're
            switch_exec()'ed.
*/
PUBLIC inline
Context *
Context::donatee() const
{
  return _donatee;
}

PUBLIC inline
void
Context::set_donatee (Context * const donatee)
{
  _donatee = donatee;
}

PUBLIC inline
Mword *
Context::get_kernel_sp() const
{
  return _kernel_sp;
}

PUBLIC inline
void
Context::set_kernel_sp (Mword * const esp)
{
  _kernel_sp = esp;
}

PUBLIC inline
Fpu_state *
Context::fpu_state()
{
  return &_fpu_state;
}

/**
 * Add to consumed CPU time.
 * @param quantum Implementation-specific time quantum (TSC ticks or usecs)
 */
PUBLIC inline
void
Context::consume_time(Clock::Time quantum)
{
  _consumed_time += quantum;
}

/**
 * Update consumed CPU time during each context switch and when
 *        reading out the current thread's consumed CPU time.
 */
IMPLEMENT inline NEEDS ["cpu.h"]
void
Context::update_consumed_time()
{
  if (Config::fine_grained_cputime)
    consume_time (_clock.cpu(cpu()).delta());
}

IMPLEMENT inline NEEDS ["config.h", "cpu.h"]
Cpu_time
Context::consumed_time()
{
  if (Config::fine_grained_cputime)
    return _clock.cpu(cpu()).us(_consumed_time);

  return _consumed_time;
}

/**
 * Switch to scheduling context and execution context while not running under
 * CPU lock.
 */
PUBLIC inline NEEDS [<cassert>]
void
Context::switch_to (Context *t)
{
  // Call switch_to_locked if CPU lock is already held
  assert (!cpu_lock.test());

  // Grab the CPU lock
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  switch_to_locked (t);
}

/**
 * Switch scheduling context and execution context.
 * @param t Destination thread whose scheduling context and execution context
 *          should be activated.
 */
PRIVATE inline NEEDS ["kdb_ke.h"]
bool FIASCO_WARN_RESULT
Context::schedule_switch_to_locked(Context *t)
{
   // Must be called with CPU lock held
  assert_kdb (cpu_lock.test());
 
  // Switch to destination thread's scheduling context
  if (current_sched() != t->sched())
    set_current_sched(t->sched());

  // XXX: IPC dependency tracking belongs here.

  // Switch to destination thread's execution context, no helping involved
  if (t != this)
    return switch_exec_locked(t, Not_Helping);

  return handle_drq();
}

PUBLIC inline NEEDS [Context::schedule_switch_to_locked]
void
Context::switch_to_locked(Context *t)
{
  if (EXPECT_FALSE(schedule_switch_to_locked(t)))
    schedule();
}


/**
 * Switch execution context while not running under CPU lock.
 */
PUBLIC inline NEEDS ["kdb_ke.h"]
bool FIASCO_WARN_RESULT
Context::switch_exec (Context *t, enum Helping_mode mode)
{
  // Call switch_exec_locked if CPU lock is already held
  assert_kdb (!cpu_lock.test());

  // Grab the CPU lock
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  return switch_exec_locked (t, mode);
}

/**
 * Switch to a specific different execution context.
 *        If that context is currently locked, switch to its locker instead
 *        (except if current() is the locker)
 * @pre current() == this  &&  current() != t
 * @param t thread that shall be activated.
 * @param mode helping mode; we either help, don't help or leave the
 *             helping state unchanged
 */
PUBLIC
bool  FIASCO_WARN_RESULT //L4_IPC_CODE
Context::switch_exec_locked (Context *t, enum Helping_mode mode)
{
  // Must be called with CPU lock held
  assert_kdb (cpu_lock.test());
  if (t->cpu() != current_cpu()){ printf("%p => %p\n", this, t); kdb_ke("ass"); }  assert_kdb (t->cpu() == current_cpu());
  assert_kdb (current() != t);
  assert_kdb (current() == this);
  assert_kdb (timeslice_timeout.cpu(cpu())->is_set()); // Coma check

  // only for logging
  Context *t_orig = t;
  (void)t_orig;

  // Time-slice lending: if t is locked, switch to its locker
  // instead, this is transitive
  while (t->donatee() &&		// target thread locked
         t->donatee() != t)		// not by itself
    {
      // Special case for Thread::kill(): If the locker is
      // current(), switch to the locked thread to allow it to
      // release other locks.  Do this only when the target thread
      // actually owns locks.
      if (t->donatee() == current())
        {
          if (t->lock_cnt() > 0)
            break;

          return handle_drq();
        }

      t = t->donatee();
    }

  LOG_CONTEXT_SWITCH;
  CNT_CONTEXT_SWITCH;

  // Can only switch to ready threads!
  if (EXPECT_FALSE (!(t->state() & Thread_ready_mask)))
    {
      assert_kdb (state() & Thread_ready_mask);
      return false;
    }


  // Ensure kernel stack pointer is non-null if thread is ready
  assert_kdb (t->_kernel_sp);

  t->set_helper (mode);

  update_ready_list();
  assert_kdb (!(state() & Thread_ready_mask) || !sched()->left()
              || in_ready_list());

  switch_fpu (t);
  switch_cpu (t);

  return handle_drq();
}

PUBLIC inline NEEDS[Context::switch_exec_locked, Context::schedule]
void
Context::switch_exec_schedule_locked (Context *t, enum Helping_mode mode)
{
  if (EXPECT_FALSE(switch_exec_locked(t, mode)))
    schedule();
}

IMPLEMENT inline
Local_id
Context::local_id() const
{
  return _local_id;
}

IMPLEMENT inline
void
Context::local_id (Local_id id)
{
  _local_id = id;
}

IMPLEMENT inline
Utcb *
Context::utcb() const
{
  return _utcb;
}

IMPLEMENT inline
void
Context::utcb (Utcb *u)
{
  _utcb = u;
}


IMPLEMENT inline NEEDS["globals.h"]
Context *
Context::Context_member::context()
{ return context_of(this); }

IMPLEMENT inline NEEDS["lock_guard.h", "kdb_ke.h"]
void
Context::Drq_q::enq(Drq *rq)
{
  assert_kdb(cpu_lock.test());
  Lock_guard<Inner_lock> guard(q_lock());
  enqueue(rq);
}

PRIVATE inline
bool
Context::do_drq_reply(Drq *r, Drq_q::Drop_mode drop)
{
  state_change_dirty(~Thread_drq_wait, Thread_ready);
  // r->state = Drq::Reply_handled;
  if (drop == Drq_q::No_drop && r->reply)
    return r->reply(r, this, r->arg) & Drq::Need_resched;

  return false;
}

IMPLEMENT inline NEEDS[Context::do_drq_reply]
bool
Context::Drq_q::execute_request(Drq *r, Drop_mode drop, bool local)
{
  bool need_resched = false;
  Context *const self = context();
  // printf("CPU[%2u:%p]: context=%p: handle request for %p (func=%p, arg=%p)\n", current_cpu(), current(), context(), r->context(), r->func, r->arg);
  if (r->context() == self)
    {
      LOG_TRACE("DRQ handling", "drq", current(), __context_drq_log_fmt,
	  Drq_log *l = tbe->payload<Drq_log>();
	  l->type = "reply";
	  l->func = (void*)r->func;
	  l->reply = (void*)r->reply;
	  l->thread = r->context();
	  l->target_cpu = current_cpu();
	  l->wait = 0;
      );
      //LOG_MSG_3VAL(current(), "hrP", current_cpu() | (drop ? 0x100: 0), (Mword)r->context(), (Mword)r->func);
      return self->do_drq_reply(r, drop);
    }
  else
    {
      LOG_TRACE("DRQ handling", "drq", current(), __context_drq_log_fmt,
	  Drq_log *l = tbe->payload<Drq_log>();
	  l->type = "request";
	  l->func = (void*)r->func;
	  l->reply = (void*)r->reply;
	  l->thread = r->context();
	  l->target_cpu = current_cpu();
	  l->wait = 0;
      );
      // r->state = Drq::Idle;
      unsigned answer = 0;
      //LOG_MSG_3VAL(current(), "hrq", current_cpu() | (drop ? 0x100: 0), (Mword)r->context(), (Mword)r->func);
      if (EXPECT_TRUE(drop == No_drop && r->func))
	answer = r->func(r, self, r->arg);
      else if (EXPECT_FALSE(drop == Drop))
	// flag DRQ abort for requester
	r->arg = (void*)-1;
       // LOG_MSG_3VAL(current(), "hrq-", answer, current()->state() /*(Mword)r->context()*/, (Mword)r->func);
      need_resched |= answer & Drq::Need_resched;
      //r->state = Drq::Handled;

      // enqueue answer
      if (!(answer & Drq::No_answer))
	{
	  if (local)
	    return r->context()->do_drq_reply(r, drop) || need_resched;
	  else
	    need_resched |= r->context()->enqueue_drq(r, Drq::Target_ctxt);
	}
    }
  return need_resched;
}

IMPLEMENT inline NEEDS["mem.h", "lock_guard.h"]
bool
Context::Drq_q::handle_requests(Drop_mode drop)
{
  // printf("CPU[%2u:%p]: > Context::Drq_q::handle_requests() context=%p\n", current_cpu(), current(), context());
  bool need_resched = false;
  while (1)
    {
      Queue_item *qi;
	{
	  Lock_guard<Inner_lock> guard(q_lock());
	  qi = first();
	  if (!qi)
	    return need_resched;

	  check_kdb (dequeue(qi, Queue_item::Ok));
	}

      Drq *r = static_cast<Drq*>(qi);
      // printf("CPU[%2u:%p]: context=%p: handle request for %p (func=%p, arg=%p)\n", current_cpu(), current(), context(), r->context(), r->func, r->arg);
      need_resched |= execute_request(r, drop, false);
    }
}
/**
 * \biref Forced dequeue from lock wait queue, or DRQ queue.
 */
PRIVATE
void
Context::force_dequeue()
{
  Queue_item *const qi = queue_item();

  if (qi->queued())
    {
      // we're waiting for a lock or have a DRQ pending
      Queue *const q = qi->queue();
	{
	  Lock_guard<Queue::Inner_lock> guard(q->q_lock());
	  // check again, with the queue lock held.
	  // NOTE: we may be already removed from the queue on another CPU
	  if (qi->queued() && qi->queue())
	    {
	      // we must never be taken from one queue to another on a
	      // different CPU
	      assert_kdb(q == qi->queue());
	      // pull myself out of the queue, mark reason as invalidation
	      q->dequeue(qi, Queue_item::Invalid);
	    }
	}
    }
}

/**
 * \brief Dequeue from lock and DRQ queues, abort pending DRQs
 */
PROTECTED
void
Context::shutdown_queues()
{
  force_dequeue();
  shutdown_drqs();
}


/**
 * \brief Check for pending DRQs.
 * \return true if there are DRQs pending, false if not.
 */
PUBLIC inline
bool
Context::drq_pending() const
{ return _drq_q.first(); }

PUBLIC inline
void
Context::try_finish_migration()
{
  if (EXPECT_FALSE(_migration_rq.in_progress))
    {
      _migration_rq.in_progress = false;
      finish_migration();
    }
}


/**
 * \brief Handle all pending DRQs.
 * \pre cpu_lock.test() (The CPU lock must be held).
 * \pre current() == this (only the currently running context is allowed to
 *      call this function).
 * \return true if re-scheduling is needed (ready queue has changed),
 *         false if not.
 */
PUBLIC //inline
bool
Context::handle_drq()
{
  //LOG_MSG_3VAL(this, ">drq", _drq_active, 0, cpu_lock.test());
  assert_kdb (current_cpu() == this->cpu());
  assert_kdb (cpu_lock.test());

  try_finish_migration();

  if (_drq_active)
    return false;

  if (drq_pending())
    _drq_active = 1;
  else
    return false;

  Mem::barrier();
  bool ret = false;
  while (true)
    {
      ret |= _drq_q.handle_requests();

      Lock_guard<Drq_q::Inner_lock> guard(_drq_q.q_lock());
      if (EXPECT_TRUE(!drq_pending()))
	{
	  state_del_dirty(Thread_drq_ready);
	  _drq_active = 0;
	  break;
	}
    }
  
  //LOG_MSG_3VAL(this, "xdrq", state(), ret, cpu_lock.test());

  /*
   * When the context is marked as dead (Thread_dead) then we must not execute
   * any usual context code, however DRQ handlers may run.
   */
  if (state() & Thread_dead)
    {
      // so disable the context after handling all DRQs and flag a reschedule.
      state_del_dirty(Thread_ready_mask);
      return true;
    }

  return ret || !(state() & Thread_ready_mask);
}


/**
 * \brief Get the queue item of the context.
 * \pre The context must currently not be in any queue.
 * \return The queue item of the context.
 *
 * The queue item can be used to enqueue the context to a Queue.
 * a context must be in at most one queue at a time.
 * To figure out the context corresponding to a queue item
 * context_of() can be used.
 */
PUBLIC inline NEEDS["kdb_ke.h"]
Queue_item *
Context::queue_item()
{
  return &_drq;
}

/**
 * \brief DRQ handler for state_change.
 *
 * This function basically wraps Context::state_change().
 */
PRIVATE static
unsigned
Context::handle_drq_state_change(Drq * /*src*/, Context *self, void * _rq)
{
  State_request *rq = reinterpret_cast<State_request*>(_rq);
  self->state_change_dirty(rq->del, rq->add);
  //LOG_MSG_3VAL(c, "dsta", c->state(), (Mword)src, (Mword)_rq);
  return false;
}


/**
 * \brief Queue a DRQ for changing the contexts state.
 * \param mask bit mask for the state (state &= mask).
 * \param add bits to add to the state (state |= add).
 * \note This function is a preemption point.
 *
 * This function must be used to change the state of contexts that are
 * potentially running on a different CPU.
 */
PUBLIC inline NEEDS[Context::drq]
void
Context::drq_state_change(Mword mask, Mword add)
{
  State_request rq;
  rq.del = mask;
  rq.add = add;
  drq(handle_drq_state_change, &rq);
}


/**
 * \brief Initiate a DRQ for the context.
 * \pre \a src must be the currently running context.
 * \param src the source of the DRQ (the context who initiates the DRQ).
 * \param func the DRQ handler.
 * \param arg the argument for the DRQ handler.
 * \param reply the reply handler (called in the context of \a src immediately
 *        after receiving a successful reply).
 *
 * DRQs are requests than any context can queue to any other context. DRQs are
 * the basic mechanism to initiate actions on remote CPUs in an MP system,
 * however, are also allowed locally.
 * DRQ handlers of pending DRQs are executed by Context::handle_drq() in the
 * context of the target context. Context::handle_drq() is basically called
 * after switching to a context in Context::switch_exec_locked().
 *
 * This function enqueues a DRQ and blocks the current context for a reply DRQ.
 */
PUBLIC inline NEEDS[Context::enqueue_drq]
void
Context::drq(Drq *drq, Drq::Request_func *func, void *arg,
             Drq::Request_func *reply = 0,
             Drq::Exec_mode exec = Drq::Target_ctxt,
             Drq::Wait_mode wait = Drq::Wait)
{
  // printf("CPU[%2u:%p]: > Context::drq(this=%p, src=%p, func=%p, arg=%p)\n", current_cpu(), current(), this, src, func,arg);
  Context *cur = current();
  LOG_TRACE("DRQ Stuff", "drq", cur, __context_drq_log_fmt,
      Drq_log *l = tbe->payload<Drq_log>();
      l->type = "send";
      l->func = (void*)func;
      l->reply = (void*)reply;
      l->thread = this;
      l->target_cpu = cpu();
      l->wait = wait;
  );
  //assert_kdb (current() == src);
  assert_kdb (!(wait == Drq::Wait && (cur->state() & Thread_drq_ready)) || cur->cpu() == cpu());
  assert_kdb (!((wait == Drq::Wait || drq == &_drq) && cur->state() & Thread_drq_wait));
  assert_kdb (!drq->queued());

  drq->func  = func;
  drq->reply = reply;
  drq->arg   = arg;
  cur->state_add(wait == Drq::Wait ? Thread_drq_wait : 0);


  enqueue_drq(drq, exec);

  //LOG_MSG_3VAL(src, "<drq", src->state(), Mword(this), 0);
  while (wait == Drq::Wait && cur->state() & Thread_drq_wait)
    {
      cur->state_del(Thread_ready_mask);
      cur->schedule();
    }

  LOG_TRACE("DRQ Stuff", "drq", cur, __context_drq_log_fmt,
      Drq_log *l = tbe->payload<Drq_log>();
      l->type = "done";
      l->func = (void*)func;
      l->reply = (void*)reply;
      l->thread = this;
      l->target_cpu = cpu();
  );
  //LOG_MSG_3VAL(src, "drq>", src->state(), Mword(this), 0);
}

PROTECTED inline
void
Context::kernel_context_drq(Drq::Request_func *func, void *arg,
                            Drq::Request_func *reply = 0)
{
  char align_buffer[2*sizeof(Drq)];
  Drq *mdrq = (Drq*)((Address(align_buffer) + __alignof__(Drq) - 1) & ~(__alignof__(Drq)-1));

  mdrq->func  = func;
  mdrq->arg   = arg;
  mdrq->reply = reply;
  Context *kc = kernel_context(current_cpu());
  kc->_drq_q.enq(mdrq);
  bool resched = schedule_switch_to_locked(kc);
  (void)resched;
}

PUBLIC inline NEEDS[Context::drq]
void
Context::drq(Drq::Request_func *func, void *arg,
             Drq::Request_func *reply = 0,
             Drq::Exec_mode exec = Drq::Target_ctxt,
             Drq::Wait_mode wait = Drq::Wait)
{ return drq(&current()->_drq, func, arg, reply, exec, wait); }

PRIVATE static
bool
Context::rcu_unblock(Rcu_item *i)
{
  assert_kdb(cpu_lock.test());
  Context *const c = static_cast<Context*>(i);
  c->state_change_dirty(~Thread_waiting, Thread_ready);
  c->sched()->deblock(c->cpu());
  return true;
}

PUBLIC inline
void
Context::recover_jmp_buf(jmp_buf *b)
{ _recover_jmpbuf = b; }

//----------------------------------------------------------------------------
IMPLEMENTATION [!mp]:

#include "warn.h"
#include "kdb_ke.h"

PUBLIC inline
unsigned
Context::cpu(bool = false) const
{ return 0; }


PROTECTED
void
Context::remote_ready_enqueue()
{
  WARN("Context::remote_ready_enqueue(): in UP system !\n");
  kdb_ke("Fiasco BUG");
}

PUBLIC
bool
Context::enqueue_drq(Drq *rq, Drq::Exec_mode)
{
  bool sched = _drq_q.execute_request(rq, Drq_q::No_drop, true);
  if (!in_ready_list() && (state() & Thread_ready_mask))
    {
      ready_enqueue();
      return true;
    }

  return sched;
}


PRIVATE inline NOEXPORT
void
Context::shutdown_drqs()
{ _drq_q.handle_requests(Drq_q::Drop); }


PUBLIC inline
void
Context::rcu_wait()
{
  // The UP case does not need to block for the next grace period, because
  // the CPU is always in a quiescent state when the interrupts where enabled
}

PUBLIC static inline
void
Context::xcpu_tlb_flush()
{}


//----------------------------------------------------------------------------
INTERFACE [mp]:

#include "queue.h"
#include "queue_item.h"

EXTENSION class Context
{
protected:

  class Pending_rqq : public Queue
  {
  public:
    static void enq(Context *c);
    bool handle_requests(Context **);
  };

  class Pending_rq : public Queue_item, public Context_member
  {
  };

  Pending_rq _pending_rq;

protected:
  static Per_cpu<Pending_rqq> _pending_rqq;
  static Per_cpu<Drq_q> _glbl_drq_q;

};



//----------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include "globals.h"
#include "ipi.h"
#include "kdb_ke.h"
#include "lock_guard.h"
#include "mem.h"

Per_cpu<Context::Pending_rqq> DEFINE_PER_CPU Context::_pending_rqq;
Per_cpu<Context::Drq_q> DEFINE_PER_CPU Context::_glbl_drq_q;


/**
 * \brief Enqueue the given \a c into its CPUs queue.
 * \param c the context to enqueue for DRQ handling.
 */
IMPLEMENT inline NEEDS["globals.h", "lock_guard.h", "kdb_ke.h"]
void
Context::Pending_rqq::enq(Context *c)
{
  // FIXME: is it safe to do the check without a locked queue, or may
  //        we loose DRQs then?

  //if (!c->_pending_rq.queued())
    {
      Queue &q = Context::_pending_rqq.cpu(c->cpu());
      Lock_guard<Inner_lock> guard(q.q_lock());
      if (c->_pending_rq.queued())
	return;
      q.enqueue(&c->_pending_rq);
    }
}


/**
 * \brief Wakeup all contexts with pending DRQs.
 *
 * This function wakes up all context from the pending queue.
 */
IMPLEMENT
bool
Context::Pending_rqq::handle_requests(Context **mq)
{
  //LOG_MSG_3VAL(current(), "phq", current_cpu(), 0, 0);
  // printf("CPU[%2u:%p]: Context::Pending_rqq::handle_requests() this=%p\n", current_cpu(), current(), this);
  bool resched = false;
  Context *curr = current();
  while (1)
    {
      Queue_item *qi;
	{
	  Lock_guard<Inner_lock> guard(q_lock());
	  qi = first();
	  if (!qi)
	    return resched;
	  check_kdb (dequeue(qi, Queue_item::Ok));
	}
      Context *c = static_cast<Context::Pending_rq *>(qi)->context();
      //LOG_MSG_3VAL(c, "pick", c->state(), c->cpu(), current_cpu());
      // Drop migrated threads
      assert_kdb (EXPECT_FALSE(c->cpu() == current_cpu()));

      if (EXPECT_TRUE(c->drq_pending()))
	c->state_add(Thread_drq_ready);

      if (EXPECT_FALSE(c->_migration_rq.pending))
	{
	  if (c != curr)
	    {
	      c->initiate_migration();
	      continue;
	    }
	  else
	    {
	      *mq = c;
	      resched = true;
	    }
	}
      else
	c->try_finish_migration();

      if (EXPECT_TRUE((c->state() & Thread_ready_mask)))
	{
	  //printf("CPU[%2u:%p]:   Context::Pending_rqq::handle_requests() dequeded %p(%u)\n", current_cpu(), current(), c, qi->queued());
	  resched |= c->sched()->deblock(current_cpu(), current()->sched(), false);
	}
    }
}

PUBLIC
void
Context::global_drq(unsigned cpu, Drq::Request_func *func, void *arg,
                    Drq::Request_func *reply = 0, bool wait = true)
{
  assert_kdb (this == current());

  _drq.func  = func;
  _drq.reply = reply;
  _drq.arg   = arg;

  state_add(wait ? Thread_drq_wait : 0);

  _glbl_drq_q.cpu(cpu).enq(&_drq);

  Ipi::cpu(cpu).send(Ipi::Global_request);

  //LOG_MSG_3VAL(src, "<drq", src->state(), Mword(this), 0);
  while (wait && (state() & Thread_drq_wait))
    {
      state_del(Thread_ready_mask);
      schedule();
    }
}


PUBLIC
static bool
Context::handle_global_requests()
{
  return _glbl_drq_q.cpu(current_cpu()).handle_requests();
}

PUBLIC
bool
Context::enqueue_drq(Drq *rq, Drq::Exec_mode /*exec*/)
{
  assert_kdb (cpu_lock.test());
  // printf("CPU[%2u:%p]: Context::enqueue_request(this=%p, src=%p, func=%p, arg=%p)\n", current_cpu(), current(), this, src, func,arg);

  if (cpu() != current_cpu())
    {
      bool ipi = true;
      _drq_q.enq(rq);

      // ready cpu again we may've been migrated meanwhile
      unsigned cpu = this->cpu();

	{
	  Queue &q = Context::_pending_rqq.cpu(cpu);
	  Lock_guard<Pending_rqq::Inner_lock> g(q.q_lock());


	  // migrated between getting the lock and reading the CPU, so the
	  // new CPU is responsible for executing our request
	  if (this->cpu() != cpu)
	    return false;

	  if (q.first())
	    ipi = false;

	  if (!_pending_rq.queued())
	    q.enqueue(&_pending_rq);
	}

      if (ipi)
	{
	  //LOG_MSG_3VAL(this, "sipi", current_cpu(), cpu(), (Mword)current());
	  Ipi::cpu(cpu).send(Ipi::Request);
	}
    }
  else
    { // LOG_MSG_3VAL(this, "adrq", state(), (Mword)current(), (Mword)rq);

      bool sched = _drq_q.execute_request(rq, Drq_q::No_drop, true);
      if (!in_ready_list() && (state() & Thread_ready_mask))
	{
	  ready_enqueue();
	  return true;
	}

      return sched;
    }
  return false;
}


PRIVATE inline NOEXPORT
void
Context::shutdown_drqs()
{
  if (_pending_rq.queued())
    {
      Lock_guard<Pending_rqq::Inner_lock> guard(_pending_rq.queue()->q_lock());
      if (_pending_rq.queued())
	_pending_rq.queue()->dequeue(&_pending_rq, Queue_item::Ok);
    }

  _drq_q.handle_requests(Drq_q::Drop);
}


PUBLIC inline
unsigned
Context::cpu(bool running = false) const
{
  (void)running;
  return _cpu;
}


/**
 * Remote helper for doing remote CPU ready enqueue.
 *
 * See remote_ready_enqueue().
 */
PRIVATE static
unsigned
Context::handle_remote_ready_enqueue(Drq *, Context *self, void *)
{
  self->state_add_dirty(Thread_ready);
  return 0;
}


PROTECTED inline NEEDS[Context::handle_remote_ready_enqueue]
void
Context::remote_ready_enqueue()
{ drq(&handle_remote_ready_enqueue, 0); }



/**
 * Block and wait for the next grace period.
 */
PUBLIC inline NEEDS["cpu_lock.h", "lock_guard.h"]
void
Context::rcu_wait()
{
  Lock_guard<Cpu_lock> gurad(&cpu_lock);
  state_change_dirty(~Thread_ready, Thread_waiting);
  Rcu::call(this, &rcu_unblock);
  schedule();
}



PRIVATE static
unsigned
Context::handle_remote_tlb_flush(Drq *, Context *, void *)
{
  // printf("RCV XCPU_FLUSH (%d)\n", current_cpu());
  if (!current()->space())
    return 0;

  Mem_space *ms = current()->mem_space();
  bool need_flush = ms->need_tlb_flush();
  if (need_flush)
    ms->tlb_flush(true);

  return 0;
}


PUBLIC static
void
Context::xcpu_tlb_flush()
{
  //printf("XCPU_ TLB FLUSH\n");
  Lock_guard<Cpu_lock> g(&cpu_lock);
  unsigned ccpu = current_cpu();
  for (unsigned i = 0; i < Config::Max_num_cpus; ++i)
    {
      if (ccpu != i && Cpu::online(i))
	current()->global_drq(i, Context::handle_remote_tlb_flush, 0);
    }
}



//----------------------------------------------------------------------------
IMPLEMENTATION [fpu && !ux]:

#include "fpu.h"

/**
 * When switching away from the FPU owner, disable the FPU to cause
 * the next FPU access to trap.
 * When switching back to the FPU owner, enable the FPU so we don't
 * get an FPU trap on FPU access.
 */
IMPLEMENT inline NEEDS ["fpu.h"]
void
Context::switch_fpu(Context *t)
{
  if (Fpu::is_owner(cpu(), this))
    Fpu::disable();
  else if (Fpu::is_owner(cpu(), t) && !(t->state() & Thread_vcpu_fpu_disabled))
    Fpu::enable();
}

//----------------------------------------------------------------------------
IMPLEMENTATION [!fpu]:

IMPLEMENT inline
void
Context::switch_fpu(Context *)
{}

//----------------------------------------------------------------------------
IMPLEMENTATION [ux]:

PUBLIC static
void
Context::boost_idle_prio(unsigned _cpu)
{
  // Boost the prio of the idle thread so that it can actually get some
  // CPU and take down the system.
  kernel_context(_cpu)->ready_dequeue();
  kernel_context(_cpu)->sched()->set_prio(255);
  kernel_context(_cpu)->ready_enqueue();
}

// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "kobject_dbg.h"

IMPLEMENT
unsigned
Context::drq_log_fmt(Tb_entry *e, int maxlen, char *buf)
{
  Drq_log *l = e->payload<Drq_log>();
  return snprintf(buf, maxlen, "drq %s(%s) to ctxt=%lx/%p (func=%p, reply=%p) cpu=%u",
      l->type, l->wait ? "wait" : "no-wait", Kobject_dbg::pointer_to_id(l->thread),
      l->thread, l->func, l->reply, l->target_cpu);
}

