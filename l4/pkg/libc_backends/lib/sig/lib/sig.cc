/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/thread>
#include <l4/sys/factory>
#include <l4/sys/scheduler>
#include <l4/sys/debugger.h>
#include <l4/cxx/ipc_server>
#include <l4/re/env>
#include <l4/re/debug>
#include <l4/re/util/cap_alloc>
#include <l4/util/util.h>
#include <l4/libc_backends/sig.h>

#include <sys/time.h>

#include "arch.h"

#include <errno.h>
#include <signal.h>
#include <cstdio>

namespace {

struct Sig_handling
{
  // handlers registered with 'signal'
  struct sigaction sigactions[_NSIG];

  l4_umword_t sigthread_stack[2048 / sizeof(l4_umword_t)]; // big stack is thanks to printf
  L4::Cap<L4::Thread> thcap;

  struct itimerval current_itimerval;
  l4_cpu_time_t alarm_timeout;

  Sig_handling();

  void ping_exc_handler();
  l4_addr_t get_handler(int signum);
  sighandler_t signal(int signum, sighandler_t handler) throw();
  int sigaction(int signum, const struct sigaction *act,
                struct sigaction *oldact) throw();
  int setitimer(__itimer_which_t __which,
                __const struct itimerval *__restrict __new,
                struct itimerval *__restrict __old) throw();

public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  int handle_exception(L4::Ipc_iostream &ios);
};

}

// -----------------------------------------------------------------------


l4_addr_t
Sig_handling::get_handler(int signum)
{
  if (signum >= _NSIG)
    return 0;
  if (   (sigactions[signum].sa_flags & SA_SIGINFO)
      && sigactions[signum].sa_sigaction)
    return (l4_addr_t)sigactions[signum].sa_sigaction;
  else if (sigactions[signum].sa_handler)
    return (l4_addr_t)sigactions[signum].sa_handler;
  return 0;
}


asm(
".text                           \n\t"
".global libc_be_sig_return_trap \n\t"
"libc_be_sig_return_trap:        \n\t"
#if defined(ARCH_x86) || defined(ARCH_amd64)
"                          ud2a  \n\t"
#elif defined(ARCH_arm)
".p2align 2                      \n\t"
"word: .long                    0xe1600070 \n\t" // smc
#elif defined(ARCH_ppc32)
"trap                            \n\t"
#else
#error Unsupported arch!
#endif
".previous                       \n\t"
);

extern char libc_be_sig_return_trap[];

static bool range_ok(l4_addr_t start, unsigned long size)
{
  l4_addr_t offset;
  unsigned flags;
  L4::Cap<L4Re::Dataspace> ds;

  return !L4Re::Env::env()->rm()->find(&start, &size, &offset, &flags, &ds)
         && !(flags & L4Re::Rm::Read_only);
}

static void dump_rm()
{
  L4::Cap<L4Re::Debug_obj> d(L4Re::Env::env()->rm().cap());
  d->debug(0);
}

static bool setup_sig_frame(l4_exc_regs_t *u, int signum)
{
  // put state + pointer to it on stack
  ucontext_t *ucf = (ucontext_t *)(u->sp - sizeof(*ucf));

  /* Check if memory access is fine */
  if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
    return false;

  fill_ucontext_frame(ucf, u);

#ifdef ARCH_arm
  u->sp = (l4_umword_t)ucf;
  u->r[0] = signum;
  u->r[1] = 0; // siginfo_t pointer, we do not have one right currently
  u->r[2] = (l4_umword_t)ucf;
  u->ulr  = (unsigned long)libc_be_sig_return_trap;
#else
  u->sp = (l4_umword_t)ucf - sizeof(void *);
  *(l4_umword_t *)u->sp = (l4_umword_t)ucf;

  // siginfo_t pointer, we do not have one right currently
  u->sp -= sizeof(siginfo_t *);
  *(l4_umword_t *)u->sp = 0;

  // both types get the signum as the first argument
  u->sp -= sizeof(l4_umword_t);
  *(l4_umword_t *)u->sp = signum;

  u->sp -= sizeof(l4_umword_t);
  *(unsigned long *)u->sp = (unsigned long)libc_be_sig_return_trap;
#endif

  return true;
}

