INTERFACE:

#include "cpu_mask.h"
#include "per_cpu_data.h"
#include "spin_lock.h"

class Rcu_glbl;
class Rcu_data;

/**
 * \brief Encapsulation of RCU batch number.
 */
class Rcu_batch
{
  friend class Jdb_rcupdate;
public:
  /// create uninitialized batch.
  Rcu_batch() {}
  /// create a btach initialized with \a b.
  Rcu_batch(long b) : _b(b) {}

  /// less than comparison.
  bool operator < (Rcu_batch const &o) const { return (_b - o._b) < 0; }
  /// greater than comparison.
  bool operator > (Rcu_batch const &o) const { return (_b - o._b) > 0; }
  /// greater than comparison.
  bool operator >= (Rcu_batch const &o) const { return (_b - o._b) >= 0; }
  /// equelity check.
  bool operator == (Rcu_batch const &o) const { return _b == o._b; }
  /// inequality test.
  bool operator != (Rcu_batch const &o) const { return _b != o._b; }
  /// increment batch.
  Rcu_batch &operator ++ () { ++_b; return *this; }
  /// increase batch with \a r.
  Rcu_batch operator + (long r) { return Rcu_batch(_b + r); }


private:
  long _b;
};

/**
 * \brief Item that can bequeued for the next grace period.
 *
 * An RCU item is basically a pointer to a callback which is called
 * after one grace period.
 */
class Rcu_item
{
  friend class Rcu_list;
  friend class Rcu_data;
  friend class Rcu;
  friend class Jdb_rcupdate;

private:
  bool (*_call_back)(Rcu_item *);
  Rcu_item *_next;
public:
  Rcu_item *next_rcu_item() const { return _next; }
};


/**
 * \brief List of Rcu_items.
 *
 * RCU lists are used a lot of times in the RCU implementation and are
 * implemented as single linked lists with FIFO semantics.
 *
 * \note Concurrent access to the list is not synchronized.
 */
class Rcu_list
{
  friend class Jdb_rcupdate;
public:
  /// create an empty list.
  Rcu_list() : _l(0), _t(&_l) {}

private:
  Rcu_list(Rcu_list const &);
  Rcu_item *_l;
  Rcu_item **_t;
};

/**
 * \brief CPU local data structure for RCU.
 */
class Rcu_data
{
  friend class Jdb_rcupdate;
public:

  Rcu_batch _q_batch;   ///< batch nr. for grace period
  bool _q_passed;       ///< quiescent state passed?
  bool _pending;        ///< wait for quiescent state

  Rcu_batch _batch;
  Rcu_list _n;
  long _len;
  Rcu_list _c;
  Rcu_list _d;
  unsigned _cpu;
};


/**
 * \brief Global RCU data structure.
 */
class Rcu_glbl
{
  friend class Rcu_data;
  friend class Rcu;
  friend class Jdb_rcupdate;

private:
  Rcu_batch _current;      ///< current batch
  Rcu_batch _completed;    ///< last completed batch
  bool _next_pending;      ///< next batch already pending?
  Spin_lock<> _lock;
  Cpu_mask _cpus;

};

/**
 * \brief encapsulation of RCU implementation.
 *
 * This calss aggregates per CPU data structures as well as the global
 * data structure for RCU and provides a common RCU interface.
 */
class Rcu
{
  friend class Rcu_data;
  friend class Jdb_rcupdate;

public:
  /// The lock to prevent a quiescent state.
  typedef Cpu_lock Lock;
  enum { Period = 3000 /* 10ms */ };
  static Rcu_glbl *rcu() { return &_rcu; }
private:
  static Rcu_glbl _rcu;
  static Per_cpu<Rcu_data> _rcu_data;
};

// ------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Rcu
{
public:
  struct Log_rcu
  {
    unsigned cpu;
    Rcu_item *item;
    void *cb;
    unsigned char event;
  } __attribute__((packed));

  enum Rcu_events
  {
    Rcu_call = 0,
    Rcu_process = 1,
  };
};


// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "logdefs.h"

unsigned log_fmt(Tb_entry *, int max, char *buf) asm ("__fmt_rcu");


IMPLEMENT
unsigned log_fmt(Tb_entry *e, int max, char *buf)
{
  Rcu::Log_rcu *p = e->payload<Rcu::Log_rcu>();
  char const *events[] = { "call", "process"};
  return snprintf(buf, max, "rcu-%s (cpu=%u) item=%p", events[p->event], p->cpu, p->item);
}


//--------------------------------------------------------------------------
IMPLEMENTATION:

