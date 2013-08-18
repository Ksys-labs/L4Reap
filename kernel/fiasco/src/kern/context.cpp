INTERFACE:

#include <csetjmp>             // typedef jmp_buf
#include "types.h"
#include "clock.h"
#include "config.h"
#include "continuation.h"
#include "fpu_state.h"
#include "globals.h"
#include "l4_types.h"
#include "member_offs.h"
#include "per_cpu_data.h"
#include "processor.h"
#include "queue.h"
#include "queue_item.h"
#include "rcupdate.h"
#include "sched_context.h"
#include "space.h"
#include "spin_lock.h"
#include "timeout.h"
#include <fiasco_defs.h>
#include <cxx/function>

class Entry_frame;
class Thread_lock;
class Context;
class Kobject_iface;

class Context_ptr
{
public:
  explicit Context_ptr(Cap_index id) : _t(id) {}
  Context_ptr() {}
  Context_ptr(Context_ptr const &o) : _t(o._t) {}
  Context_ptr const &operator = (Context_ptr const &o)
  { _t = o._t; return *this; }

  Kobject_iface *ptr(Space *, L4_fpage::Rights *) const;

  bool is_kernel() const { return false; }
  bool is_valid() const { return _t != Cap_index(~0UL); }

  // only for debugging use
  Cap_index raw() const { return _t; }

private:
  Cap_index _t;

};

template< typename T >
class Context_ptr_base : public Context_ptr
{
public:
  enum Invalid_type { Invalid };
  enum Null_type { Null };
  explicit Context_ptr_base(Invalid_type) : Context_ptr(Cap_index(~0UL)) {}
  explicit Context_ptr_base(Null_type) : Context_ptr(Cap_index(0)) {}
  explicit Context_ptr_base(Cap_index id) : Context_ptr(id) {}
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

class Context_space_ref
{
public:
  typedef Spin_lock_coloc<Space *> Space_n_lock;

private:
  Space_n_lock _s;
  Address _v;

public:
  Space *space() const { return _s.get_unused(); }
  Space_n_lock *lock() { return &_s; }
  Address user_mode() const { return _v & 1; }
  Space *vcpu_user() const { return reinterpret_cast<Space*>(_v & ~3); }
  Space *vcpu_aware() const { return user_mode() ? vcpu_user() : space(); }

  void space(Space *s) { _s.set_unused(s); }
  void vcpu_user(Space *s) { _v = (Address)s; }
  void user_mode(bool enable)
  {
    if (enable)
      _v |= (Address)1;
    else
      _v &= (Address)(~1);
  }
};

/** An execution context.  A context is a runnable, schedulable activity.
    It carries along some state used by other subsystems: A lock count,
    and stack-element forward/next pointers.
 */
class Context :
  public Context_base,
  protected Rcu_item
{
  MEMBER_OFFSET();
  friend class Jdb_thread_list;
  friend class Context_ptr;
  friend class Jdb_utcb;

protected:
  virtual void finish_migration() = 0;
  virtual bool initiate_migration() = 0;

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
  private:
    Context_member(Context_member const &);

  public:
    Context_member() {}
    /**
     * \brief Get the aggregating Context object.
     */
    Context *context() const;
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
    struct Result
    {
      unsigned char v;
      CXX_BITFIELD_MEMBER(0, 0, need_resched, v);
      CXX_BITFIELD_MEMBER(1, 1, no_answer, v);
    };

    static Result done()
    { Result r; r.v = 0; return r; }

    static Result no_answer()
    { Result r; r.v = Result::no_answer_bfm_t::Mask; return r; }

    static Result need_resched()
    { Result r; r.v = Result::need_resched_bfm_t::Mask; return r; }

    static Result no_answer_resched()
    {
      Result r;
      r.v = Result::no_answer_bfm_t::Mask | Result::need_resched_bfm_t::Mask;
      return r;
    }

    typedef Result (Request_func)(Drq *, Context *target, void *);
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
    bool dequeue(Drq *drq, Queue_item::Status reason);
    bool handle_requests(Drop_mode drop = No_drop);
    bool execute_request(Drq *r, Drop_mode drop, bool local);
  };

  class Cpu_call_queue;

  class Cpu_call : private Queue_item
  {
    friend class Context::Cpu_call_queue;

  private:
    cxx::functor<bool (Cpu_number cpu)> _func;
    Mword _wait;

  public:
    template< typename Functor >
    Cpu_call(Functor &&f)
    : _func(f), _wait(false) {}

    Cpu_call(cxx::functor<bool (Cpu_number)> &&f)
    : _func(f), _wait(false) {}

    Cpu_call() : _func(), _wait(false) {}

    void set(cxx::functor<bool (Cpu_number)> &f)
    { _func = f; }

    void set_queued()
    { _wait = true; }

    void done()
    {
      Mem::mp_mb();
      write_now(&_wait, (Mword)false);
    }

    bool run(Cpu_number cpu, bool done = true)
    {
      bool res = _func(cpu);
      if (done)
        this->done();
      return res;
    }

    bool is_done() const
    { return !access_once(&_wait); }

    bool remote_call(Cpu_number cpu);

  };

  template<unsigned MAX>
  class Cpu_calls
  {
    Cpu_calls(Cpu_calls const &) = delete;
    Cpu_calls &operator = (Cpu_calls const &) = delete;

  public:
    enum { Max = Config::Max_num_cpus < MAX ?  Config::Max_num_cpus : MAX };
    Cpu_calls() : _used(0) {}

    Cpu_call *next()
    {
      if (_used < Max)
        return &_cs[_used++];
      return 0;
    }

    Cpu_call *find_done()
    {
      for (unsigned i = 0; i < _used; ++i)
        if (_cs[i].is_done())
          return &_cs[i];

      return 0;
    }

    void wait_all()
    {
      for (unsigned i = 0; i < _used; ++i)
        while (!_cs[i].is_done())
          Proc::pause();
    }

    unsigned char used() const { return _used; }

  private:
    Cpu_call _cs[Max];
    unsigned char _used;
  };

  class Cpu_call_queue : public Queue
  {
  public:
    void enq(Cpu_call *rq);
    bool dequeue(Cpu_call *drq, Queue_item::Status reason);
    bool handle_requests();
    bool execute_request(Cpu_call *r);
  };

  struct Migration
  {
    Cpu_number cpu;
    L4_sched_param const *sp;
    bool in_progress;

    Migration() : in_progress(false) {}
  };