int Sig_handling::handle_exception(L4::Ipc_iostream &ios)
{
  l4_exc_regs_t _u;
  l4_exc_regs_t *u = &_u;
  l4_addr_t handler;
  int pc_delta = 0;

  *u = *l4_utcb_exc();

#ifdef ARCH_arm
  pc_delta = -4;
#endif

#ifdef ARCH_arm
  if ((u->err & 0x00f00000) == 0x00500000)
#elif defined(ARCH_ppc32)
  if ((u->err & 3) == 4)
#else
  if (u->trapno == 0xff)
#endif
    {
      //printf("SIGALRM\n");

      if (!(handler = get_handler(SIGALRM)))
        {
          printf("No signal handler found\n");
          return -L4_ENOREPLY;
        }

      if (!setup_sig_frame(u, SIGALRM))
        {
          printf("Invalid user memory for sigframe...\n");
          return -L4_ENOREPLY;
        }

      l4_utcb_exc_pc_set(u, handler);
      ios.put(*u); // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  // x86: trap6
  if (l4_utcb_exc_pc(u) + pc_delta == (l4_addr_t)libc_be_sig_return_trap)
    {
      // sig-return
      //printf("Sigreturn\n");

#ifdef ARCH_arm
      ucontext_t *ucf = (ucontext_t *)u->sp;
#else
      ucontext_t *ucf = (ucontext_t *)(u->sp + sizeof(l4_umword_t) * 3);
#endif

      if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
        {
          dump_rm();
          printf("Invalid memory...\n");
          return -L4_ENOREPLY;
        }

      fill_utcb_exc(u, ucf);

      //show_regs(u);

      ios.put(*u); // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  if (!(handler = get_handler(SIGSEGV)))
    {
      printf("No signal handler found\n");
      return -L4_ENOREPLY;
    }


  printf("Doing SIGSEGV\n");

  if (!setup_sig_frame(u, SIGSEGV))
    {
      printf("Invalid user memory for sigframe...\n");
      return -L4_ENOREPLY;
    }

  show_regs(u);

  l4_utcb_exc_pc_set(u, handler);
  ios.put(*u); // expensive? how to set amount of words in tag without copy?

  //printf("and back\n");
  return -L4_EOK;
}

int Sig_handling::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  switch (t.label())
    {
    case L4_PROTO_EXCEPTION:
      return handle_exception(ios);
    default:
      return -L4_ENOSYS;
    };
}

static Sig_handling _sig_handling;

namespace {

struct Loop_hooks :
  public L4::Ipc_svr::Compound_reply,
  public L4::Ipc_svr::Default_setup_wait
{
  static l4_timeout_t timeout()
  {
    if (_sig_handling.alarm_timeout)
      {
	l4_timeout_t t;
	l4_rcv_timeout(l4_timeout_abs(_sig_handling.alarm_timeout, 1), &t);
	_sig_handling.alarm_timeout = 0;
	return t;
      }

    if (_sig_handling.current_itimerval.it_value.tv_sec == 0
	&& _sig_handling.current_itimerval.it_value.tv_usec == 0)
      return L4_IPC_NEVER;
    return l4_timeout(L4_IPC_TIMEOUT_NEVER,
	l4util_micros2l4to(_sig_handling.current_itimerval.it_value.tv_sec * 1000000 +
	  _sig_handling.current_itimerval.it_value.tv_usec));
  }

  void error(l4_msgtag_t res, L4::Ipc_istream &s)
  {
    long ipc_error = l4_ipc_error(res, s.utcb());

    if (ipc_error == L4_IPC_RETIMEOUT)
      {
	l4_msgtag_t t;

        // any thread is ok, right?!
	t = L4Re::Env::env()->main_thread()
            ->ex_regs(~0UL, ~0UL,
	              L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
	if (l4_error(t))
	  printf("ex_regs error\n");



	// reload
	_sig_handling.current_itimerval.it_value = _sig_handling.current_itimerval.it_interval;

	return;
      }
    printf("(unsupported/strange) loopabort: %lx\n", ipc_error);
  }
};


static void __handler_main()
{
  L4::Server<Loop_hooks> srv(l4_utcb());
  srv.loop_noexc(&_sig_handling);
}
}

Sig_handling::Sig_handling()
{
  l4_utcb_t *u = (l4_utcb_t *)L4Re::Env::env()->first_free_utcb();

  //L4Re::Env::env()->first_free_utcb((l4_addr_t)u + L4_UTCB_OFFSET);
  // error: passing ‘const L4Re::Env’ as ‘this’ argument of ‘void
  // L4Re::Env::first_free_utcb(l4_addr_t)’ discards qualifiers
  l4re_global_env->first_free_utcb = (l4_addr_t)u + L4_UTCB_OFFSET;


  L4Re::Util::Auto_cap<L4::Thread>::Cap tc = L4Re::Util::cap_alloc.alloc<L4::Thread>();
  if (!tc.is_valid())
    {
      fprintf(stderr, "libsig: Failed to acquire cap\n");
      return;
    }

  int err = l4_error(L4Re::Env::env()->factory()->create_thread(tc.get()));
  if (err < 0)
    {
      fprintf(stderr, "libsig: Failed create thread: %s(%d)\n",
              l4sys_errtostr(err), err);
      return;
    }

  L4::Thread::Attr a;

  a.bind(u, L4Re::This_task);
  a.pager(L4Re::Env::env()->rm());
  a.exc_handler(L4Re::Env::env()->rm());
  tc->control(a);

  l4_addr_t sp = (l4_addr_t)sigthread_stack + sizeof(sigthread_stack);
  //printf("stack top %lx\n", sp);

  tc->ex_regs(l4_addr_t(__handler_main), sp, 0);

  L4Re::Env::env()->scheduler()->run_thread(tc.get(), l4_sched_param(0xff));

  l4_debugger_set_object_name(tc.cap(), "&-");

  thcap = tc.release();

  libsig_be_add_thread(l4re_env()->main_thread);

  return;
}

void libsig_be_set_dbg_name(const char *n)
{
  char s[15];
  snprintf(s, sizeof(s) - 1, "&%s", n);
  s[sizeof(s) - 1] = 0;
  l4_debugger_set_object_name(_sig_handling.thcap.cap(), s);
}

void libsig_be_add_thread(l4_cap_idx_t t)
{
  L4::Cap<L4::Thread> tt(t);
  L4::Thread::Attr a;
  a.exc_handler(_sig_handling.thcap);
  if (int e = l4_error(tt->control(a)))
    fprintf(stderr, "libsig: thread-control error: %d\n", e);
  //printf("Set exc-handler %lx for %lx\n", thcap.cap(), t);
}

inline
void Sig_handling::ping_exc_handler() throw()
{
  l4_ipc_call(thcap.cap(), l4_utcb(), l4_msgtag(0, 0, 0, 0), L4_IPC_NEVER);
}

inline
sighandler_t
Sig_handling::signal(int signum, sighandler_t handler) throw()
{
  if (signum < _NSIG)
    {
      sighandler_t old = sigactions[signum].sa_handler;
      sigactions[signum].sa_handler = handler;
      return old;
    }

  return SIG_ERR;
}

extern "C"
sighandler_t signal(int signum, sighandler_t handler) L4_NOTHROW
{
  //printf("Called: %s(%d, %p)\n", __func__, signum, handler);
  return _sig_handling.signal(signum, handler);
}

inline
int
Sig_handling::sigaction(int signum, const struct sigaction *act,
                        struct sigaction *oldact) throw()
{
  if (signum == SIGKILL || signum == SIGSTOP)
    return -EINVAL;

  if (signum < _NSIG)
    {
      if (oldact)
        *oldact = sigactions[signum];
      if (act)
        sigactions[signum] = *act;
      return 0;
    }

  return -EINVAL;
}

extern "C"
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact) L4_NOTHROW
{
  //printf("Called: %s(%d, %p, %p)\n", __func__, signum, act, oldact);
  int err = _sig_handling.sigaction(signum, act, oldact);
  if (err < 0)
    {
      errno = -err;
      return -1;
    }

  return err;
}

extern "C"
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) throw()
{
  printf("%s(%d, %p, %p): Unimplemented\n", __func__, how, set, oldset);
  errno = EINVAL;
  return -1;
}

