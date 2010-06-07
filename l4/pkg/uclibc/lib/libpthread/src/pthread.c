
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

/* Thread creation, initialization, and basic low-level routines */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <l4/re/env.h>

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"


#if !(USE_TLS && HAVE___THREAD)
/* These variables are used by the setup code.  */
extern int _errno;
extern int _h_errno;

# if defined __UCLIBC_HAS_IPv4__ || defined __UCLIBC_HAS_IPV6__
/* We need the global/static resolver state here.  */
# include <resolv.h>
# undef _res
extern struct __res_state _res;
# endif
#endif

#ifdef USE_TLS

/* We need only a few variables.  */
#define manager_thread __pthread_manager_threadp
pthread_descr __pthread_manager_threadp attribute_hidden;

#else

/* Descriptor of the initial thread */

struct _pthread_descr_struct __pthread_initial_thread = {
  .p_header.data.self = &__pthread_initial_thread,
  .p_nextlive = &__pthread_initial_thread,
  .p_prevlive = &__pthread_initial_thread,
  .p_start_args = PTHREAD_START_ARGS_INITIALIZER(NULL),
#if !(USE_TLS && HAVE___THREAD)
  .p_errnop = &_errno,
  .p_h_errnop = &_h_errno,
# if defined __UCLIBC_HAS_IPv4__ || defined __UCLIBC_HAS_IPV6__
  .p_resp = &_res,
# endif
#endif
  .p_userstack = 1,
  .p_resume_count = __ATOMIC_INITIALIZER,
  .p_alloca_cutoff = __MAX_ALLOCA_CUTOFF
};

/* Descriptor of the manager thread; none of this is used but the error
   variables, the p_pid and p_priority fields,
   and the address for identification.  */

#define manager_thread (&__pthread_manager_thread)
struct _pthread_descr_struct __pthread_manager_thread = {
  .p_header.data.self = &__pthread_manager_thread,
  .p_header.data.multiple_threads = 1,
  .p_start_args = PTHREAD_START_ARGS_INITIALIZER(__pthread_manager),
#if !(USE_TLS && HAVE___THREAD)
  .p_errnop = &__pthread_manager_thread.p_errno,
#endif
  .p_resume_count = __ATOMIC_INITIALIZER,
  .p_alloca_cutoff = PTHREAD_STACK_MIN / 4
};
#endif

/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

#ifdef USE_TLS
pthread_descr __pthread_main_thread;
#else
pthread_descr __pthread_main_thread = &__pthread_initial_thread;
#endif

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

char *__pthread_initial_thread_bos;

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

l4_cap_idx_t __pthread_manager_request = L4_INVALID_CAP;

int __pthread_multiple_threads attribute_hidden;

/* Limits of the thread manager stack */

char *__pthread_manager_thread_bos;
char *__pthread_manager_thread_tos;

/* For process-wide exit() */

int __pthread_exit_requested;
int __pthread_exit_code;

/* Maximum stack size.  */
size_t __pthread_max_stacksize;

/* Deffault size for thread stacks */
size_t __attribute__((weak)) __pthread_default_stacksize = 32 * 1024;

/* Nozero if the machine has more than one processor.  */
int __pthread_smp_kernel;


/* Communicate relevant LinuxThreads constants to gdb */

//const int __pthread_threads_max = PTHREAD_THREADS_MAX;
const int __linuxthreads_pthread_sizeof_descr
  = sizeof(struct _pthread_descr_struct);


/* Forward declarations */

static void pthread_onexit_process(int retcode, void *arg);
#ifndef HAVE_Z_NODELETE
static void pthread_atexit_process(void *arg, int retcode);
static void pthread_atexit_retcode(void *arg, int retcode);
#endif


/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

#ifndef HAVE_Z_NODELETE
extern void *__dso_handle __attribute__ ((weak));
#endif


#if defined USE_TLS && !defined SHARED
extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
#endif

