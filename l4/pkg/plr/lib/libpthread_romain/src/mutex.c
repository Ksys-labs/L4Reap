/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Mutexes */

#include "descr.h"
#include <bits/libc-lock.h>
#include <errno.h>
#ifdef NOT_FOR_L4
#include <sched.h>
#endif
#include <stddef.h>
#include <limits.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

#include <l4/util/atomic.h>
#include <l4/plr/pthread_rep.h>
#include <l4/plr/measurements.h>


int pthread_mutex_lock_rep(pthread_mutex_t * mutex);
int pthread_mutex_unlock_rep(pthread_mutex_t * mutex);

lock_info* __lip_address = LOCK_INFO_ADDR;


#define LOCKli(li, mtx) (li)->locks[(mtx)->__m_reserved]
#define ACQ(li, mtx)    lock_li(  (li), (mtx)->__m_reserved)
#define REL(li, mtx)    unlock_li((li), (mtx)->__m_reserved)

#define YIELD()  yield() 
#define BARRIER() asm volatile ("" : : : "memory");

#define GO_TO_SLEEP 0

/*
 * The generated code uses registers to access and modify data in
 * the lock info page. This page is shared between all replicas, but
 * the counts written to it may differ between replicas, which in turn
 * may lead to the master process detecting state deviation if the values
 * remain in those registers.
 *
 * To fix that, we store the original values of EBX, ECX, and EDX in
 * thread-private storage and restore them before we a) leave the function or
 * b) perform a system call that would be observed by the master.
 *
 * XXX: This assumes that we will not cause a page fault or
 *      any other exception during execution, because then we might end
 *      up with differing register values as well.
 */
static inline void rep_function_save_regs(void)
{
#if 0
  BARRIER();
  asm volatile ("mov %%ebx, %0\t\n"
                "mov %%ecx, %1\t\n"
                "mov %%edx, %2\t\n"
                /*
                "mov %%esi, %3\t\n"
                "mov %%edi, %4\t\n"*/
                : "=m" (thread_self()->ebx),
                  "=m" (thread_self()->ecx),
                  "=m" (thread_self()->edx)/*,
                  "=m" (thread_self()->esi),
                  "=m" (thread_self()->edi)*/
                :
                : "memory"
  );
  BARRIER();
#endif
}


static inline void rep_function_restore_regs(void)
{
  BARRIER();
#if 0
  asm volatile ("mov %0, %%ebx\t\n"
                "mov %1, %%ecx\t\n"
                "mov %2, %%edx\t\n"
                /*
                "mov %3, %%esi\t\n"
                "mov %4, %%edi\t\n"*/
                :
                : "m" (thread_self()->ebx),
                  "m" (thread_self()->ecx),
                  "m" (thread_self()->edx)/*,
                  "m" (thread_self()->esi),
                  "m" (thread_self()->edi)*/
                : "memory"
  );
#else
  asm volatile ("mov $0, %%ebx\t\n"
                "mov $0, %%ecx\t\n"
                "mov $0, %%edx\t\n"
                ::: "memory");
#endif
  BARRIER();
}

#ifdef PT_SOLO
#define yield stall
#else
static inline void yield()
{
	rep_function_restore_regs();
	asm volatile ("ud2" : : : "edx", "ecx", "ebx", "memory");
	rep_function_save_regs();
}
#endif


static inline void lock_rep_wait(pthread_mutex_t* mutex)
{
    /*
     * Go to sleep. This is a system call and will be checked by the master. Therefore,
     * we need to load the ECX and EDX values we pushed in the beginning, so that the
     * master process sees a consistent state here.
     */
    rep_function_restore_regs();
    asm volatile (
                  "push %0\t\n"
                  "mov $0xA020, %%eax\t\n"
                  "call *%%eax\t\n"
                  "pop %0\t\n"
                  :
                  : "r" (mutex)
                  : "eax", "memory");
    rep_function_save_regs();
}



static inline void lock_rep_post(pthread_mutex_t* mutex)
{
    /*
     * Send the actual notification. This is a special case in the master,
     * because here only one replica performs the system call while all
     * others continue untouched.
     */
    BARRIER();
    asm volatile ("push %0\t\n"
                  "mov $0xA040, %%eax\t\n"
                  "call *%%eax\t\n"
                  "pop %0\t\n": : "r" (mutex) : "eax", "memory");
}


