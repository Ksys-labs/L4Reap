INTERFACE [sched_fixed_prio || sched_fp_wfq]:

#include "member_offs.h"
#include "types.h"
#include "globals.h"


template<typename E>
class Ready_queue_fp
{
  friend class Jdb_thread_list;

private:
  unsigned prio_highest;
  E *prio_next[256];

public:
  void set_idle(E *sc)
  { E::fp_elem(sc)->_prio = Config::kernel_prio; }

  void enqueue(E *, bool);
  void dequeue(E *);
  E *next_to_run() const;
};


// ---------------------------------------------------------------------------
IMPLEMENTATION [sched_fixed_prio || sched_fp_wfq]:

#include <cassert>
#include "cpu_lock.h"
#include "kdb_ke.h"
#include "std_macros.h"
#include "config.h"


IMPLEMENT inline
template<typename E>
E *
Ready_queue_fp<E>::next_to_run() const
{ return prio_next[prio_highest]; }

/**
 * Enqueue context in ready-list.
 */
IMPLEMENT
template<typename E>
void
Ready_queue_fp<E>::enqueue(E *i, bool is_current_sched)
{
  assert_kdb(cpu_lock.test());

  // Don't enqueue threads which are already enqueued
  if (EXPECT_FALSE (i->in_ready_list()))
    return;

  unsigned short prio = E::fp_elem(i)->prio();

  if (prio > prio_highest)
    prio_highest = prio;

  if (!prio_next[prio])
    prio_next[prio] = E::fp_elem(i)->_ready_next = E::fp_elem(i)->_ready_prev = i;

  else
    {
      E::fp_elem(i)->_ready_next = prio_next[prio];
      E::fp_elem(i)->_ready_prev = E::fp_elem(prio_next[prio])->_ready_prev;
      E::fp_elem(prio_next[prio])->_ready_prev = E::fp_elem(E::fp_elem(i)->_ready_prev)->_ready_next = i;

      // Special care must be taken wrt. the position of current() in the ready
      // list. Logically current() is the next thread to run at its priority.
      if (is_current_sched)
        prio_next[prio] = i;
    }
}

/**
 * Remove context from ready-list.
 */
IMPLEMENT inline NEEDS ["cpu_lock.h", "kdb_ke.h", "std_macros.h"]
template<typename E>
void
Ready_queue_fp<E>::dequeue(E *i)
{
  assert_kdb (cpu_lock.test());

  // Don't dequeue threads which aren't enqueued
  if (EXPECT_FALSE (!i->in_ready_list()))
    return;

  unsigned short prio = E::fp_elem(i)->prio();

  if (prio_next[prio] == i)
    prio_next[prio] = E::fp_elem(i)->_ready_next == i ? 0 : E::fp_elem(i)->_ready_next;

  E::fp_elem(E::fp_elem(i)->_ready_prev)->_ready_next = E::fp_elem(i)->_ready_next;
  E::fp_elem(E::fp_elem(i)->_ready_next)->_ready_prev = E::fp_elem(i)->_ready_prev;
  E::fp_elem(i)->_ready_next = 0;				// Mark dequeued

  while (!prio_next[prio_highest] && prio_highest)
    prio_highest--;
}


PUBLIC inline
template<typename E>
void
Ready_queue_fp<E>::requeue(E *i)
{
  if (!i->in_ready_list())
    enqueue(i, false);

  // rotate ready list
  prio_next[E::fp_elem(i)->prio()] = E::fp_elem(i)->_ready_next;
}