struct pthread_functions __pthread_functions =
  {
#if !(USE_TLS && HAVE___THREAD)
    .ptr_pthread_internal_tsd_set = __pthread_internal_tsd_set,
    .ptr_pthread_internal_tsd_get = __pthread_internal_tsd_get,
    .ptr_pthread_internal_tsd_address = __pthread_internal_tsd_address,
#endif
    .ptr_pthread_fork = NULL, /*__pthread_fork,*/
    .ptr_pthread_attr_destroy = __pthread_attr_destroy,
    .ptr_pthread_attr_init = __pthread_attr_init,
    .ptr_pthread_attr_getdetachstate = __pthread_attr_getdetachstate,
    .ptr_pthread_attr_setdetachstate = __pthread_attr_setdetachstate,
    .ptr_pthread_attr_getinheritsched = __pthread_attr_getinheritsched,
    .ptr_pthread_attr_setinheritsched = __pthread_attr_setinheritsched,
    .ptr_pthread_attr_getschedparam = __pthread_attr_getschedparam,
    .ptr_pthread_attr_setschedparam = __pthread_attr_setschedparam,
    .ptr_pthread_attr_getschedpolicy = __pthread_attr_getschedpolicy,
    .ptr_pthread_attr_setschedpolicy = __pthread_attr_setschedpolicy,
    .ptr_pthread_attr_getscope = __pthread_attr_getscope,
    .ptr_pthread_attr_setscope = __pthread_attr_setscope,
    .ptr_pthread_condattr_destroy = __pthread_condattr_destroy,
    .ptr_pthread_condattr_init = __pthread_condattr_init,
    .ptr_pthread_cond_broadcast = __pthread_cond_broadcast,
    .ptr_pthread_cond_destroy = __pthread_cond_destroy,
    .ptr_pthread_cond_init = __pthread_cond_init,
    .ptr_pthread_cond_signal = __pthread_cond_signal,
    .ptr_pthread_cond_wait = __pthread_cond_wait,
    .ptr_pthread_cond_timedwait = __pthread_cond_timedwait,
    .ptr_pthread_equal = __pthread_equal,
    .ptr___pthread_exit = __pthread_exit,
    .ptr_pthread_getschedparam = __pthread_getschedparam,
    .ptr_pthread_setschedparam = __pthread_setschedparam,
    .ptr_pthread_mutex_destroy = __pthread_mutex_destroy,
    .ptr_pthread_mutex_init = __pthread_mutex_init,
    .ptr_pthread_mutex_lock = __pthread_mutex_lock,
    .ptr_pthread_mutex_trylock = __pthread_mutex_trylock,
    .ptr_pthread_mutex_unlock = __pthread_mutex_unlock,
    .ptr_pthread_self = __pthread_self,
    .ptr_pthread_setcancelstate = __pthread_setcancelstate,
    .ptr_pthread_setcanceltype = __pthread_setcanceltype,
    .ptr_pthread_do_exit = __pthread_do_exit,
    .ptr_pthread_thread_self = __pthread_thread_self,
    .ptr_pthread_cleanup_upto = __pthread_cleanup_upto,
#ifdef __NOT_FOR_L4__
    .ptr_pthread_sigaction = __pthread_sigaction,
    .ptr_pthread_sigwait = __pthread_sigwait,
    .ptr_pthread_raise = __pthread_raise,
#else
    .ptr_pthread_sigaction = NULL,
    .ptr_pthread_sigwait = NULL,
    .ptr_pthread_raise = NULL,
#endif
    .ptr__pthread_cleanup_push = _pthread_cleanup_push,
    .ptr__pthread_cleanup_push_defer = _pthread_cleanup_push_defer,
    .ptr__pthread_cleanup_pop = _pthread_cleanup_pop,
    .ptr__pthread_cleanup_pop_restore = _pthread_cleanup_pop_restore,
  };
#ifdef SHARED
# define ptr_pthread_functions &__pthread_functions
#else
# define ptr_pthread_functions NULL
#endif

static int *__libc_multiple_threads_ptr;
l4_utcb_t *__pthread_first_free_handle attribute_hidden;

