INTERFACE:

#include "l4_types.h"
#include "per_cpu_data.h"

/** A timeout basic object. It contains the necessary queues and handles
    enqueuing, dequeuing and handling of timeouts. Real timeout classes
    should overwrite expired(), which will do the real work, if an
    timeout hits.
 */
class Timeout
{
  friend class Jdb_timeout_list;
  friend class Jdb_list_timeouts;
  friend class Timeout_q;

public:
  /**
   * Timeout constructor.
   */
  Timeout();


  void reset();

  /**
   * Check if timeout is set.
   */
  bool is_set();

  /**
   * Check if timeout has hit.
   */
  bool has_hit();

  void set(Unsigned64 clock, unsigned cpu);

  void set_again(unsigned cpu);


  /**
   * Return remaining time of timeout.
   */
  Signed64 get_timeout(Unsigned64 clock);

  /**
   * Dequeue an expired timeout.
   * @return true if a reschedule is necessary, false otherwise.
   */
  bool dequeue(bool is_expired = true);

protected:
  /**
   * Absolute system time we want to be woken up at.
   */
  Unsigned64 _wakeup;

private:
  /**
   * Default copy constructor (is undefined).
   */
  Timeout(const Timeout&);



  /**
   * Overwritten timeout handler function.
   * @return true if a reschedule is necessary, false otherwise.
   */
  virtual bool expired();

  struct {
    bool     set  : 1;
    bool     hit  : 1;
    unsigned res  : 6; // performance optimization
  } _flags;

  /**
   * Next/Previous Timeout in timer list
   */
  Timeout *_next, *_prev;

};


class Timeout_q
{
  friend class Timeout;
private:
  /**
   * Timeout queue count (2^n) and  distance between two queues in 2^n.
   */
  enum {
    Wakeup_queue_count	  = 8,
    Wakeup_queue_distance = 12 // i.e. (1<<12)us
  };

  /**
   * The timeout queues.
   */
  Timeout _q[Wakeup_queue_count];

  /**
   * The current programmed timeout.
   */
  Unsigned64 _current;
  Unsigned64 _old_clock;


public:
  /**
   * Enqueue a new timeout.
   */
  void enqueue(Timeout *to);

  Timeout* first(int index = 0);

  /**
   * Handles the timeouts, i.e. call expired() for the expired timeouts
   * and programs the "oneshot timer" to the next timeout.
   * @return true if a reschedule is necessary, false otherwise.
   */
  bool do_timeouts();

public:
  static Per_cpu<Timeout_q> timeout_queue;
};

IMPLEMENTATION:

#include <cassert>
#include "cpu_lock.h"
#include "kip.h"
#include "lock_guard.h"
#include "timer.h"
#include "static_init.h"
#include <climits>
#include "config.h"
#include "kdb_ke.h"


Per_cpu<Timeout_q> Timeout_q::timeout_queue DEFINE_PER_CPU;


IMPLEMENT inline
Timeout*
Timeout_q::first(int index)
{ return _q + (index & (Wakeup_queue_count-1)); }



/* Hazelnut uses an unsortet queue, this is fast in enqueuing and dequeue,
   but slow in finding the next programmable timeout.
*/
IMPLEMENT inline NEEDS["timer.h", "config.h"]
void
Timeout_q::enqueue(Timeout *to)
{
  int queue = (to->_wakeup >> Wakeup_queue_distance) & (Wakeup_queue_count-1);

  to->_flags.set = 1;

  Timeout *tmp = first(queue);

  while (tmp->_next != first(queue+1) && tmp->_next->_wakeup < to->_wakeup)
    tmp = tmp->_next;

  to->_next = tmp->_next;
  tmp->_next = to;

  to->_prev = tmp;
  to->_next->_prev = to;


  if (Config::scheduler_one_shot && (to->_wakeup <= _current))
    {
      _current = to->_wakeup;
      Timer::update_timer(_current);
    }
}


IMPLEMENT inline
Timeout::Timeout()
{
  _flags.set  = _flags.hit = 0;
  _flags.res  = 0;
}


/**
 * Initializes an timeout object.
 */
PUBLIC inline  NEEDS[<climits>]
void
Timeout::init()
{
  _next = _prev = this;
  _wakeup = ULONG_LONG_MAX;
}

/* Yeah, i know, an derived and specialized timeout class for
   the root node would be nicer. I already had done this, but
   it was significantly slower than this solution */
IMPLEMENT virtual bool
Timeout::expired()
{
  kdb_ke("Wakeup List Terminator reached");
  return false;
}

IMPLEMENT inline
bool
Timeout::is_set()
{
  return _flags.set;
}

IMPLEMENT inline
bool
Timeout::has_hit()
{
  return _flags.hit;
}


IMPLEMENT inline NEEDS [<cassert>, "cpu_lock.h", "lock_guard.h",
			Timeout_q::enqueue, Timeout::is_set]