#include "cpu.h"
#include "cpu_lock.h"
#include "globals.h"
#include "kdb_ke.h"
#include "lock_guard.h"
#include "mem.h"
#include "static_init.h"
#include "timeout.h"
#include "logdefs.h"

// XXX: includes for debugging
// #include "logdefs.h"


class Rcu_timeout : public Timeout
{
};

/**
 * Timeout expiration callback function
 * @return true if reschedule is necessary, false otherwise
 */
PRIVATE
bool
Rcu_timeout::expired()
{ return Rcu::process_callbacks(); }


Rcu_glbl Rcu::_rcu INIT_PRIORITY(EARLY_INIT_PRIO);
Per_cpu<Rcu_data> DEFINE_PER_CPU Rcu::_rcu_data(true);
static Per_cpu<Rcu_timeout> DEFINE_PER_CPU _rcu_timeout;

PUBLIC
Rcu_glbl::Rcu_glbl()
: _current(-300),
  _completed(-300)
{}

PUBLIC
Rcu_data::Rcu_data(unsigned cpu)
: _q_batch(Rcu::rcu()->_completed),
  _cpu(cpu)
{}


/**
 * \brief Enqueue Rcu_item into the list (at the tail).
 * \prarm i the RCU item to enqueue.
 */
PUBLIC inline
void
Rcu_list::enqueue(Rcu_item *i)
{
  *_t = i;
  i->_next = 0;
  _t = &i->_next;
}

PUBLIC inline
Rcu_item *
Rcu_list::head() const { return _l; }

PUBLIC inline
void
Rcu_list::head(Rcu_item *h)
{
  _l = h;
  if (!h)
    _t = &_l;
}

PUBLIC inline
void
Rcu_list::clear()
{ _l = 0; _t = &_l; }


PUBLIC inline NEEDS["kdb_ke.h"]
void
Rcu_list::operator = (Rcu_list const &o)
{
  assert_kdb(o._l); // assignment is allowed only from a non-empty list
  assert_kdb(!_l);  // to an empty list
  _l = o._l; _t = o._t;
}

PUBLIC inline
void
Rcu_list::append(Rcu_list &o)
{
  *_t = o._l;
  if (o._l)
    _t = o._t;

  o._l = 0;
  o._t = &o._l;
}

PUBLIC inline
bool
Rcu_list::empty() { return !_l; }

PUBLIC inline
bool
Rcu_list::full() { return _l; }

/**
 * \pre must run under cpu lock
 */
PUBLIC inline
void
Rcu_data::enqueue(Rcu_item *i)
{
  _n.enqueue(i);
  ++_len;
}

PRIVATE inline NOEXPORT NEEDS["cpu_lock.h", "lock_guard.h"]
bool
Rcu_data::do_batch()
{
  Rcu_item *n, *l;
  int count = 0;
  bool need_resched = false;
  l = _d.head();
  while (l)
    {
      n = l->_next;
      need_resched |= l->_call_back(l);
      l = n;
      ++count;
    }
  _d.head(l);

    {
      Lock_guard<Cpu_lock> guard(&cpu_lock);
      _len -= count;
    }
#if 0
  if (_d.full())
    {
      Timeout *t = &_rcu_timeout.cpu(_cpu);
      t->set(t->get_timeout(0) + Rcu::Period, _cpu);
    }
#endif
  return need_resched;
}

PRIVATE inline NOEXPORT
void
Rcu_glbl::start_batch()
{
  if (_next_pending && _completed == _current)
    {
      _next_pending = 0;
      Mem::mp_wmb();
      ++_current;
      Mem::mp_mb();
      _cpus = Cpu::online_mask();
    }
}

PRIVATE inline NOEXPORT
void
Rcu_glbl::cpu_quiet(unsigned cpu)
{
  _cpus.clear(cpu);
  if (_cpus.empty())
    {
      _completed = _current;
      start_batch();
    }
}

PRIVATE
void
Rcu_data::check_quiescent_state(Rcu_glbl *rgp)
{
  if (_q_batch != rgp->_current)
    {
      // start new grace period
      _pending = 1;
      _q_passed = 0;
      _q_batch = rgp->_current;
      return;
    }

  // Is the grace period already completed for this cpu?
  // use _pending, not bitmap to avoid cache trashing
  if (!_pending)
    return;

  // Was there a quiescent state since the beginning of the grace period?
  if (!_q_passed)
    return;

  _pending = 0;

  Lock_guard<typeof(rgp->_lock)> guard(&rgp->_lock);

  if (EXPECT_TRUE(_q_batch == rgp->_current))
    rgp->cpu_quiet(_cpu);
}


