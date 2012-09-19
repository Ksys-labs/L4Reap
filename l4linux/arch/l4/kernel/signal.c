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