/* Do some minimal initialization which has to be done during the
   startup of the C library.  */
void
__pthread_initialize_minimal(void)
{
  /* initialize free list */
  l4_fpage_t utcb_area = l4re_env()->utcb_area;
  l4_addr_t free_utcb = l4re_env()->first_free_utcb;
  l4_addr_t utcbs_end = ((l4_addr_t)l4_fpage_page(utcb_area) << 12UL)
    + (1UL << (l4_addr_t)l4_fpage_size(utcb_area));
  l4_utcb_t **last_free = &__pthread_first_free_handle;
  while ((l4_addr_t)free_utcb + L4_UTCB_OFFSET <= utcbs_end)
    {
      l4_utcb_t *u = (l4_utcb_t*)free_utcb;
      l4_thread_regs_t *tcr = l4_utcb_tcr_u(u);
      tcr->user[0] = 0;
      __pthread_init_lock(handle_to_lock(u));
      *last_free = u;
      last_free = (l4_utcb_t**)(&tcr->user[0]);
      free_utcb += L4_UTCB_OFFSET;
    }

  __pthread_init_lock(handle_to_lock(l4_utcb()));

#ifdef USE_TLS
  pthread_descr self;

  /* First of all init __pthread_handles[0] and [1] if needed.  */
# ifndef SHARED
  /* Unlike in the dynamically linked case the dynamic linker has not
     taken care of initializing the TLS data structures.  */
  __libc_setup_tls (TLS_TCB_SIZE, TLS_TCB_ALIGN);
# elif !USE___THREAD
  if (__builtin_expect (GL(dl_tls_dtv_slotinfo_list) == NULL, 0))
    {
      tcbhead_t *tcbp;

      /* There is no actual TLS being used, so the thread register
	 was not initialized in the dynamic linker.  */

      /* We need to install special hooks so that the malloc and memalign
	 calls in _dl_tls_setup and _dl_allocate_tls won't cause full
	 malloc initialization that will try to set up its thread state.  */

      extern void __libc_malloc_pthread_startup (bool first_time);
      __libc_malloc_pthread_startup (true);

      if (__builtin_expect (_dl_tls_setup (), 0)
	  || __builtin_expect ((tcbp = _dl_allocate_tls (NULL)) == NULL, 0))
	{
	  static const char msg[] = "\
cannot allocate TLS data structures for initial thread\n";
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  abort ();
	}
      const char *lossage = TLS_INIT_TP (tcbp, 0);
      if (__builtin_expect (lossage != NULL, 0))
	{
	  static const char msg[] = "cannot set up thread-local storage: ";
	  const char nl = '\n';
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						msg, sizeof msg - 1));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO,
						lossage, strlen (lossage)));
	  TEMP_FAILURE_RETRY (write_not_cancel (STDERR_FILENO, &nl, 1));
	}

      /* Though it was allocated with libc's malloc, that was done without
	 the user's __malloc_hook installed.  A later realloc that uses
	 the hooks might not work with that block from the plain malloc.
	 So we record this block as unfreeable just as the dynamic linker
	 does when it allocates the DTV before the libc malloc exists.  */
      GL(dl_initial_dtv) = GET_DTV (tcbp);

      __libc_malloc_pthread_startup (false);
    }
# endif

  self = THREAD_SELF;

  /* The memory for the thread descriptor was allocated elsewhere as
     part of the TLS allocation.  We have to initialize the data
     structure by hand.  This initialization must mirror the struct
     definition above.  */
  self->p_nextlive = self->p_prevlive = self;
  self->p_tid = PTHREAD_THREADS_MAX;
  self->p_lock = &__pthread_handles[0].h_lock;
# ifndef HAVE___THREAD
  self->p_errnop = &_errno;
  self->p_h_errnop = &_h_errno;
# endif
  /* self->p_start_args need not be initialized, it's all zero.  */
  self->p_userstack = 1;