  template<typename T>
  class Ku_mem_ptr : public Context_member
  {
    MEMBER_OFFSET();

  private:
    typename User<T>::Ptr _u;
    T *_k;

  public:
    Ku_mem_ptr() : _u(0), _k(0) {}
    Ku_mem_ptr(typename User<T>::Ptr const &u, T *k) : _u(u), _k(k) {}

    void set(typename User<T>::Ptr const &u, T *k)
    { _u = u; _k = k; }

    T *access(bool is_current = false) const
    {
      // assert_kdb (!is_current || current() == context());
      if (is_current
          && (int)Config::Access_user_mem == Config::Access_user_mem_direct)
        return _u.get();

      Cpu_number const cpu = current_cpu();
      if ((int)Config::Access_user_mem == Config::Must_access_user_mem_direct
          && cpu == context()->home_cpu()
          && Mem_space::current_mem_space(cpu) == context()->space())
        return _u.get();
      return _k;
    }

    typename User<T>::Ptr usr() const { return _u; }
    T* kern() const { return _k; }
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
  enum Helping_mode
  {
    Helping,
    Not_Helping,
    Ignore_Helping
  };

  /**
   * Return consumed CPU time.
   * @return Consumed CPU time in usecs
   */
  Cpu_time consumed_time();

  virtual bool kill() = 0;

  void spill_user_state();
  void fill_user_state();

  Space * FIASCO_PURE space() const { return _space.space(); }
  Mem_space * FIASCO_PURE mem_space() const { return static_cast<Mem_space*>(space()); }

  Cpu_number home_cpu() const { return _home_cpu; }
  void set_home_cpu(Cpu_number cpu) { _home_cpu = cpu; }

protected:
  Cpu_number _home_cpu;

  /**
   * Update consumed CPU time during each context switch and when
   *        reading out the current thread's consumed CPU time.
   */
  void update_consumed_time();

  Mword *_kernel_sp;
  void *_utcb_handler;
  Ku_mem_ptr<Utcb> _utcb;

private:
  friend class Jdb;
  friend class Jdb_tcb;

  /// low level page table switching
  void switchin_context(Context *) asm ("switchin_context_label") FIASCO_FASTCALL;

  /// low level fpu switching
  void switch_fpu(Context *t);

  /// low level cpu switching
  void switch_cpu(Context *t);

protected:
  Context_space_ref _space;

private:
  Context *_donatee;
  Context *_helper;

  // Lock state
  // how many locks does this thread hold on other threads
  // incremented in Thread::lock, decremented in Thread::clear
  // Thread::kill needs to know
  int _lock_cnt;

  // The scheduling parameters.  We would only need to keep an
  // anonymous reference to them as we do not need them ourselves, but
  // we aggregate them for performance reasons.
  Sched_context _sched_context;
  Sched_context *_sched;
  Unsigned64 _period;
  Sched_mode _mode;

  // Pointer to floating point register state
  Fpu_state _fpu_state;
  // Implementation-specific consumed CPU time (TSC ticks or usecs)
  Clock::Time _consumed_time;

  Drq _drq;
  Drq_q _drq_q;

protected:
  // for trigger_exception
  Continuation _exc_cont;

  jmp_buf *_recover_jmpbuf;     // setjmp buffer for page-fault recovery

  Migration *_migration;
  bool _need_to_finish_migration;

public:
  void arch_load_vcpu_kern_state(Vcpu_state *vcpu, bool do_load);

protected:
  void arch_load_vcpu_user_state(Vcpu_state *vcpu, bool do_load);
  void arch_update_vcpu_state(Vcpu_state *vcpu);