static void init_replica_mutex(pthread_mutex_t* mtx)
{
  unsigned i = 0;
  lock_info* li = get_lock_info();

  rep_function_save_regs();
  
  /*
   * find either the respective lock (if it has been registered by another
   * replica yet) or a free slot to use
   */
  for (i = 0; i < NUM_LOCKS; ++i) {

	lock_li(li, i);

	// slot already acquired by another replica
	if (li->locks[i].lockdesc == (l4_addr_t)mtx) {
	  mtx->__m_reserved = i;
	  unlock_li(li, i);
	  break;
	}

	// free slot -> we can use it
	if (li->locks[i].owner == lock_entry_free) {
	  li->locks[i].lockdesc = (l4_addr_t)mtx;
	  li->locks[i].owner    = lock_unowned;
	  mtx->__m_reserved     = i;
	  unlock_li(li, i);
	  break;
	}

	unlock_li(li, i);
  }

  if (i >= NUM_LOCKS) {
	  enter_kdebug("out of locks");
  }
  
  rep_function_restore_regs();
}

#if 0
#define EVENT(_mtx, _type, _data1, _data2) \
do { \
  char *evb = evbuf_get_address(); \
  struct GenericEvent *ev = evbuf_next(evb); \
  ev->header.tsc          = evbuf_get_time(evb, 1); \
  ev->header.vcpu         = (unsigned)thread_self(); \
  ev->header.type         = 9; \
  ev->data.shml.lockid    = (_mtx)->__m_reserved; \
  ev->data.shml.type      = _type; \
  ev->data.shml.epoch     = _data1; \
  ev->data.shml.owner     = _data2; \
} while (0)
#else
#define EVENT(_mtx, _type, _data1, _data2) do {} while(0)
#endif

int
attribute_hidden
pthread_mutex_lock_rep(pthread_mutex_t * mutex)
{

  rep_function_save_regs();
  
	/*
	 * not initialized yet? -> happens for statically initialized
	 * locks as those don't call mutex_init(). And as we need to
	 * handle this anyway, we simply don't overwrite mutex_init()
	 * at all.
	 */
	if (!mutex->__m_reserved) {
	  init_replica_mutex(mutex);
	}

  lock_info* li            = get_lock_info();
  thread_self()->p_epoch  += 1;
  
  ACQ(li, mutex);
  EVENT(mutex, 2, thread_self()->p_epoch, LOCKli(li, mutex).owner);
  REL(li, mutex);
  
  /*outstring("lock() "); outhex32(thread_self()->p_epoch); outstring("\n");*/

  while (1) {

	ACQ(li, mutex);
  
    if (LOCKli(li, mutex).owner == lock_unowned)
    {
      LOCKli(li, mutex).owner       = (l4_addr_t)thread_self();
      LOCKli(li, mutex).owner_epoch = thread_self()->p_epoch;
      LOCKli(li, mutex).acq_count   = li->replica_count;
      break;
    }
    else if (LOCKli(li, mutex).owner == (l4_addr_t)thread_self())
    {
      if (LOCKli(li, mutex).owner_epoch != thread_self()->p_epoch) {
        //outchar42('.'); outchar42(' '); outhex42(thread_self());
        REL(li, mutex);
        YIELD();
        continue;
      }

      break;
    }
    else
    {
      REL(li, mutex);
      YIELD();
      continue;
    }
  }

  EVENT(mutex, 3, thread_self()->p_epoch, 0);

  REL(li, mutex);

  rep_function_restore_regs();

	return 0;
}


int pthread_mutex_unlock_rep(pthread_mutex_t * mutex);

int
attribute_hidden
pthread_mutex_unlock_rep(pthread_mutex_t * mutex)
{
  rep_function_save_regs();

  lock_info *li = get_lock_info();
  
  ACQ(li, mutex);
  EVENT(mutex, 4, LOCKli(li, mutex).acq_count, 0);
  
  LOCKli(li, mutex).acq_count -= 1;
  if (LOCKli(li, mutex).acq_count == 0) {
      LOCKli(li, mutex).owner = lock_unowned;
  }

  EVENT(mutex, 5, thread_self()->p_epoch, LOCKli(li, mutex).owner);

  REL(li, mutex);

  rep_function_restore_regs();

  return 0;
}

#undef LOCKli

int
attribute_hidden
__pthread_mutex_init(pthread_mutex_t * mutex,
                       const pthread_mutexattr_t * mutex_attr)
{
  __pthread_init_lock(&mutex->__m_lock);
  mutex->__m_kind =
    mutex_attr == NULL ? PTHREAD_MUTEX_TIMED_NP : mutex_attr->__mutexkind;
  mutex->__m_count = 0;
  mutex->__m_owner = NULL;
  return 0;
}
strong_alias (__pthread_mutex_init, pthread_mutex_init)