# if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
# endif
  self->p_alloca_cutoff = __MAX_ALLOCA_CUTOFF;

  /* Another variable which points to the thread descriptor.  */
  __pthread_main_thread = self;

  /* And fill in the pointer the the thread __pthread_handles array.  */
  __pthread_handles[0].h_descr = self;

#else  /* USE_TLS */

  /* If we have special thread_self processing, initialize that for the
     main thread now.  */
# ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_initial_thread, 0);
# endif
#endif

#if HP_TIMING_AVAIL
# ifdef USE_TLS
  self->p_cpuclock_offset = GL(dl_cpuclock_offset);
# else
  __pthread_initial_thread.p_cpuclock_offset = GL(dl_cpuclock_offset);
# endif
#endif
#ifndef NOT_FOR_L4
  if (__pthread_l4_initialize_main_thread(&__pthread_initial_thread))
    exit(1);

#endif

  // FIXME: need this for shared libc....
  __libc_multiple_threads_ptr = __libc_pthread_init (ptr_pthread_functions);
}


void
__pthread_init_max_stacksize(void)
{
  size_t max_stack;
  /* Play with the stack size limit to make sure that no stack ever grows
     beyond STACK_SIZE minus one page (to act as a guard page). */
  max_stack = STACK_SIZE - L4_PAGESIZE;
  __pthread_max_stacksize = max_stack;
  if (__pthread_default_stacksize > __pthread_max_stacksize)
    __pthread_default_stacksize = __pthread_max_stacksize;

  if (max_stack / 4 < __MAX_ALLOCA_CUTOFF)
    {
#ifdef USE_TLS
      pthread_descr self = THREAD_SELF;
      self->p_alloca_cutoff = max_stack / 4;
#else
      __pthread_initial_thread.p_alloca_cutoff = max_stack / 4;
#endif
    }
}

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
# if USE___THREAD
/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));
# else
static void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  return &thread_self ()->p_libc_specific[_LIBC_TSD_KEY_DL_ERROR];
}
# endif
#endif

#ifdef USE_TLS
static inline void __attribute__((always_inline))
init_one_static_tls (pthread_descr descr, struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  dtv_t *dtv = GET_DTV (descr);
  void *dest = (char *) descr - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  dtv_t *dtv = GET_DTV ((pthread_descr) ((char *) descr + TLS_PRE_TCB_SIZE));
  void *dest = (char *) descr + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (__mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

static void
__pthread_init_static_tls (struct link_map *map)
{
  size_t i;

  for (i = 0; i < PTHREAD_THREADS_MAX; ++i)
    if (__pthread_handles[i].h_descr != NULL && i != 1)
      {
        __pthread_lock (&__pthread_handles[i].h_lock, NULL);
	if (__pthread_handles[i].h_descr != NULL)
	  init_one_static_tls (__pthread_handles[i].h_descr, map);
        __pthread_unlock (&__pthread_handles[i].h_lock);
      }
}
#endif

static void pthread_initialize(void)
{
  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread_bos != NULL) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
  /* We don't need to know the bottom of the stack.  Give the pointer some
     value to signal that initialization happened.  */
  __pthread_initial_thread_bos = (void *) -1l;
#ifdef USE_TLS
  /* Update the descriptor for the initial thread. */
  THREAD_SETMEM (((pthread_descr) NULL), p_pid, __getpid());
# if !defined HAVE___THREAD && (defined __UCLIBC_HAS_IPv4__ || defined __UCLIBC_HAS_IPV6__)
  /* Likewise for the resolver state _res.  */
  THREAD_SETMEM (((pthread_descr) NULL), p_resp, &_res);
# endif
#else
#endif
  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_*exit_process. */
#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_process, NULL,
		  __dso_handle);
  else
#endif
    __on_exit (pthread_onexit_process, NULL);
  /* How many processors.  */
  __pthread_smp_kernel = 1; //is_smp_system ();

/* psm: we do not have any ld.so support yet
 *	 remove the USE_TLS guard if nptl is added */
#if defined SHARED && defined USE_TLS
  /* Transfer the old value from the dynamic linker's internal location.  */
  *__libc_dl_error_tsd () = *(*GL(dl_error_catch_tsd)) ();
  GL(dl_error_catch_tsd) = &__libc_dl_error_tsd;

  /* Make __rtld_lock_{,un}lock_recursive use pthread_mutex_{,un}lock,
     keep the lock count from the ld.so implementation.  */
  GL(dl_rtld_lock_recursive) = (void *) __pthread_mutex_lock;
  GL(dl_rtld_unlock_recursive) = (void *) __pthread_mutex_unlock;
  unsigned int rtld_lock_count = GL(dl_load_lock).mutex.__m_count;
  GL(dl_load_lock).mutex.__m_count = 0;
  while (rtld_lock_count-- > 0)
    __pthread_mutex_lock (&GL(dl_load_lock).mutex);
#endif

#ifdef USE_TLS
  GL(dl_init_static_tls) = &__pthread_init_static_tls;
#endif
}

