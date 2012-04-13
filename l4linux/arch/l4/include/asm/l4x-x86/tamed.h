/*
 * Architecture specific handling for tamed mode for i386.
 */
#ifndef __ASM_L4__L4X_I386__TAMED_H__
#define __ASM_L4__L4X_I386__TAMED_H__

#ifndef L4X_TAMED_LABEL
#error Only use from within tamed.c!
#endif

static inline void l4x_tamed_sem_down(void)
{
	unsigned dummy1, dummy2, dummy3;
	unsigned cpu = current_thread_info()->cpu;

	asm volatile
	  (
	   "1:                         \n\t"
	   "decl    0(%%ebx)           \n\t"        /* decrement counter */
	   "jge     2f                 \n\t"

#ifdef CONFIG_L4_DEBUG_TAMED_COUNT_INTERRUPT_DISABLE
	   "incl    cli_taken          \n\t"
#endif
	   "push    %%edx              \n\t"
	   "push    %%eax              \n\t"
	   "push    %%esi              \n\t"
	   "push    %%ebx              \n\t"

	   "xor     %%ecx,%%ecx        \n\t"        /* timeout never */

	   L4_ENTER_KERNEL

	   "test    $0x00010000, %%eax \n\t"        /* Check return tag */

	   "pop     %%ebx              \n\t"
	   "pop     %%esi              \n\t"
	   "pop     %%eax              \n\t"
	   "pop     %%edx              \n\t"

	   "jnz     2f                 \n\t"
	   "jmp     1b                 \n\t"

	   "2:                         \n\t"
	   : "=c" (dummy1), "=D" (dummy2), "=S" (dummy3)
	   : "a"  ((l4x_stack_prio_get() << 20) | (1 << 16)),
	     "b"  (&tamed_per_nr(cli_lock, get_tamer_nr(cpu)).sem),
	     "D"  (l4_utcb()),
	     "d"  (tamed_per_nr(cli_sem_thread_id, get_tamer_nr(cpu)) | L4_SYSF_CALL),
             "S"  (l4x_stack_id_get())
	   : "memory", "cc");
}

static inline void l4x_tamed_sem_up(void)
{
	unsigned dummy1, dummy2, dummy3, dummy4;
	unsigned cpu = current_thread_info()->cpu;
	l4_msgtag_t rtag;

	asm volatile
	  (
	   "incl    0(%%ebx)           \n\t"        /* increment counter */
	   "jg      2f                 \n\t"

	   L4_ENTER_KERNEL

	   "2:                         \n\t"
	   : "=a" (rtag), "=c" (dummy1), "=D" (dummy2), "=S" (dummy3), "=d" (dummy4)
	   : "a"  ((l4x_stack_prio_get() << 20) | (2 << 16)),
	     "b"  (&tamed_per_nr(cli_lock, get_tamer_nr(cpu)).sem),
	     "D"  (l4_utcb()),
	     "d"  (tamed_per_nr(cli_sem_thread_id, get_tamer_nr(cpu)) | L4_SYSF_CALL),
             "c"  (0),
	     "S"  (l4x_stack_id_get())
	   : "memory");

	if (unlikely(l4_ipc_error(rtag, l4_utcb())))
		outstring("l4x_tamed_sem_up ipc failed\n");
}

#endif /* ! __ASM_L4__L4X_I386__TAMED_H__ */