  // XXX Timeout for both, sender and receiver! In normal case we would have
  // to define own timeouts in Receiver and Sender but because only one
  // timeout can be set at one time we use the same timeout. The timeout
  // has to be defined here because Dirq::hit has to be able to reset the
  // timeout (Irq::_irq_thread is of type Receiver).
  Timeout *_timeout;

private:
  static Per_cpu<Clock> _clock;
  static Per_cpu<Context *> _kernel_ctxt;
};


INTERFACE [debug]:

#include "tb_entry.h"

EXTENSION class Context
{
public:
  struct Drq_log : public Tb_entry
  {
    void *func;
    void *reply;
    Context *thread;
    Drq const *rq;
    Cpu_number target_cpu;
    enum class Type
    {
      Send, Do_send, Do_request, Send_reply, Do_reply, Done
    } type;
    bool wait;
    void print(String_buffer *buf) const;
    Group_order has_partner() const
    {
      switch (type)
        {
        case Type::Send: return Group_order::first();
        case Type::Do_send: return Group_order(1);
        case Type::Do_request: return Group_order(2);
        case Type::Send_reply: return Group_order(3);
        case Type::Do_reply: return Group_order(4);
        case Type::Done: return Group_order::last();
        }
      return Group_order::none();
    }

    Group_order is_partner(Drq_log const *o) const
    {
      if (rq != o->rq || func != o->func || reply != o->reply)
        return Group_order::none();

      return o->has_partner();
    }
  };


  struct Vcpu_log : public Tb_entry
  {
    Mword state;
    Mword ip;
    Mword sp;
    Mword space;
    Mword err;
    unsigned char type;
    unsigned char trap;
    void print(String_buffer *buf) const;
  };
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

DEFINE_PER_CPU Per_cpu<Clock> Context::_clock(Per_cpu_data::Cpu_num);
DEFINE_PER_CPU Per_cpu<Context *> Context::_kernel_ctxt;

IMPLEMENT inline NEEDS["kdb_ke.h"]
Kobject_iface * __attribute__((nonnull(2, 3)))
Context_ptr::ptr(Space *s, L4_fpage::Rights *rights) const
{
  assert_kdb (cpu_lock.test());

  return static_cast<Obj_space*>(s)->lookup_local(_t, rights);
}



#include <cstdio>

/** Initialize a context.  After setup, a switch_exec to this context results
    in a return to user code using the return registers at regs().  The
    return registers are not initialized however; neither is the space_context
    to be used in thread switching (use set_space_context() for that).
    @pre (_kernel_sp == 0)  &&  (* (stack end) == 0)
 */
PUBLIC inline NEEDS ["atomic.h", "entry_frame.h", <cstdio>]
Context::Context()
: _kernel_sp(reinterpret_cast<Mword*>(regs())),
  /* TCBs are zero initialized _utcb_handler(0), */
  _helper(this),
  _sched_context(),
  _sched(&_sched_context)
  /* TCBs should be initialized to zero!
   * _mode(Sched_mode(0)),
   * _migration(0),
   * _need_to_finish_migration(false)
  */

{
  // NOTE: We do not have to synchronize the initialization of
  // _space_context because it is constant for all concurrent
  // invocations of this constructor.  When two threads concurrently
  // try to create a new task, they already synchronize in
  // sys_task_new() and avoid calling us twice with different
  // space_context arguments.

  _home_cpu = Cpu::invalid();
}

PUBLIC inline
void
Context::reset_kernel_sp()
{
  _kernel_sp = reinterpret_cast<Mword*>(regs());
}

PUBLIC inline
void
Context::spill_fpu_if_owner()
{
  // spill FPU state into memory before migration
  if (state() & Thread_fpu_owner)
    {
      Fpu &f = Fpu::fpu.current();
      if (current() != this)
        f.enable();

      spill_fpu();
      f.set_owner(0);
      f.disable();
    }
}


PUBLIC inline
void
Context::release_fpu_if_owner()
{
  // If this context owned the FPU, noone owns it now
  Fpu &f = Fpu::fpu.current();
  if (f.is_owner(this))
    {
      f.set_owner(0);
      f.disable();
    }
}

/** Destroy context.
 */
PUBLIC virtual
Context::~Context()
{}

PUBLIC inline
bool
Context::check_for_current_cpu() const
{
  bool r = home_cpu() == current_cpu() || !Cpu::online(home_cpu());
  if (0 && EXPECT_FALSE(!r)) // debug output disabled
    printf("FAIL: cpu=%u (current=%u)\n",
           cxx::int_value<Cpu_number>(home_cpu()),
           cxx::int_value<Cpu_number>(current_cpu()));
  return r;
}


PUBLIC inline
Mword
Context::state(bool check = false) const
{
  (void)check;
  assert_kdb(!check || check_for_current_cpu());
  return _state;
}

PUBLIC static inline
Context*
Context::kernel_context(Cpu_number cpu)
{ return _kernel_ctxt.cpu(cpu); }

PROTECTED static inline
void
Context::kernel_context(Cpu_number cpu, Context *ctxt)
{ _kernel_ctxt.cpu(cpu) = ctxt; }


/** @name State manipulation */
//@{
//-


/**
 * Does the context exist? .
 * @return true if this context has been initialized.
 */
PUBLIC inline NEEDS [Context::is_invalid]
Mword
Context::exists() const
{
  return !is_invalid();
}

/**
 * Is the context about to be deleted.
 * @return true if this context is in deletion.
 */
PUBLIC inline NEEDS ["thread_state.h"]
bool
Context::is_invalid(bool check_cpu_local = false) const
{
  unsigned s = state(check_cpu_local);
  return (s & Thread_dead);
}

/**
 * Atomically add bits to state flags.
 * @param bits bits to be added to state flags
 * @return 1 if none of the bits that were added had been set before
 */
PUBLIC inline NEEDS ["atomic.h"]
void
Context::state_add(Mword bits)
{
  assert_kdb(check_for_current_cpu());
  atomic_or(&_state, bits);
}

/**
 * Add bits in state flags. Unsafe (non-atomic) and
 *        fast version -- you must hold the kernel lock when you use it.
 * @pre cpu_lock.test() == true
 * @param bits bits to be added to state flags
 */ 
PUBLIC inline
void
Context::state_add_dirty(Mword bits, bool check = true)
{
  (void)check;
  assert_kdb(!check || check_for_current_cpu());
  _state |= bits;
}

/**
 * Atomically delete bits from state flags.
 * @param bits bits to be removed from state flags
 * @return 1 if all of the bits that were removed had previously been set
 */
PUBLIC inline NEEDS ["atomic.h"]
void
Context::state_del(Mword bits)
{
  assert_kdb (check_for_current_cpu());
  atomic_and(&_state, ~bits);
}

/**
 * Delete bits in state flags. Unsafe (non-atomic) and
 *        fast version -- you must hold the kernel lock when you use it.
 * @pre cpu_lock.test() == true
 * @param bits bits to be removed from state flags
 */
PUBLIC inline
void
Context::state_del_dirty(Mword bits, bool check = true)
{
  (void)check;
  assert_kdb(!check || check_for_current_cpu());
  _state &= ~bits;
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
Context::state_change_safely(Mword mask, Mword bits)
{
  assert_kdb (check_for_current_cpu());
  Mword old;

  do
    {
      old = _state;
      if (old & bits & mask | ~old & ~mask)
        return 0;
    }
  while (!cas(&_state, old, old & mask | bits));

  return 1;
}

/**
 * Atomically delete and add bits in state flags.
 * @param mask bits not set in mask shall be deleted from state flags
 * @param bits bits to be added to state flags
 */
PUBLIC inline NEEDS ["atomic.h"]
Mword
Context::state_change(Mword mask, Mword bits)
{
  assert_kdb (check_for_current_cpu());
  return atomic_change(&_state, mask, bits);
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
Context::state_change_dirty(Mword mask, Mword bits, bool check = true)
{
  (void)check;
  assert_kdb(!check || check_for_current_cpu());
  _state &= mask;
  _state |= bits;
}

//@}
//-


PUBLIC inline
Context_space_ref *
Context::space_ref()
{ return &_space; }

PUBLIC inline
Space *
Context::vcpu_aware_space() const
{ return _space.vcpu_aware(); }

/** Registers used when iret'ing to user mode.
    @return return registers
 */
PUBLIC inline NEEDS["cpu.h", "entry_frame.h"]
Entry_frame *
Context::regs() const
{
  return reinterpret_cast<Entry_frame *>
    (Cpu::stack_align(reinterpret_cast<Mword>(this) + Size)) - 1;
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

/**
 * Switch active timeslice of this Context.
 * @param next Sched_context to switch to
 */
PUBLIC
void
Context::switch_sched(Sched_context *next, Sched_context::Ready_queue *queue)
{
  queue->switch_sched(sched(), next);
  set_sched(next);
}

/**
 * Select a different context for running and activate it.
 */
PUBLIC
void
Context::schedule()
{
  auto guard = lock_guard(cpu_lock);
  assert (!Sched_context::rq.current().schedule_in_progress);

  CNT_SCHEDULE;

  // Ensure only the current thread calls schedule
  assert_kdb (this == current());

  Cpu_number current_cpu = Cpu_number::nil();
  Sched_context::Ready_queue *rq = 0;

  // Enqueue current thread into ready-list to schedule correctly
  update_ready_list();

  // Select a thread for scheduling.
  Context *next_to_run;

  do
    {
      // I may've been migrated during the switch_exec_locked in the while
      // statement below. So check out if I've to use a new ready queue.
        {
          Cpu_number new_cpu = access_once(&_home_cpu);
          if (new_cpu != current_cpu)
            {
              Mem::barrier();
              current_cpu = new_cpu;
              rq = &Sched_context::rq.current();
              if (rq->schedule_in_progress)
                return;
            }
        }

      for (;;)
	{
	  next_to_run = rq->next_to_run()->context();

	  // Ensure ready-list sanity
	  assert_kdb (next_to_run);

	  if (EXPECT_TRUE (next_to_run->state() & Thread_ready_mask))
	    break;

          rq->ready_dequeue(next_to_run->sched());

	  rq->schedule_in_progress = this;

	  Proc::preemption_point();

	  // check if we've been migrated meanwhile
	  if (EXPECT_FALSE(current_cpu != access_once(&_home_cpu)))
	    {
	      current_cpu = _home_cpu;
              Mem::barrier();
	      rq = &Sched_context::rq.current();
	      if (rq->schedule_in_progress)
		return;
	    }
	  else
	    rq->schedule_in_progress = 0;
	}
    }
  while (EXPECT_FALSE(schedule_switch_to_locked(next_to_run)));
}


PUBLIC inline
void
Context::schedule_if(bool s)
{
  if (!s || Sched_context::rq.current().schedule_in_progress)
    return;

  schedule();
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
Context::set_sched(Sched_context * const sched)
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
Context::set_period(Unsigned64 const period)
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
Context::set_mode(Context::Sched_mode const mode)
{
  _mode = mode;
}

// queue operations

// XXX for now, synchronize with global kernel lock
//-

/**
 * Enqueue current() if ready to fix up ready-list invariant.
 */
PRIVATE inline
void
Context::update_ready_list()
{
  assert_kdb (this == current());

  if ((state() & Thread_ready_mask) && sched()->left())
    Sched_context::rq.current().ready_enqueue(sched());
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
 * \brief Activate a newly created thread.
 *
 * This function sets a new thread onto the ready list and switches to
 * the thread if it can preempt the currently running thread.
 */
PUBLIC
bool
Context::activate()
{
  auto guard = lock_guard(cpu_lock);
  if (home_cpu() == current_cpu())
    {
      state_add_dirty(Thread_ready);
      if (Sched_context::rq.current().deblock(sched(), current()->sched(), true))
	{
	  current()->switch_to_locked(this);
	  return true;
	}
    }
  else
    remote_ready_enqueue();

  return false;
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
Context::set_helper(Helping_mode const mode)
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
Context::set_donatee(Context * const donatee)
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
Context::set_kernel_sp(Mword * const esp)
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
  if (Config::Fine_grained_cputime)
    consume_time(_clock.current().delta());
}

IMPLEMENT inline NEEDS ["config.h", "cpu.h"]
Cpu_time
Context::consumed_time()
{
  if (Config::Fine_grained_cputime)
    return _clock.current().us(_consumed_time);

  return _consumed_time;
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

  Sched_context::Ready_queue &rq = Sched_context::rq.current();
  // Switch to destination thread's scheduling context
  if (rq.current_sched() != t->sched())
    rq.set_current_sched(t->sched());

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

PRIVATE inline
Context *
Context::handle_helping(Context *t)
{
  // XXX: maybe we do not need this on MP, because we have no helping there
  assert_kdb (current() == this);
  // Time-slice lending: if t is locked, switch to its locker
  // instead, this is transitive
  while (t->donatee() &&		// target thread locked
         t->donatee() != t)		// not by itself
    {
      // Special case for Thread::kill(): If the locker is
      // current(), switch to the locked thread to allow it to
      // release other locks.  Do this only when the target thread
      // actually owns locks.
      if (t->donatee() == this)
        {
          if (t->lock_cnt() > 0)
            break;

          return this;
        }

      t = t->donatee();
    }
  return t;
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
Context::switch_exec_locked(Context *t, enum Helping_mode mode)
{
  // Must be called with CPU lock held
  assert_kdb (t);
  assert_kdb (cpu_lock.test());
  assert_kdb (current() != t);
  assert_kdb (current() == this);

  // only for logging
  Context *t_orig = t;
  (void)t_orig;

  // Time-slice lending: if t is locked, switch to its locker
  // instead, this is transitive
  //
  // For the Thread_lock case only, so skip this
  // t = handle_helping(t);
  assert (handle_helping(t) == t);

  if (t == this)
    return handle_drq();

  if (EXPECT_FALSE(t->running_on_different_cpu()))
    {
      if (!t->in_ready_list())
        Sched_context::rq.current().ready_enqueue(t->sched());
      return true;
    }


  LOG_CONTEXT_SWITCH;
  CNT_CONTEXT_SWITCH;

  // Can only switch to ready threads!
  // do not consider CPU locality here t can be temporarily migrated
  if (EXPECT_FALSE (!(t->state(false) & Thread_ready_mask)))
    {
      assert_kdb (state(false) & Thread_ready_mask);
      return false;
    }


  // Ensure kernel stack pointer is non-null if thread is ready
  assert_kdb (t->_kernel_sp);

  t->set_helper(mode);

  update_ready_list();
  assert_kdb (!(state(false) & Thread_ready_mask) || !sched()->left()
              || in_ready_list());

  t->set_current_cpu(get_current_cpu());
  switch_fpu(t);
  switch_cpu(t);

  return handle_drq();
}

PUBLIC
bool
Context::switch_exec_helping(Context *t, Helping_mode mode, Mword const *lock, Mword val)
{
  // Must be called with CPU lock held
  assert_kdb (t);
  assert_kdb (cpu_lock.test());
  assert_kdb (current() != t);
  assert_kdb (current() == this);

  // only for logging
  Context *t_orig = t;
  (void)t_orig;

  // Time-slice lending: if t is locked, switch to its locker
  // instead, this is transitive
  //
  // For the Thread_lock case only, so skip this
  // t = handle_helping(t);

  // we actually hold locks
  if (!t->need_help(lock, val))
    return true;

  // Can only switch to ready threads!
  // do not consider CPU locality here t can be temporarily migrated
  if (EXPECT_FALSE (!(t->state(false) & Thread_ready_mask)))
    {
      assert_kdb (state(false) & Thread_ready_mask);
      return true;
    }


  // Ensure kernel stack pointer is non-null if thread is ready
  assert_kdb (t->_kernel_sp);

  update_ready_list();

  t->set_helper(mode);
  t->set_current_cpu(get_current_cpu());
  switch_fpu(t);
  switch_cpu(t);
  return handle_drq();
}

PUBLIC inline
Context::Ku_mem_ptr<Utcb> const &
Context::utcb() const
{ return _utcb; }

IMPLEMENT inline NEEDS["globals.h"]
Context *
Context::Context_member::context() const
{ return context_of(this); }

IMPLEMENT inline NEEDS["lock_guard.h", "kdb_ke.h"]
void
Context::Drq_q::enq(Drq *rq)
{
  assert_kdb(cpu_lock.test());
  auto guard = lock_guard(q_lock());
  enqueue(rq);
}

PRIVATE inline
bool
Context::do_drq_reply(Drq *r, Drq_q::Drop_mode drop)
{
  state_change_dirty(~Thread_drq_wait, Thread_ready);
  // r->state = Drq::Reply_handled;
  if (drop == Drq_q::No_drop && r->reply)
    return r->reply(r, this, r->arg).need_resched();

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
      LOG_TRACE("DRQ handling", "drq", current(), Drq_log,
	  l->type = Drq_log::Type::Do_reply;
          l->rq = r;
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
      LOG_TRACE("DRQ handling", "drq", current(), Drq_log,
	  l->type = Drq_log::Type::Do_request;
          l->rq = r;
	  l->func = (void*)r->func;
	  l->reply = (void*)r->reply;
	  l->thread = r->context();
	  l->target_cpu = current_cpu();
	  l->wait = 0;
      );
      // r->state = Drq::Idle;
      Drq::Result answer = Drq::done();
      //LOG_MSG_3VAL(current(), "hrq", current_cpu() | (drop ? 0x100: 0), (Mword)r->context(), (Mword)r->func);
      if (EXPECT_TRUE(drop == No_drop && r->func))
	answer = r->func(r, self, r->arg);
      else if (EXPECT_FALSE(drop == Drop))
	// flag DRQ abort for requester
	r->arg = (void*)-1;
       // LOG_MSG_3VAL(current(), "hrq-", answer, current()->state() /*(Mword)r->context()*/, (Mword)r->func);
      need_resched |= answer.need_resched();
      //r->state = Drq::Handled;

      // enqueue answer
      if (!(answer.no_answer()))
	{
	  if (local)
	    return r->context()->do_drq_reply(r, drop) || need_resched;
	  else
	    need_resched |= r->context()->enqueue_drq(r, Drq::Target_ctxt);
	}
    }
  return need_resched;
}

IMPLEMENT inline NEEDS["lock_guard.h"]
bool
Context::Drq_q::dequeue(Drq *drq, Queue_item::Status reason)
{
  auto guard = lock_guard(q_lock());
  if (!drq->queued())
    return false;
  return Queue::dequeue(drq, reason);
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
	  auto guard = lock_guard(q_lock());
	  qi = first();
	  if (!qi)
	    return need_resched;

	  check_kdb (Queue::dequeue(qi, Queue_item::Ok));
	}

      Drq *r = static_cast<Drq*>(qi);
      // printf("CPU[%2u:%p]: context=%p: handle request for %p (func=%p, arg=%p)\n", current_cpu(), current(), context(), r->context(), r->func, r->arg);
      need_resched |= execute_request(r, drop, false);
    }
}

IMPLEMENT inline NEEDS["lock_guard.h", "kdb_ke.h"]
void
Context::Cpu_call_queue::enq(Cpu_call *rq)
{
  assert_kdb(cpu_lock.test());
  auto guard = lock_guard(q_lock());
  enqueue(rq);
}

IMPLEMENT inline
bool
Context::Cpu_call_queue::execute_request(Cpu_call *r)
{
  return r->run(current_cpu(), true);
}

IMPLEMENT inline NEEDS["lock_guard.h"]
bool
Context::Cpu_call_queue::dequeue(Cpu_call *r, Queue_item::Status reason)
{
  auto guard = lock_guard(q_lock());
  if (!r->queued())
    return false;
  return Queue::dequeue(r, reason);
}

IMPLEMENT inline NEEDS["mem.h", "lock_guard.h"]
bool
Context::Cpu_call_queue::handle_requests()
{
  bool need_resched = false;
  while (1)
    {
      Queue_item *qi;
	{
	  auto guard = lock_guard(q_lock());
	  qi = first();
	  if (!qi)
	    return need_resched;

	  check_kdb (Queue::dequeue(qi, Queue_item::Ok));
	}

      Cpu_call *r = static_cast<Cpu_call*>(qi);
      need_resched |= execute_request(r);
    }
}


/**
 * \brief Forced dequeue from lock wait queue, or DRQ queue.
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
	  auto guard = lock_guard(q->q_lock());
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
  if (EXPECT_FALSE(_need_to_finish_migration))
    {
      _need_to_finish_migration = false;
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
  if (EXPECT_FALSE(current_cpu() != home_cpu())
      && home_cpu() != Cpu::invalid())
    return false;

  assert_kdb (check_for_current_cpu());
  assert_kdb (cpu_lock.test());

  try_finish_migration();

  if (EXPECT_TRUE(!drq_pending()))
    return false;

  Mem::barrier();
  bool ret = _drq_q.handle_requests();
  state_del_dirty(Thread_drq_ready);

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
Context::Drq::Result
Context::handle_drq_state_change(Drq * /*src*/, Context *self, void * _rq)
{
  State_request *rq = reinterpret_cast<State_request*>(_rq);
  self->state_change_dirty(rq->del, rq->add);
  //LOG_MSG_3VAL(c, "dsta", c->state(), (Mword)src, (Mword)_rq);
  return Drq::done();
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
  if (current() == this)
    {
      state_change_dirty(mask, add);
      return;
    }

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
 * DRQs are requests that any context can queue to any other context. DRQs are
 * the basic mechanism to initiate actions on remote CPUs in an MP system,
 * however, are also allowed locally.
 * DRQ handlers of pending DRQs are executed by Context::handle_drq() in the
 * context of the target context. Context::handle_drq() is basically called
 * after switching to a context in Context::switch_exec_locked().
 *
 * This function enqueues a DRQ and blocks the current context for a reply DRQ.
 */
PUBLIC inline NEEDS[Context::enqueue_drq, "logdefs.h"]
void
Context::drq(Drq *drq, Drq::Request_func *func, void *arg,
             Drq::Request_func *reply = 0,
             Drq::Exec_mode exec = Drq::Target_ctxt,
             Drq::Wait_mode wait = Drq::Wait)
{
  // printf("CPU[%2u:%p]: > Context::drq(this=%p, src=%p, func=%p, arg=%p)\n", current_cpu(), current(), this, src, func,arg);
  Context *cur = current();
  LOG_TRACE("DRQ handling", "drq", cur, Drq_log,
      l->type = Drq_log::Type::Send;
      l->rq = drq;
      l->func = (void*)func;
      l->reply = (void*)reply;
      l->thread = this;
      l->target_cpu = home_cpu();
      l->wait = wait;
  );
  //assert_kdb (current() == src);
  assert_kdb (!(wait == Drq::Wait && (cur->state() & Thread_drq_ready)) || cur->home_cpu() == home_cpu());
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

  LOG_TRACE("DRQ handling", "drq", cur, Drq_log,
      l->type = Drq_log::Type::Done;
      l->rq = drq;
      l->func = (void*)func;
      l->reply = (void*)reply;
      l->thread = this;
      l->target_cpu = home_cpu();
  );
  //LOG_MSG_3VAL(src, "drq>", src->state(), Mword(this), 0);
}

PUBLIC inline NEEDS[Context::update_ready_list]
bool
Context::kernel_context_drq(Drq::Request_func *func, void *arg,
                            Drq::Request_func *reply = 0)
{
  char align_buffer[2*sizeof(Drq)];
  Drq *mdrq = new ((void*)((Address)(align_buffer + __alignof__(Drq) - 1) & ~(__alignof__(Drq)-1))) Drq;

  update_ready_list();

  mdrq->func  = func;
  mdrq->arg   = arg;
  mdrq->reply = reply;
  Context *kc = kernel_context(current_cpu());
  kc->_drq_q.enq(mdrq);
  return schedule_switch_to_locked(kc);
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
  Sched_context::rq.current().deblock(c->sched());
  return true;
}

PUBLIC inline
void
Context::recover_jmp_buf(jmp_buf *b)
{ _recover_jmpbuf = b; }

PUBLIC static
void
Context::xcpu_tlb_flush(...)
{
  // This should always be optimized away
  assert(0);
}

IMPLEMENT_DEFAULT inline
void
Context::arch_load_vcpu_kern_state(Vcpu_state *, bool)
{}

IMPLEMENT_DEFAULT inline
void
Context::arch_load_vcpu_user_state(Vcpu_state *, bool)
{}

IMPLEMENT_DEFAULT inline
void
Context::arch_update_vcpu_state(Vcpu_state *)
{}

//----------------------------------------------------------------------------
IMPLEMENTATION [!mp]:

#include "warn.h"
#include "kdb_ke.h"

PUBLIC static inline
void
Context::enable_tlb(Cpu_number)
{}

PUBLIC static inline
void
Context::disable_tlb(Cpu_number)
{}

PRIVATE inline
void
Context::handle_lock_holder_preemption()
{}

/** Increment lock count.
    @post lock_cnt() > 0 */
PUBLIC inline
void
Context::inc_lock_cnt()
{
  ++_lock_cnt;
}

/** Decrement lock count.
    @pre lock_cnt() > 0
 */
PUBLIC inline
void
Context::dec_lock_cnt()
{
  --_lock_cnt;
}

PRIVATE inline
bool
Context::running_on_different_cpu()
{ return false; }

PRIVATE inline
bool
Context::need_help(Mword const *, Mword)
{ return true; }



PROTECTED
void
Context::remote_ready_enqueue()
{
  WARN("Context::remote_ready_enqueue(): in UP system !\n");
  kdb_ke("Fiasco BUG");
}

PUBLIC
bool
Context::enqueue_drq(Drq *rq, Drq::Exec_mode /*exec*/)
{
  assert_kdb (cpu_lock.test());

  LOG_TRACE("DRQ handling", "drq", current(), Drq_log,
      l->type = rq->context() == this
                                 ? Drq_log::Type::Send_reply
                                 : Drq_log::Type::Do_send;
      l->func = (void*)rq->func;
      l->reply = (void*)rq->reply;
      l->thread = this;
      l->target_cpu = home_cpu();
      l->wait = 0;
      l->rq = rq;
  );

  if (access_once(&_home_cpu) != current_cpu())
    {
      bool do_sched = _drq_q.execute_request(rq, Drq_q::No_drop, true);
      //LOG_MSG_3VAL(this, "drqX", access_once(&_cpu), current_cpu(), state());
      if (access_once(&_home_cpu) == current_cpu() && (state() & Thread_ready_mask))
        {
          Sched_context::rq.current().ready_enqueue(sched());
          return true;
        }
      return do_sched;
    }
  else
    { // LOG_MSG_3VAL(this, "adrq", state(), (Mword)current(), (Mword)rq);

      bool do_sched = _drq_q.execute_request(rq, Drq_q::No_drop, true);
      if (!in_ready_list() && (state() & Thread_ready_mask))
	{
	  Sched_context::rq.current().ready_enqueue(sched());
	  return true;
	}

      return do_sched;
    }
  return false;
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
bool
Context::cpu_call_many(Cpu_mask const &, cxx::functor<bool (Cpu_number)> &&func)
{
  func(current_cpu());
  return true;
}

PUBLIC static inline
Cpu_mask
Context::active_tlb()
{ return Cpu_mask(); }

PUBLIC static bool Context::handle_global_requests() { return false; }


//----------------------------------------------------------------------------
INTERFACE [mp]:

#include "queue.h"
#include "queue_item.h"

EXTENSION class Context
{
private:
  friend class Switch_lock;
  // FIXME: could be a byte, but nee cas byte for that
  Mword _running_under_lock;

protected:

  class Pending_rqq : public Queue
  {
  public:
    static void enq(Context *c);
    bool handle_requests(Context **);
  };

  class Pending_rq : public Queue_item, public Context_member
  {} _pending_rq;

protected:
  static Per_cpu<Pending_rqq> _pending_rqq;
  static Per_cpu<Cpu_call_queue> _glbl_q;
  static Cpu_mask _tlb_active;

};



//----------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include "globals.h"
#include "ipi.h"
#include "kdb_ke.h"
#include "lock_guard.h"
#include "mem.h"

DEFINE_PER_CPU Per_cpu<Context::Pending_rqq> Context::_pending_rqq;
DEFINE_PER_CPU Per_cpu<Context::Cpu_call_queue> Context::_glbl_q;
Cpu_mask Context::_tlb_active;

PRIVATE inline
void
Context::handle_lock_holder_preemption()
{
  assert (current() != this);
  write_now(&_running_under_lock, Mword(false));
}

PUBLIC static inline
Cpu_mask const &
Context::active_tlb()
{ return _tlb_active; }

PUBLIC static inline
void
Context::enable_tlb(Cpu_number cpu)
{ _tlb_active.atomic_set(cpu); }

PUBLIC static inline
void
Context::disable_tlb(Cpu_number cpu)
{ _tlb_active.atomic_clear(cpu); }

/** Increment lock count.
    @post lock_cnt() > 0 */
PUBLIC inline
void
Context::inc_lock_cnt()
{
  write_now(&_lock_cnt, _lock_cnt + 1);
}

/** Decrement lock count.
    @pre lock_cnt() > 0
 */
PUBLIC inline
void
Context::dec_lock_cnt()
{
  --_lock_cnt;
  if (EXPECT_TRUE(home_cpu() == current_cpu() && !_lock_cnt))
    {
      Mem::mp_wmb();
      write_now(&_running_under_lock, Mword(false));
    }
}

PRIVATE inline
bool
Context::running_on_different_cpu()
{
  // we actually hold locks
  if (EXPECT_FALSE(_running_under_lock))
    return true;

  if (EXPECT_FALSE(lock_cnt())
      && EXPECT_FALSE(!mp_cas(&_running_under_lock, Mword(false), Mword(true))))
    return true;

  return false;
}

PRIVATE inline
bool
Context::need_help(Mword const *lock, Mword val)
{
  if (EXPECT_FALSE(_running_under_lock))
    return false;

  if (EXPECT_FALSE(!mp_cas(&_running_under_lock, Mword(false), Mword(true))))
    return false;

  Mem::mp_mb();

  // double check if the lock is held by us
  if (EXPECT_FALSE(access_once(lock) != val))
    {
      write_now(&_running_under_lock, Mword(false));
      return false;
    }

  return true;
}

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
      Queue &q = Context::_pending_rqq.cpu(c->home_cpu());
      auto guard = lock_guard(q.q_lock());
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
      Context *c;
        {
          auto guard = lock_guard(q_lock());
          Queue_item *qi = first();
          if (!qi)
            return resched;

          check_kdb (dequeue(qi, Queue_item::Ok));
          c = static_cast<Context::Pending_rq *>(qi)->context();
        }

      assert_kdb (c->check_for_current_cpu());

      if (EXPECT_FALSE(c->_migration != 0))
	{
          // if the currently executing thread shall be migrated we must defer
          // this until we have handled the whole request queue, otherwise we
          // would miss the remaining requests or execute them on the wrong CPU.
	  if (c != curr)
	    {
              // we can directly migrate the thread...
	      resched |= c->initiate_migration();

              // if migrated away skip the resched test below
              if (access_once(&c->_home_cpu) != current_cpu())
                continue;
	    }
	  else
            *mq = c;
	}
      else
	c->try_finish_migration();

      if (EXPECT_TRUE(c != curr && c->drq_pending()))
        c->state_add(Thread_drq_ready);

      // FIXME: must we also reschedule when c cannot preempt the current
      // thread but its current scheduling context?
      if (EXPECT_TRUE(c != curr && (c->state() & Thread_ready_mask)))
	{
	  //printf("CPU[%2u:%p]:   Context::Pending_rqq::handle_requests() dequeded %p(%u)\n", current_cpu(), current(), c, qi->queued());
	  resched |= Sched_context::rq.current().deblock(c->sched(), curr->sched(), false);
	}
    }
}

IMPLEMENT inline NEEDS["ipi.h"]
bool
Context::Cpu_call::remote_call(Cpu_number cpu)
{
  auto guard = lock_guard(cpu_lock);
  if (current_cpu() == cpu)
    {
      assert (is_done());
      run(cpu, false);
      return false;
    }

  Cpu_call_queue &q = _glbl_q.cpu(cpu);

  set_queued();
  Mem::mp_mb();
  assert (!is_done());
  q.enq(this);

  Mem::mp_mb();
  if (EXPECT_FALSE(!Cpu::online(cpu)))
    {
      Mem::mp_mb();
      if (q.dequeue(this, Queue_item::Ok))
        done();
      assert (is_done());
      return false;
    }

  if (!is_done())
    {
      Ipi::send(Ipi::Global_request, current_cpu(), cpu);
      return true;
    }
  return false;
}

PUBLIC static inline
bool
Context::cpu_call_many(Cpu_mask const &cpus, cxx::functor<bool (Cpu_number)> &&func)
{
  assert_kdb (!cpu_lock.test());
  Cpu_calls<8> cs;
  Cpu_number n;
  Cpu_call *c = cs.next();
  for (n = Cpu_number::first(); n < Config::max_num_cpus() && c; ++n)
    {
      if (!cpus.get(n))
        continue;

      c->set(func);
      if (c->remote_call(n))
        c = cs.next();
    }

  // hmm, nothing to do, we should optimize this and check
  // this before the loops
  if (cs.used() == 0)
    return true;

  for (; n < Config::max_num_cpus(); ++n)
    {
      if (!cpus.get(n))
        continue;

      while (!(c = cs.find_done()))
        Proc::pause();

      c->remote_call(n);
    }

  cs.wait_all();
  return true;
}

PUBLIC
static bool
Context::handle_global_requests()
{
  return _glbl_q.current().handle_requests();
}


PUBLIC static inline
void
Context::take_cpu_offline(Cpu_number cpu)
{
  assert_kdb (cpu == current_cpu());
  (void)cpu;

    {
      Queue &q = Context::_pending_rqq.current();
      auto guard = lock_guard(q.q_lock());

      if (q.first())
        return;

      Cpu::cpus.current().set_online(false);
    }
  Mem::mp_mb();

  handle_global_requests();
}

PUBLIC static inline
void
Context::take_cpu_online(Cpu_number cpu)
{
  Cpu::cpus.cpu(cpu).set_online(true);
}


PUBLIC
bool
Context::enqueue_drq(Drq *rq, Drq::Exec_mode /*exec*/)
{
  assert_kdb (cpu_lock.test());
  // printf("CPU[%2u:%p]: Context::enqueue_request(this=%p, src=%p, func=%p, arg=%p)\n", current_cpu(), current(), this, src, func,arg);
  LOG_TRACE("DRQ handling", "drq", current(), Drq_log,
      l->type = rq->context() == this
                                 ? Drq_log::Type::Send_reply
                                 : Drq_log::Type::Do_send;
      l->func = (void*)rq->func;
      l->reply = (void*)rq->reply;
      l->thread = this;
      l->target_cpu = home_cpu();
      l->wait = 0;
      l->rq = rq;
  );

  if (home_cpu() != current_cpu())
    {
      bool ipi = false;
      _drq_q.enq(rq);

      // read cpu again we may've been migrated meanwhile
      Cpu_number cpu = access_once(&_home_cpu);

	{
	  Queue &q = Context::_pending_rqq.cpu(cpu);
	  auto guard = lock_guard(q.q_lock());


	  // migrated between getting the lock and reading the CPU, so the
	  // new CPU is responsible for executing our request
	  if (access_once(&_home_cpu) != cpu)
	    return false;

          if (EXPECT_FALSE(!Cpu::online(cpu)))
            {
              if (EXPECT_FALSE(!_drq_q.dequeue(rq, Queue_item::Ok)))
                // handled already
                return false;

              // execute locally under the target CPU's queue lock
              _drq_q.execute_request(rq, Drq_q::No_drop, true);

              // free the lock early
              guard.reset();
              if (   access_once(&_home_cpu) == current_cpu()
                  && !in_ready_list()
                  && (state() & Thread_ready_mask))
                {
                  Sched_context::rq.current().ready_enqueue(sched());
                  return true;
                }
              return false;
            }

          if (!_pending_rq.queued())
            {
              if (!q.first())
                ipi = true;

              q.enqueue(&_pending_rq);
            }
        }

      if (ipi)
	{
	  //LOG_MSG_3VAL(this, "sipi", current_cpu(), cpu(), (Mword)current());
	  Ipi::send(Ipi::Request, current_cpu(), cpu);
	}
    }
  else
    { // LOG_MSG_3VAL(this, "adrq", state(), (Mword)current(), (Mword)rq);

      bool do_sched = _drq_q.execute_request(rq, Drq_q::No_drop, true);
      if (!in_ready_list() && (state() & Thread_ready_mask))
	{
          Sched_context::rq.current().ready_enqueue(sched());
	  return true;
	}

      return do_sched;
    }
  return false;
}


PRIVATE inline NOEXPORT
void
Context::shutdown_drqs()
{
  if (_pending_rq.queued())
    {
      auto guard = lock_guard(_pending_rq.queue()->q_lock());
      if (_pending_rq.queued())
	_pending_rq.queue()->dequeue(&_pending_rq, Queue_item::Ok);
    }

  _drq_q.handle_requests(Drq_q::Drop);
}


/**
 * Remote helper for doing remote CPU ready enqueue.
 *
 * See remote_ready_enqueue().
 */
PRIVATE static
Context::Drq::Result
Context::handle_remote_ready_enqueue(Drq *, Context *self, void *)
{
  self->state_add_dirty(Thread_ready);
  return Drq::done();
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
  auto gurad = lock_guard(cpu_lock);
  state_change_dirty(~Thread_ready, Thread_waiting);
  Rcu::call(this, &rcu_unblock);
  while (state() & Thread_waiting)
    schedule();
}



//----------------------------------------------------------------------------
IMPLEMENTATION [fpu && !ux]:

#include "fpu.h"

PUBLIC inline NEEDS ["fpu.h"]
void
Context::spill_fpu()
{
  // If we own the FPU, we should never be getting an "FPU unavailable" trap
  assert_kdb (Fpu::fpu.current().owner() == this);
  assert_kdb (state() & Thread_fpu_owner);
  assert_kdb (fpu_state());

  // Save the FPU state of the previous FPU owner (lazy) if applicable
  Fpu::save_state(fpu_state());
  state_del_dirty(Thread_fpu_owner);
}


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
  Fpu &f = Fpu::fpu.current();
  if (f.is_owner(this))
    f.disable();
  else if (f.is_owner(t) && !(t->state() & Thread_vcpu_fpu_disabled))
    f.enable();
}

//----------------------------------------------------------------------------
IMPLEMENTATION [!fpu]:

PUBLIC inline
void
Context::spill_fpu()
{}

IMPLEMENT inline
void
Context::switch_fpu(Context *)
{}

//----------------------------------------------------------------------------
INTERFACE [debug]:

#include "tb_entry.h"

/** logged context switch. */
class Tb_entry_ctx_sw : public Tb_entry
{
public:
  using Tb_entry::_ip;

  Context const *dst;		///< switcher target
  Context const *dst_orig;
  Address kernel_ip;
  Mword lock_cnt;
  Space const *from_space;
  Sched_context const *from_sched;
  Mword from_prio;
  void print(String_buffer *buf) const;
} __attribute__((packed));



// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "kobject_dbg.h"
#include "string_buffer.h"

IMPLEMENT
void
Context::Drq_log::print(String_buffer *buf) const
{
  static char const *const _types[] =
    { "send", "do send", "do request", "send reply", "do reply", "done" };

  char const *t = "unk";
  if ((unsigned)type < sizeof(_types)/sizeof(_types[0]))
    t = _types[(unsigned)type];

  buf->printf("%s(%s) rq=%p to ctxt=%lx/%p (func=%p, reply=%p) cpu=%u",
      t, wait ? "wait" : "no-wait", rq, Kobject_dbg::pointer_to_id(thread),
      thread, func, reply, cxx::int_value<Cpu_number>(target_cpu));
}

// context switch
IMPLEMENT
void
Tb_entry_ctx_sw::print(String_buffer *buf) const
{
  Context *sctx = 0;
  Mword sctxid = ~0UL;
  Mword dst;
  Mword dst_orig;

  sctx = from_sched->context();
  sctxid = Kobject_dbg::pointer_to_id(sctx);

  dst = Kobject_dbg::pointer_to_id(this->dst);
  dst_orig = Kobject_dbg::pointer_to_id(this->dst_orig);

  if (sctx != ctx())
    buf->printf("(%lx)", sctxid);

  buf->printf(" ==> %lx ", dst);

  if (dst != dst_orig || lock_cnt)
    buf->printf("(");

  if (dst != dst_orig)
    buf->printf("want %lx", dst_orig);

  if (dst != dst_orig && lock_cnt)
    buf->printf(" ");

  if (lock_cnt)
    buf->printf("lck %ld", lock_cnt);

  if (dst != dst_orig || lock_cnt)
    buf->printf(") ");

  buf->printf(" krnl " L4_PTR_FMT, kernel_ip);
}