void __pthread_initialize(void)
{
  pthread_initialize();
}

int __pthread_initialize_manager(void)
{
  pthread_descr mgr;
#ifdef USE_TLS
  tcbhead_t *tcbp;
#endif

  __pthread_multiple_threads = 1;
#if TLS_MULTIPLE_THREADS_IN_TCB || !defined USE_TLS || !TLS_DTV_AT_TP
  __pthread_main_thread->p_multiple_threads = 1;
#endif
  *__libc_multiple_threads_ptr = 1;

#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_retcode, NULL,
		  __dso_handle);
#endif

  if (__pthread_max_stacksize == 0)
    __pthread_init_max_stacksize ();
  /* If basic initialization not done yet (e.g. we're called from a
     constructor run before our constructor), do it now */
  if (__pthread_initial_thread_bos == NULL) pthread_initialize();
  /* Setup stack for thread manager */
  __pthread_manager_thread_bos = malloc(THREAD_MANAGER_STACK_SIZE);
  if (__pthread_manager_thread_bos == NULL) return -1;
  __pthread_manager_thread_tos =
    __pthread_manager_thread_bos + THREAD_MANAGER_STACK_SIZE;

#ifdef USE_TLS
  /* Allocate memory for the thread descriptor and the dtv.  */
  tcbp = _dl_allocate_tls (NULL);
  if (tcbp == NULL) {
    free(__pthread_manager_thread_bos);
    close_not_cancel(manager_pipe[0]);
    close_not_cancel(manager_pipe[1]);
    return -1;
  }

# if defined(TLS_TCB_AT_TP)
  mgr = (pthread_descr) tcbp;
# elif defined(TLS_DTV_AT_TP)
  /* pthread_descr is located right below tcbhead_t which _dl_allocate_tls
     returns.  */
  mgr = (pthread_descr) ((char *) tcbp - TLS_PRE_TCB_SIZE);
# endif
  __pthread_handles[1].h_descr = manager_thread = mgr;

  /* Initialize the descriptor.  */
#if !defined USE_TLS || !TLS_DTV_AT_TP
  mgr->p_header.data.tcb = tcbp;
  mgr->p_header.data.self = mgr;
  mgr->p_header.data.multiple_threads = 1;
#elif TLS_MULTIPLE_THREADS_IN_TCB
  mgr->p_multiple_threads = 1;
#endif
  mgr->p_lock = &__pthread_handles[1].h_lock;
# ifndef HAVE___THREAD
  mgr->p_errnop = &mgr->p_errno;
# endif
  mgr->p_start_args = (struct pthread_start_args) PTHREAD_START_ARGS_INITIALIZER(__pthread_manager);
  mgr->p_nr = 1;
# if __LT_SPINLOCK_INIT != 0
  self->p_resume_count = (struct pthread_atomic) __ATOMIC_INITIALIZER;
# endif
  mgr->p_alloca_cutoff = PTHREAD_STACK_MIN / 4;
#else
  mgr = &__pthread_manager_thread;