int
attribute_hidden
__pthread_mutex_destroy(pthread_mutex_t * mutex)
{
  switch (mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if ((mutex->__m_lock.__status & 1) != 0)
      return EBUSY;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
  case PTHREAD_MUTEX_TIMED_NP:
    if (mutex->__m_lock.__status != 0)
      return EBUSY;
    return 0;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_destroy, pthread_mutex_destroy)

int
attribute_hidden
__pthread_mutex_trylock(pthread_mutex_t * mutex)
{
  pthread_descr self;
  int retcode;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    retcode = __pthread_trylock(&mutex->__m_lock);
    return retcode;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    retcode = __pthread_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = self;
      mutex->__m_count = 0;
    }
    return retcode;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    retcode = __pthread_alt_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = thread_self();
    }
    return retcode;
  case PTHREAD_MUTEX_TIMED_NP:
    retcode = __pthread_alt_trylock(&mutex->__m_lock);
    return retcode;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)

int
attribute_hidden
__pthread_mutex_lock(pthread_mutex_t * mutex)
{
  pthread_descr self;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
#if 0
    __pthread_lock(&mutex->__m_lock, NULL);
#else
    pthread_mutex_lock_rep(mutex);
#endif
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
#if 0
    __pthread_lock(&mutex->__m_lock, self);
#else
    pthread_mutex_lock_rep(mutex);
#endif
    mutex->__m_owner = self;
    mutex->__m_count = 0;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->__m_owner == self) return EDEADLK;
#if 0
    __pthread_alt_lock(&mutex->__m_lock, self);
#else
    pthread_mutex_lock_rep(mutex);
#endif
    mutex->__m_owner = self;
    return 0;
  case PTHREAD_MUTEX_TIMED_NP:
#if 0
    __pthread_alt_lock(&mutex->__m_lock, NULL);
#else
    pthread_mutex_lock_rep(mutex);
#endif
    return 0;
  default:
    return EINVAL;
  }
}

strong_alias (__pthread_mutex_lock, pthread_mutex_lock)


int
attribute_hidden
__pthread_mutex_timedlock (pthread_mutex_t *mutex,
			       const struct timespec *abstime)
{
  pthread_descr self;
  int res;

  if (__builtin_expect (abstime->tv_nsec, 0) < 0
      || __builtin_expect (abstime->tv_nsec, 0) >= 1000000000)
    return EINVAL;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    __pthread_lock(&mutex->__m_lock, NULL);
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    __pthread_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    mutex->__m_count = 0;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->__m_owner == self) return EDEADLK;
    res = __pthread_alt_timedlock(&mutex->__m_lock, self, abstime);
    if (res != 0)
      {
	mutex->__m_owner = self;
	return 0;
      }
    return ETIMEDOUT;
  case PTHREAD_MUTEX_TIMED_NP:
    /* Only this type supports timed out lock. */
    return (__pthread_alt_timedlock(&mutex->__m_lock, NULL, abstime)
	    ? 0 : ETIMEDOUT);
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_timedlock, pthread_mutex_timedlock)

int
attribute_hidden
__pthread_mutex_unlock(pthread_mutex_t * mutex)
{
  switch (mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
#if 0
    __pthread_unlock(&mutex->__m_lock);
#else
    pthread_mutex_unlock_rep(mutex);
#endif
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->__m_owner != thread_self())
      return EPERM;
    if (mutex->__m_count > 0) {
      mutex->__m_count--;
      return 0;
    }
    mutex->__m_owner = NULL;
#if 0
    __pthread_unlock(&mutex->__m_lock);
#else
    pthread_mutex_unlock_rep(mutex);
#endif
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    if (mutex->__m_owner != thread_self() || mutex->__m_lock.__status == 0)
      return EPERM;
    mutex->__m_owner = NULL;
#if 0
    __pthread_alt_unlock(&mutex->__m_lock);
#else
    pthread_mutex_unlock_rep(mutex);
#endif
    return 0;
  case PTHREAD_MUTEX_TIMED_NP:
#if 0
    __pthread_alt_unlock(&mutex->__m_lock);
#else
    pthread_mutex_unlock_rep(mutex);
#endif
    return 0;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)

int
attribute_hidden
__pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  attr->__mutexkind = PTHREAD_MUTEX_TIMED_NP;
  return 0;
}
strong_alias (__pthread_mutexattr_init, pthread_mutexattr_init)

int
attribute_hidden
__pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  return 0;
}
strong_alias (__pthread_mutexattr_destroy, pthread_mutexattr_destroy)