PUBLIC static //inline NEEDS["cpu_lock.h", "globals.h", "lock_guard.h", "logdefs.h"]
void
Rcu::call(Rcu_item *i, bool (*cb)(Rcu_item *))
{
  i->_call_back = cb;
  i->_next = 0;
  LOG_TRACE("Rcu call", "rcu", ::current(), __fmt_rcu,
      Log_rcu *p = tbe->payload<Log_rcu>();
      p->cpu   = current_cpu();
      p->event = Rcu_call;
      p->item = i;
      p->cb = (void*)cb);

  Lock_guard<Cpu_lock> guard(&cpu_lock);

  Rcu_data *rdp = &_rcu_data.cpu(current_cpu());
  rdp->enqueue(i);
}

PRIVATE
void
Rcu_data::move_batch(Rcu_list &l)
{
  Lock_guard<Cpu_lock> guard(&cpu_lock);
  _n.append(l);
}


PUBLIC
Rcu_data::~Rcu_data()
{
  if (current_cpu() == _cpu)
    return;

  Rcu_data *current_rdp = &Rcu::_rcu_data.cpu(current_cpu());
  Rcu_glbl *rgp = Rcu::rcu();

    {
      Lock_guard<typeof(rgp->_lock)> guard(&rgp->_lock);
      if (rgp->_current != rgp->_completed)
	rgp->cpu_quiet(_cpu);
    }

  current_rdp->move_batch(_c);
  current_rdp->move_batch(_n);
  current_rdp->move_batch(_d);
}

PUBLIC
bool FIASCO_WARN_RESULT
Rcu_data::process_callbacks(Rcu_glbl *rgp)
{
  LOG_TRACE("Rcu callbacks", "rcu", ::current(), __fmt_rcu,
      Rcu::Log_rcu *p = tbe->payload<Rcu::Log_rcu>();
      p->cpu = _cpu;
      p->item = 0;
      p->event = Rcu::Rcu_process);

  if (_c.full() && !(rgp->_completed < _batch))
    _d.append(_c);

  if (_n.full() && _c.empty())
    {
	{
	  Lock_guard<Cpu_lock> guard(&cpu_lock);
	  _c = _n;
	  _n.clear();
	}

      // start the next batch of callbacks

      _batch = rgp->_current + 1;
      Mem::mp_rmb();

      if (!rgp->_next_pending)
	{
	  // start the batch and schedule start if it's a new batch
	  Lock_guard<typeof(rgp->_lock)> guard(&rgp->_lock);
	  rgp->_next_pending = 1;
	  rgp->start_batch();
	}
    }

  check_quiescent_state(rgp);
  if (_d.full())
    return do_batch();

  return false;
}

PUBLIC inline
bool
Rcu_data::pending(Rcu_glbl *rgp)
{
  // The CPU has pending RCU callbacks and the grace period for them
  // has been completed.
  if (_c.full() && rgp->_completed >= _batch)
    return 1;

  // The CPU has no pending RCU callbacks, however there are new callbacks
  if (_c.empty() && _n.full())
    return 1;

  // The CPU has callbacks to be invoked finally
  if (_d.full())
    return 1;

  // RCU waits for a quiescent state from the CPU
  if ((_q_batch != rgp->_current) || _pending)
    return 1;

  // OK, no RCU work to do
  return 0;

}

PUBLIC static inline NEEDS["globals.h"]
bool FIASCO_WARN_RESULT
Rcu::process_callbacks()
{ return _rcu_data.cpu(current_cpu()).process_callbacks(&_rcu); }

PUBLIC static inline NEEDS["globals.h"]
bool FIASCO_WARN_RESULT
Rcu::process_callbacks(unsigned cpu)
{ return _rcu_data.cpu(cpu).process_callbacks(&_rcu); }

PUBLIC static inline
bool
Rcu::pending(unsigned cpu)
{
  return _rcu_data.cpu(cpu).pending(&_rcu);
}


PUBLIC static inline
void
Rcu::inc_q_cnt(unsigned cpu)
{ _rcu_data.cpu(cpu)._q_passed = 1; }

PUBLIC static
void
Rcu::schedule_callbacks(unsigned cpu, Unsigned64 clock)
{
  Timeout *t = &_rcu_timeout.cpu(cpu);
  if (!t->is_set())
    t->set(clock, cpu);
}

PUBLIC static inline NEEDS["cpu_lock.h"]
Rcu::Lock *
Rcu::lock()
{ return &cpu_lock; }


PUBLIC static inline
bool
Rcu::do_pending_work(unsigned cpu)
{
  if (pending(cpu))
    {
      inc_q_cnt(cpu);
      return process_callbacks(cpu);
#if 0
      Rcu::schedule_callbacks(cpu, Kip::k()->clock + Rcu::Period);
#endif
    }
  return false;
}
