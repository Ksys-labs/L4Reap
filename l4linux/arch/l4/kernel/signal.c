#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/spinlock.h>

#include <asm/api/macros.h>
#include <asm/l4x/signal.h>
#include <l4/sys/kdebug.h>

#if defined(CONFIG_X86)
int l4x_deliver_signal(int exception_nr, int errcode)
{
	siginfo_t info;

	info.si_signo = SIGSEGV;
	info.si_errno = 0;
	info.si_code  = SEGV_MAPERR;
	info.si_addr  = (void __user *)L4X_THREAD_REGSP(&current->thread)->ip;

	force_sig_info(SIGSEGV, &info, current);

	if (signal_pending(current)) {
		do_signal(L4X_THREAD_REGSP(&current->thread));
		return 1;
	}

	return 0;
}
#elif defined(ARCH_arm)
int l4x_deliver_signal(int exception_nr, int errcode)
{
	siginfo_t info;

	info.si_signo = SIGSEGV;
	info.si_errno = 0;
	info.si_code  = SEGV_MAPERR;
	info.si_addr  = (void __user *)L4X_THREAD_REGSP(&current->thread)->ARM_pc;

	force_sig_info(SIGSEGV, &info, current);

	if (signal_pending(current)) {
		do_signal(L4X_THREAD_REGSP(&current->thread), 0);
		return 1;
	}

	return 0;
}
#else
#error Unknown arch
#endif

void l4x_sig_current_kill(void)
{
	/*
	 * We're a user process which just got a SIGKILL/SEGV and we're now
	 * preparing to die...
	 */

	/*
	 * empty queue and only put SIGKILL/SEGV into it so that the process
	 * gets killed ASAP
	 */
	spin_lock_irq(&current->sighand->siglock);
	flush_signals(current);
	force_sig(SIGKILL, current);
	spin_unlock_irq(&current->sighand->siglock);

	/*
	 * invoke do_signal which will dequeue the signal from the queue
	 * and feed us further to do_exit
	 */
#if defined(CONFIG_X86)
	do_signal(L4X_THREAD_REGSP(&current->thread));
#elif defined(ARCH_arm)
	do_signal(L4X_THREAD_REGSP(&current->thread), 0);
#else
#error Wrong arch
#endif
	panic("The zombie walks after SIGKILL!");
}

