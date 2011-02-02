INTERFACE:

#include "mapping_tree.h"
#include "kobject.h"
#include "kmem_slab.h"
#include "l4_types.h"
#include "prio_list.h"
#include "thread.h"
#include "slab_cache_anon.h"

class Ram_quota;

class U_semaphore : public Kobject
{
  friend class Jdb_semaphore;

  FIASCO_DECLARE_KOBJ();

private:
  typedef slab_cache_anon Allocator;

  Ram_quota *_q;
  Locked_prio_list _queue;
  bool _valid;
public:
  enum Result { Ok, Retry, Timeout, Invalid };

public:
  virtual ~U_semaphore();
};


IMPLEMENTATION:

#include "cpu_lock.h"
#include "entry_frame.h"
#include "ipc_timeout.h"
#include "logdefs.h"
#include "mem_space.h"
#include "thread_state.h"
#include "timer.h"

FIASCO_DEFINE_KOBJ(U_semaphore);

PUBLIC /*inline*/
U_semaphore::U_semaphore(Ram_quota *q)
  : _q(q), _valid(true)
{}


PRIVATE inline NOEXPORT
void
U_semaphore::set_queued(L4_semaphore *sem, bool q)
{
  current()->mem_space()->poke_user(&(sem->flags), (Mword)q);
}


PRIVATE inline NOEXPORT
bool
U_semaphore::pagein_set_queued(Thread *c, L4_semaphore *sem, bool q)
{
  jmp_buf pf_recovery;
  int err;
  if (EXPECT_TRUE ((err = setjmp(pf_recovery)) == 0))
    {
      c->recover_jmp_buf(&pf_recovery);
      // we are preemptible here, in case of a page fault
      current()->mem_space()->poke_user(&(sem->flags), (Mword)q);
    }

  c->recover_jmp_buf(0);
  return err == 0;
}


PRIVATE inline NOEXPORT
bool
U_semaphore::add_counter(L4_semaphore *sem, long v)
{
  Smword cnt = current()->mem_space()->peek_user(&(sem->counter)) + v;
  current()->mem_space()->poke_user(&(sem->counter), cnt);
  return cnt;
}


PRIVATE inline NOEXPORT
bool
U_semaphore::valid_semaphore(L4_semaphore *s)
{
  if (EXPECT_FALSE(((unsigned long)s & (sizeof(L4_semaphore)-1)) != 0))
    return false;

  if (EXPECT_FALSE((unsigned long)s >= Mem_layout::User_max))
    return false;

  return true;
}

PUBLIC
L4_msg_tag
U_semaphore::block_locked(L4_timeout const &to, L4_semaphore *sem, Utcb *u)
{
  if (EXPECT_FALSE(!valid_semaphore(sem)))
    return L4_msg_tag(0, 0, 0, Invalid);

  Thread *c = current_thread();
  if (EXPECT_FALSE (!pagein_set_queued(c, sem, true)))
    // unhandled page fault semaphore is considered invalid
    return L4_msg_tag(0, 0, 0, Invalid);

  // *counter is now paged in writable
  if (add_counter(sem, 1) > 0) 
    {
      if (!_queue.head())
	set_queued(sem, false);

      add_counter(sem, -1);
      return L4_msg_tag(0, 0, 0, Ok);
    }

  Unsigned64 t = 0;
  if (!to.is_never())
    {
      t = to.microsecs(Timer::system_clock(), u);
      if (!t)
	return L4_msg_tag(0, 0, 0, Timeout);
    }

  c->wait_queue(&_queue);
  c->sender_enqueue(&_queue, c->sched_context()->prio());
  c->state_change_dirty(~Thread_ready, Thread_ipc_in_progress);

  IPC_timeout timeout;
  if (t)
    {
      timeout.set(t, c->cpu());
      c->set_timeout(&timeout);
    }

  c->schedule();
  // We go here by: (a) a wakeup, (b) a timeout, (c) wait_queue delete,
  // (d) ex_regs

  // The wait_queue was destroyed
  if (EXPECT_FALSE(!_valid))
    return L4_msg_tag(0, 0, 0, Invalid);

  // Two cases:
  // 1. c is not in the queue, then the wakeup already occured
  // 2. c is in the sender list an the timeout has hit a timeout is flagged
  if (EXPECT_FALSE(c->in_sender_list() && timeout.has_hit()))
    {
      // The timeout really hit so remove c from the queue
      c->sender_dequeue(&_queue);
      return L4_msg_tag(0, 0, 0, Timeout);
    }

  return L4_msg_tag(0, 0, 0, Retry);
}