extern "C"
int sigpending(sigset_t *set) throw()
{
  printf("%s(%p): Unimplemented\n", __func__, set);
  errno = EFAULT;
  return -1;
}

int sigsuspend(const sigset_t *mask) throw()
{
  printf("%s(%p): Unimplemented\n", __func__, mask);
  errno = EFAULT;
  return -1;
}

extern "C"
int killpg(int pgrp, int sig) throw()
{
  printf("%s(%d, %d): Unimplemented\n", __func__, pgrp, sig);
  errno = EPERM;
  return -1;
}

extern "C"
unsigned int alarm(unsigned int seconds)
{
  //printf("unimplemented: alarm(%u)\n", seconds);

  _sig_handling.alarm_timeout = l4re_kip()->clock + seconds * 1000000;

  _sig_handling.ping_exc_handler();
  return 0;
}

extern "C"
pid_t wait(int *status)
{
  printf("unimplemented: wait(%p)\n", status);
  return -1;
}



int getitimer(__itimer_which_t __which,
              struct itimerval *__value) L4_NOTHROW
{
  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  *__value = _sig_handling.current_itimerval;

  _sig_handling.ping_exc_handler();
  return 0;
}

inline
int
Sig_handling::setitimer(__itimer_which_t __which,
                        __const struct itimerval *__restrict __new,
                        struct itimerval *__restrict __old) throw()
{
  printf("called %s(..)\n", __func__);

  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  if (__old)
    *__old = current_itimerval;

  if (__new->it_value.tv_usec < 0
      || __new->it_value.tv_usec > 999999
      || __new->it_interval.tv_usec < 0
      || __new->it_interval.tv_usec > 999999)
    {
      errno = EINVAL;
      return -1;
    }

  printf("%s: setting stuff\n", __func__);
  current_itimerval = *__new;

  ping_exc_handler();
  return 0;
}

int setitimer(__itimer_which_t __which,
              __const struct itimerval *__restrict __new,
              struct itimerval *__restrict __old) L4_NOTHROW
{
  int err = _sig_handling.setitimer(__which, __new, __old);
  if (err < 0)
    {
      errno = -err;
      return -1;
    }
  return 0;
}