void
Timeout::set(Unsigned64 clock, unsigned cpu)
{
  // XXX uses global kernel lock
  Lock_guard<Cpu_lock> guard (&cpu_lock);

  assert (!is_set());

  _wakeup = clock;
  Timeout_q::timeout_queue.cpu(cpu).enqueue(this);
}

IMPLEMENT inline
Signed64
Timeout::get_timeout(Unsigned64 clock)
{
  return _wakeup - clock;
}

IMPLEMENT inline NEEDS [<cassert>, "cpu_lock.h", "lock_guard.h",
                        Timeout::is_set, Timeout_q::enqueue, Timeout::has_hit]
void
Timeout::set_again(unsigned cpu)
{
  // XXX uses global kernel lock
  Lock_guard<Cpu_lock> guard (&cpu_lock);

  assert(! is_set());
  if (has_hit())
    return;

  Timeout_q::timeout_queue.cpu(cpu).enqueue(this);
}

IMPLEMENT inline NEEDS ["cpu_lock.h", "lock_guard.h", "timer.h",
			"kdb_ke.h", Timeout::is_set]
void
Timeout::reset()
{
  assert_kdb (cpu_lock.test());
  if (EXPECT_FALSE(!is_set()))
    return;			// avoid lock overhead if not set

  _next->_prev = _prev;
  _prev->_next = _next;

  _flags.set = 0;

  // Normaly we should reprogramm the timer in one shot mode
  // But we let the timer interrupt handler to do this "lazily", to save cycles
}

IMPLEMENT inline
bool
Timeout::dequeue(bool is_expired)
{
  // XXX assume we run kernel-locked

  _next->_prev = _prev;
  _prev->_next = _next;

  if (is_expired)
    _flags.hit = 1;

  _flags.set = 0;

  return is_expired ? expired() : false;
}

IMPLEMENT inline NEEDS [<cassert>, <climits>, "kip.h", "timer.h", "config.h",
			Timeout::dequeue]
bool
Timeout_q::do_timeouts()
{
  bool reschedule = false;

  // We test if the time between 2 activiations of this function is greater
  // than the distance between two timeout queues.
  // To avoid to lose timeouts, we calculating the missed queues and
  // scan them too.
  // This can only happen, if we dont enter the timer interrupt for a long
  // time, usual with one shot timer.
  // Because we initalize old_dequeue_time with zero,
  // we can get an "miss" on the first timerinterrupt.
  // But this is booting the system, which is uncritical.

  // Calculate which timeout queues needs to be checked.
  int start = (_old_clock >> Wakeup_queue_distance);
  int diff  = (Kip::k()->clock >> Wakeup_queue_distance) - start;
  int end   = (start + diff + 1) & (Wakeup_queue_count - 1);

  // wrap around
  start = start & (Wakeup_queue_count - 1);

  // test if an complete miss
  if (diff >= Wakeup_queue_count)
    start = end = 0; // scan all queues

  // update old_clock for the next run
  _old_clock = Kip::k()->clock;

  // ensure we always terminate
  assert((end >= 0) && (end < Wakeup_queue_count));

  for (;;)
    {
      Timeout *timeout = first(start)->_next;

      // now scan this queue for timeouts below current clock
      while ((timeout != first(start+1)))
	{
	  Timeout *tmp = timeout->_next;

	  if (!timeout->_wakeup || timeout->_wakeup > (Kip::k()->clock))
	    break;


	  if (timeout->dequeue())
	    reschedule = true;

	  timeout = tmp;
	}

      // next queue
      start = (start + 1) & (Wakeup_queue_count - 1);

      if (start == end)
	break;
    }

  if (Config::scheduler_one_shot)
    {
      // scan all queues for the next minimum
      //_current = (Unsigned64) ULONG_LONG_MAX;
      _current = Kip::k()->clock + 10000; //ms
      bool update_timer = true;

      for (int i = 0; i < Wakeup_queue_count; i++)
	{
	  // make sure that something enqueued other than the dummy element
	  if (first(i)->_next == first(i+1))
	    continue;

	  update_timer = true;

	  if (first(i)->_next->_wakeup < _current)
	    _current =  first(i)->_next->_wakeup;
	}

      if (update_timer)
	Timer::update_timer(_current);

    }
  return reschedule;
}


PUBLIC inline
bool Timeout_q::is_root_node(Address addr)
{
  if ((addr >= (Address) &_q) && (addr <= (Address)&_q + sizeof(_q)))
    return true;
  return false;
}


PUBLIC inline NEEDS[Timeout_q::first]
Timeout_q::Timeout_q()
: _current(ULONG_LONG_MAX), _old_clock(0)
{
  for (int i=0; i< Wakeup_queue_count; i++)
    {
      Timeout *t = _q + i;
      t->_next = first(i+1);
      t->_prev = first(i-1);
      t->_wakeup =  0;
    }
}