PUBLIC
void
U_semaphore::wakeup_locked(L4_semaphore *sem, bool yield)
{
  if (EXPECT_FALSE(!valid_semaphore(sem)))
    return;

  Thread *c = current_thread();

  // basically make queued flag writable
  if (EXPECT_FALSE (!pagein_set_queued(c, sem, true)))
    // semaphore is invalid
    return;

  Prio_list_elem *h = _queue.head();
  if (!h)
    {
      set_queued(sem, false); // queue is empty
      return;
    }

  Thread *w = static_cast<Thread*>(Sender::cast(h));
  w->sender_dequeue(&_queue);
  w->state_change_dirty(~Thread_ipc_in_progress, Thread_ready);

  w->reset_timeout();
  w->wait_queue(0);

  if (!_queue.head())
    set_queued(sem, false); // dequeued the last thread

  // XXX: bad hack, need to sync queue
  if (w->cpu() != current_cpu())
    {
      w->activate();
      return;
    }

  if (c->schedule_in_progress())
    return;

  if (w->sched()->deblock(current_cpu(), current()->sched(), true))
    current()->switch_to_locked(w);
  else if (yield && w->sched()->prio() == current()->sched()->prio())
    {
      current()->switch_to_locked(w);
#if 0
      w->ready_enqueue();
      current()->switch_sched(current()->sched());
      current()->schedule();
#endif
    }
  else
    w->ready_enqueue();
}


IMPLEMENT
U_semaphore::~U_semaphore()
{
  _valid = false;

  while (Prio_list_elem *h = _queue.head())
    {
      Thread *w = static_cast<Thread*>(Sender::cast(h));
      w->sender_dequeue(&_queue);
      w->state_change_safely(~Thread_ipc_in_progress, Thread_ready);
      w->ready_enqueue();
      w->reset_timeout();
    }

  Lock_guard<Cpu_lock> guard(&cpu_lock);

  current()->schedule();
}

PUBLIC static
U_semaphore*
U_semaphore::alloc(Ram_quota *q)
{
  void *nq;
  if (q->alloc(sizeof(U_semaphore)) && (nq = allocator()->alloc()))
    return new (nq) U_semaphore(q);

  return 0;
}

PUBLIC
void *
U_semaphore::operator new (size_t, void *p)
{ return p; }

PUBLIC
void
U_semaphore::operator delete (void *_l)
{
  U_semaphore *l = reinterpret_cast<U_semaphore*>(_l);
  if (l->_q)
    l->_q->free(sizeof(U_semaphore));

  allocator()->free(l);
}


static Kmem_slab_t<U_semaphore> _usem_allocator("U_semaphore");

PRIVATE static
U_semaphore::Allocator *
U_semaphore::allocator()
{ return &_usem_allocator; }


PUBLIC
void
U_semaphore::invoke(L4_obj_ref, Mword, Syscall_frame *f, Utcb *u)
{
  //printf ("  do it (%p)\n", l);
  LOG_TRACE("User semaphore", "sem", ::current(), __usem_fmt,
    Log_entry *le = tbe->payload<Log_entry>();
    le->tag = f->tag().raw();
    le->id = dbg_id();
    le->sem = u->values[0]);

  switch (f->tag().proto())
    {
    case 0: //Sys_u_lock_frame::Sem_sleep:
      //LOG_MSG_3VAL(this, "USBLOCK", regs->timeout().raw(), 0, 0);
      f->tag(block_locked(f->timeout().rcv, (L4_semaphore*)u->values[0], u));
      //LOG_MSG_3VAL(this, "USBLOCK+", res, 0, 0);
      return;
    case 1: //Sys_u_lock_frame::Sem_wakeup:
      //LOG_MSG(this, "USWAKE");
      wakeup_locked((L4_semaphore*)u->values[0], true);
      f->tag(L4_msg_tag(0,0,0,0));
      return;
    case 2: //Sys_u_lock_frame::Sem_wakeup:
      //LOG_MSG(this, "USWAKE");
      wakeup_locked((L4_semaphore*)u->values[0], false);
      f->tag(L4_msg_tag(0,0,0,0));
      return;
    default:
      break;
    }

  f->tag(L4_msg_tag(0,0,0,-L4_err::EInval));
}


// -----------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class U_semaphore
{
public:
  struct Log_entry
  {
    Mword tag;
    Mword id;
    Address sem;
  };

  static unsigned log_fmt(Tb_entry *, int, char *) asm ("__usem_fmt");
;
};

// -----------------------------------------------------------------------
IMPLEMENTATION [debug]:

