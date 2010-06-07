#include "local.h"

/******************************************************************************
 ** Dummy signal implementation.                                             **
 ** DDE does not provide its own signal implementation. To make it compile,  **
 ** we provide dummy versions of signalling functions here. If later on      **
 ** someone *REALLY* wants to use signals in the DDE context, he might       **
 ** erase this file and use something like the L4 signalling library for     **
 ** such purposes.                                                           **
*******************************************************************************/

int sigprocmask(int how, sigset_t *set, sigset_t *oldset)
{
	return 0;
}

void flush_signals(struct task_struct *t)
{
}

int do_sigaction(int sig, struct k_sigaction *act, struct k_sigaction *oact)
{
	return 0;
}