#endif

  /* Start the thread manager */
  int err = __pthread_start_manager(mgr);

  if (__builtin_expect (err, 0) == -1) {
#ifdef USE_TLS
    _dl_deallocate_tls (tcbp, true);
#endif
    free(__pthread_manager_thread_bos);
    return -1;
  }
  return 0;
}

/* Thread creation */

int __pthread_create(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  struct pthread_request request;
  int retval;
  if (__builtin_expect (l4_is_invalid_cap(__pthread_manager_request), 0)) {
    if (__pthread_initialize_manager() < 0) return EAGAIN;
  }
  request.req_thread = self;
  request.req_kind = REQ_CREATE;
  request.req_args.create.attr = attr;
  request.req_args.create.fn = start_routine;
  request.req_args.create.arg = arg;
  __pthread_send_manager_rq(&request, 1);
  retval = THREAD_GETMEM(self, p_retcode);
  if (__builtin_expect (retval, 0) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return retval;
}
strong_alias (__pthread_create, pthread_create)

/* Simple operations on thread identifiers */

pthread_descr __pthread_thread_self(void)
{
  return thread_self();
}

pthread_t __pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}
strong_alias (__pthread_self, pthread_self)

int __pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}
strong_alias (__pthread_equal, pthread_equal)


/* Process-wide exit() request */

static void pthread_onexit_process(int retcode, void *arg)
{
  //l4/if (__builtin_expect (__pthread_manager_request, 0) >= 0) {
  if (!l4_is_invalid_cap(__pthread_manager_request)) {
    struct pthread_request request;
    pthread_descr self = thread_self();

    /* Make sure we come back here after suspend(), in case we entered
       from a signal handler.  */
    //l4/THREAD_SETMEM(self, p_signal_jmp, NULL);

    request.req_thread = self;
    request.req_kind = REQ_PROCESS_EXIT;
    request.req_args.exit.code = retcode;
    __pthread_send_manager_rq(&request, 1);
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread)
      {
	UNIMPL();
#if 0
#ifdef USE_TLS
	waitpid(manager_thread->p_pid, NULL, __WCLONE);
#else
	waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
#endif
	/* Since all threads have been asynchronously terminated
           (possibly holding locks), free cannot be used any more.
           For mtrace, we'd like to print something though.  */
	/* #ifdef USE_TLS
	   tcbhead_t *tcbp = (tcbhead_t *) manager_thread;
	   # if TLS_DTV_AT_TP
	   tcbp = (tcbhead_t) ((char *) tcbp + TLS_PRE_TCB_SIZE);
	   # endif
	   _dl_deallocate_tls (tcbp, true);
	   #endif
	   free (__pthread_manager_thread_bos); */
#endif
	__pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
      }
  }
}

#ifndef HAVE_Z_NODELETE
static int __pthread_atexit_retcode;

static void pthread_atexit_process(void *arg, int retcode)
{
  pthread_onexit_process (retcode ?: __pthread_atexit_retcode, arg);
}

static void pthread_atexit_retcode(void *arg, int retcode)
{
  __pthread_atexit_retcode = retcode;
}
#endif


/* Reset the state of the thread machinery after a fork().
   Close the pipe used for requests and set the main thread to the forked
   thread.
   Notice that we can't free the stack segments, as the forked thread
   may hold pointers into them. */



/* Concurrency symbol level.  */
static int current_level;

int __pthread_setconcurrency(int level)
{
  /* We don't do anything unless we have found a useful interpretation.  */
  current_level = level;
  return 0;
}
weak_alias (__pthread_setconcurrency, pthread_setconcurrency)

int __pthread_getconcurrency(void)
{
  return current_level;
}
weak_alias (__pthread_getconcurrency, pthread_getconcurrency)

/* Primitives for controlling thread execution */


/* Debugging aid */

#ifdef DEBUG
#include <stdarg.h>

void __pthread_message(const char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", __getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  TEMP_FAILURE_RETRY(write_not_cancel(2, buffer, strlen(buffer)));
}

#endif
