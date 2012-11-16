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


static void init_replica_mutex(pthread_mutex_t* mtx)
{
  unsigned i = 0;
  lock_info* li = get_lock_info();

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

	// free slot -> we can use it as we do not release
	// slots yet
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
}


int pthread_mutex_lock_rep(pthread_mutex_t * mutex);
int
attribute_hidden
pthread_mutex_lock_rep(pthread_mutex_t * mutex)
{
	/*
	 * not initialized yet? -> happens for statically initialized
	 * locks as those don't call mutex_init(). And as we need to
	 * handle this anyway, we simply don't overwrite mutex_init()
	 * at all.
	 */
	if (!mutex->__m_reserved) {
	  init_replica_mutex(mutex);
	}

	lock_info* li      = get_lock_info();
    pthread_descr self = thread_self();
    if (!self) enter_kdebug("self == NULL");

#define LOCKli(li, mtx) (li)->locks[(mtx)->__m_reserved]

    /*
     * The generated code uses ECX and EDX to access and modify data in
     * the lock info page. This page is shared between all replicas, but
     * the counts written to it may differ between replicas, which in turn
     * may lead to the master process detecting state deviation if the values
     * remain in those registers.
     *
     * To fix that, we store the original values of ECX and EDX to the stack
     * and restore them before we a) leave the function or b) perform a system
     * call that would be observed by the master.
     * 
     * XXX: Ben points out that this only works if all the code
     *      below does not use any (%esp)-indirect addresses.
     *      I validated this for now. A proper solution would be
     *      to store these registers to some replica-private page.
     *
     * XXX: Furthermore, this assumes that we will not cause a page fault or
     *      any other exception during execution, because then we might end
     *      up with differing register values as well.
     */
	asm volatile ("push %ecx\t\n"
	              "push %edx\t\n");

retry:
	lock_li(li, mutex->__m_reserved);
	
	if (LOCKli(li, mutex).owner == lock_unowned) {
		/*
		 * Case 1: The lock was previously unlocked.
		 *   -> make ourselves the lock owner
		 *   -> set acq_count to number of replicas
		 *      (it is decremented in unlock())
		 */
		LOCKli(li, mutex).owner     = (l4_addr_t)self;
		LOCKli(li, mutex).acq_count = li->replica_count;
        asm volatile ("" : : : "memory");
	} else if (LOCKli(li, mutex).owner != (l4_addr_t)self) {
		/*
		 * Case 2: someone else owns the lock
		 *   -> best we can do is go to sleep
		 *   -> XXX: maybe spinning with thread_yield
		 *      would help even more?
		 */
		LOCKli(li, mutex).wait_count += 1;
		unlock_li(li, mutex->__m_reserved);
        
		/*
		 * Go to sleep. This is a system call and will be checked by the master. Therefore,
		 * we need to load the ECX and EDX values we pushed in the beginning, so that the
		 * master process sees a consistent state here.
		 */
        asm volatile ("pop %%edx\t\n"
                      "pop %%ecx\t\n"
                      "push %0\t\n"
                      "mov $0xA020, %%eax\t\n"
                      "call *%%eax\t\n"
                      "pop %0\t\n"
                      "push %%ecx\t\n"
                      "push %%edx\t\n"
                      : : "r" (mutex) : "eax", "memory");

		/*
		 * If we return from the call above, the previous lock
		 * owner signalled us. The locking protocol makes sure that
		 * the lock is not marked as unlocked, but instead appears
		 * to still belong to the old owner.
		 *
		 * If we are the first replica to exit the call
		 * (wait_count == replica count), we adjust the lock owner
		 * to be ourselves.
		 */
        lock_li(li, mutex->__m_reserved);

        LOCKli(li, mutex).wait_count   -= 1;
        
        if (LOCKli(li, mutex).wake_count == li->replica_count) {
	        LOCKli(li, mutex).owner     = (l4_addr_t)self;
			LOCKli(li, mutex).acq_count = li->replica_count;
		}

        LOCKli(li, mutex).wake_count   -= 1;

        asm volatile ("" : : : "memory");
        unlock_li(li, mutex->__m_reserved);
	} else if (LOCKli(li, mutex).owner == (l4_addr_t)self) {
		/*
		 * Case 3: my thread group owns the lock,
		 *         but i'm not the first to acquire it
		 */

		/* Not so good case: If the wake count is larger than 0, this means that
		 * the current thread previously had acquired the lock, then called unlock,
		 * but not all replicas reached the end of unlock() yet. In this case a
		 * fast replica might already try to grab the lock again and find out that
		 * it is already the lock owner. In this case, the thread must not grab the
		 * lock, but instead release the CPU and retry at a later point in time.
		 */
		if (LOCKli(li, mutex).wake_count > 0) {
			unlock_li(li, mutex->__m_reserved);
			asm volatile ("ud2" : : : "memory");
			goto retry;
		}
	}

	unlock_li(li, mutex->__m_reserved);

#if 1
	asm volatile ("pop %edx\t\n"
	              "pop %ecx\t\n");
#endif

#undef LOCKli

	//enter_kdebug("acquired lock");

	return 0;
}


int pthread_mutex_unlock_rep(pthread_mutex_t * mutex);

int
attribute_hidden
pthread_mutex_unlock_rep(pthread_mutex_t * mutex)
{
#define LOCKli(li, mtx) (li)->locks[(mtx)->__m_reserved]

	lock_info* li      = get_lock_info();

    /*
     * See documentation at the beginning of pthread_mutex_lock_rep()
     */
	asm volatile ("push %ecx\t\n"
	              "push %edx\t\n");

	lock_li(li, mutex->__m_reserved);

	LOCKli(li, mutex).acq_count -= 1;

	/*
	 * All replicas are required to decrement the lock acquisition count. However,
	 * only the last replica to do so will actually wake up a sleeping thread.
	 */
	if (LOCKli(li, mutex).acq_count == 0) {

		// 1. send unlock notification if there are waiting threads
        if (LOCKli(li, mutex).wait_count > 0) {

			/*
			 * The wake count is used to figure out how many replicas will
			 * exit their sleep() inside mutex_lock_rep(). See there.
			 */	        
	        LOCKli(li, mutex).wake_count = li->replica_count;

			/*
			 * Send the actual notification. This is a special case in the master,
			 * because here only one replica performs the system call while all
			 * others continue untouched.
			 */
            asm volatile ("push %0\t\n"
                          "mov $0xA040, %%eax\t\n"
                          "call *%%eax\t\n"
                          "pop %0\t\n": : "r" (mutex) : "eax");
            /* Not resetting the owner here. We want to prevent other
             * threads from grabbing the lock and instead leave the lock
             * to the thread we just signalled. Therefore, this thread
             * will need to set ownership directly.
             */
        } else {
	        // no waiters -> lock is free now
            LOCKli(li, mutex).owner = lock_unowned;
        }

        asm volatile ("" : : : "memory");
	}

	unlock_li(li, mutex->__m_reserved);

	asm volatile ("pop %edx\t\n"
	              "pop %ecx\t\n");

#undef LOCKli

	return 0;
}


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