int
attribute_hidden
__pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
  if (kind != PTHREAD_MUTEX_ADAPTIVE_NP
      && kind != PTHREAD_MUTEX_RECURSIVE_NP
      && kind != PTHREAD_MUTEX_ERRORCHECK_NP
      && kind != PTHREAD_MUTEX_TIMED_NP)
    return EINVAL;
  attr->__mutexkind = kind;
  return 0;
}
weak_alias (__pthread_mutexattr_settype, pthread_mutexattr_settype)
strong_alias ( __pthread_mutexattr_settype, __pthread_mutexattr_setkind_np)
weak_alias (__pthread_mutexattr_setkind_np, pthread_mutexattr_setkind_np)

int
attribute_hidden
__pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *kind)
{
  *kind = attr->__mutexkind;
  return 0;
}
weak_alias (__pthread_mutexattr_gettype, pthread_mutexattr_gettype)
strong_alias (__pthread_mutexattr_gettype, __pthread_mutexattr_getkind_np)
weak_alias (__pthread_mutexattr_getkind_np, pthread_mutexattr_getkind_np)

int
attribute_hidden
__pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr,
				   int *pshared)
{
  *pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}
weak_alias (__pthread_mutexattr_getpshared, pthread_mutexattr_getpshared)

int
attribute_hidden
__pthread_mutexattr_setpshared (pthread_mutexattr_t *attr, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  /* For now it is not possible to shared a conditional variable.  */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return ENOSYS;

  return 0;
}
weak_alias (__pthread_mutexattr_setpshared, pthread_mutexattr_setpshared)

/* Once-only execution */

static pthread_mutex_t once_masterlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t once_finished = PTHREAD_COND_INITIALIZER;
static int fork_generation = 0;	/* Child process increments this after fork. */

enum { NEVER = 0, IN_PROGRESS = 1, DONE = 2 };

/* If a thread is canceled while calling the init_routine out of
   pthread once, this handler will reset the once_control variable
   to the NEVER state. */

static void pthread_once_cancelhandler(void *arg)
{
    pthread_once_t *once_control = arg;

    pthread_mutex_lock(&once_masterlock);
    *once_control = NEVER;
    pthread_mutex_unlock(&once_masterlock);
    pthread_cond_broadcast(&once_finished);
}

int
attribute_hidden
__pthread_once(pthread_once_t * once_control, void (*init_routine)(void))
{
  /* flag for doing the condition broadcast outside of mutex */
  int state_changed;

  /* Test without locking first for speed */
  if (*once_control == DONE) {
    READ_MEMORY_BARRIER();
    return 0;
  }
  /* Lock and test again */

  state_changed = 0;

  pthread_mutex_lock(&once_masterlock);

  /* If this object was left in an IN_PROGRESS state in a parent
     process (indicated by stale generation field), reset it to NEVER. */
  if ((*once_control & 3) == IN_PROGRESS && (*once_control & ~3) != fork_generation)
    *once_control = NEVER;

  /* If init_routine is being called from another routine, wait until
     it completes. */
  while ((*once_control & 3) == IN_PROGRESS) {
    pthread_cond_wait(&once_finished, &once_masterlock);
  }
  /* Here *once_control is stable and either NEVER or DONE. */
  if (*once_control == NEVER) {
    *once_control = IN_PROGRESS | fork_generation;
    pthread_mutex_unlock(&once_masterlock);
    pthread_cleanup_push(pthread_once_cancelhandler, once_control);
    init_routine();
    pthread_cleanup_pop(0);
    pthread_mutex_lock(&once_masterlock);
    WRITE_MEMORY_BARRIER();
    *once_control = DONE;
    state_changed = 1;
  }
  pthread_mutex_unlock(&once_masterlock);

  if (state_changed)
    pthread_cond_broadcast(&once_finished);

  return 0;
}
strong_alias (__pthread_once, pthread_once)

/*
 * Handle the state of the pthread_once mechanism across forks.  The
 * once_masterlock is acquired in the parent process prior to a fork to ensure
 * that no thread is in the critical region protected by the lock.  After the
 * fork, the lock is released. In the child, the lock and the condition
 * variable are simply reset.  The child also increments its generation
 * counter which lets pthread_once calls detect stale IN_PROGRESS states
 * and reset them back to NEVER.
 */

void
attribute_hidden
__pthread_once_fork_prepare(void)
{
  pthread_mutex_lock(&once_masterlock);
}

void
attribute_hidden
__pthread_once_fork_parent(void)
{
  pthread_mutex_unlock(&once_masterlock);
}

void
attribute_hidden
__pthread_once_fork_child(void)
{
  pthread_mutex_init(&once_masterlock, NULL);
  pthread_cond_init(&once_finished, NULL);
  if (fork_generation <= INT_MAX - 4)
    fork_generation += 4;	/* leave least significant two bits zero */
  else
    fork_generation = 0;
}