IMPLEMENT
unsigned
U_semaphore::log_fmt(Tb_entry *e, int maxlen, char *buf)
{
  Log_entry *le = e->payload<Log_entry>();
  char const *op;
  L4_msg_tag tag(le->tag);

  switch (tag.proto())
    {
    case 0: op = "block"; break;
    case 1: op = "signal"; break;
    default: op = "invalid"; break;
    }
  return snprintf(buf, maxlen, "sem=%lx op=%s usem=%lx", le->id,
                  op, le->sem);
}



#if 0 // Promela model of the lock
#define MAX_THREADS    4
#define MAX_WQ_ENTRIES MAX_THREADS
#define MAX 1
#define LOOPS 1

#define LOCKED 0
#define RETRY  1
#define ERROR  2

bit thread_state[MAX_THREADS];
hidden byte loops[MAX_THREADS];
unsigned in_critical : 4 = 0;
hidden byte temp;

typedef sem_t
{
  short counter;
  bit queued;
  bit queue[MAX_WQ_ENTRIES];
}

sem_t sem;  /*  maybe move init. to init*/


inline init_globals()
{
  d_step
  {
    temp = 0;
    sem.counter = MAX;
    do
      ::
        sem.queue[temp] = 0;
	temp++;
	if
	  :: (temp >= MAX_WQ_ENTRIES) -> break;
	  :: else;
	fi
    od;
  }
}

inline enqueue_thread(t)
{
  sem.queue[t] = 1;
}

inline dequeue_thread(t)
{
  sem.queue[t] = 0;
}

inline queue_head(head)
{
  local_temp = 0;
  do
    ::
      if
        :: (sem.queue[local_temp]) -> head = local_temp; break;
	:: else;
      fi;
      local_temp++;
      if
        :: (local_temp >= MAX_WQ_ENTRIES) -> head = local_temp; break;
	:: else;
      fi;
  od
}

inline block(ret, thread)
{
  do ::
  atomic
  {
    sem.counter++;
    if
      :: (sem.counter > 0) -> sem.counter--; ret = LOCKED; break;
      :: else
    fi;

    sem.queued = 1;
    enqueue_thread(thread);
    thread_state[thread] = 0;

    if
      :: (thread_state[thread] == 1) -> skip
    fi;

    if
      :: (sem.queue[thread] == 0) -> ret = RETRY; break;
      :: else;
    fi;

    dequeue_thread(thread);
    ret = ERROR;
    break;
  }
  od
}

inline wakeup()
{
  do ::
  atomic
  {
    queue_head(pf);
    if
      :: (pf == MAX_THREADS) -> sem.queued = 0; break;
      :: else;
    fi;

    dequeue_thread(pf);
    thread_state[pf] = 1;
    queue_head(pf);
    if :: (pf == MAX_THREADS) -> sem.queued = 0;
       :: else;
    fi;
    break;
  }
  od
}

inline down(ret)
{
  do
    ::
      atomic
      {
	sem.counter--;
	if
	  :: (sem.counter >= 0) -> break;
	  :: else;
	fi
      }

      block(ret, thread);

      if
        :: (ret == LOCKED || ret == ERROR) -> break;
	:: (ret == RETRY);
	:: else assert(false);
      fi
  od
}


inline up()
{
  do
    ::
      sem.counter++;
      if
        :: (!sem.queued) -> break;
        :: else;
      fi;

      wakeup();
      break;
  od
}


proctype Killer()
{
  end_k:
  do
    ::
      if
        :: (thread_state[0] == 0) -> thread_state[0] = 1;
        :: (thread_state[1] == 0) -> thread_state[1] = 1;
        :: (thread_state[2] == 0) -> thread_state[2] = 1;
      fi
  od
}

proctype Thread(byte thread)
{
  unsigned pf : 4;
  unsigned ret : 4;
  unsigned local_temp : 4;

before_down:
L1:  do
    ::
      down(ret);
      if
        :: (ret == ERROR) -> goto L1;
        :: else;
      fi;
      atomic {
      in_critical++;
      assert (in_critical <= MAX);
      }

progress1:
      in_critical--;
      up();

      if
        :: (loops[thread] == 0) -> break;
	:: else;
      fi;
      loops[thread]--;
  od
}

hidden byte threads = 0;
init
{
  threads = 0;
  in_critical = 0;
  init_globals();
  run Killer();
  do
    ::
       loops[threads] = LOOPS - 1;
       run Thread(threads);
       threads++;
       if
         :: (threads >= MAX_THREADS) -> break;
         :: else;
       fi
  od
}

/*
never
{
  do
    :: (in_critical == MAX) -> assert(false)
    :: else;
  od
}
*/

#endif 
