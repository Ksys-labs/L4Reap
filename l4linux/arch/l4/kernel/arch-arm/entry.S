
#include <asm/assembler.h>
#include <linux/linkage.h>
#include <asm/api/config.h>

	.global NR_syscalls
	.equ NR_syscalls,0
#define CALL(x) .equ NR_syscalls,NR_syscalls+1
#include "calls.S"
#undef CALL
#define CALL(x) .long x


/*
 * This is the syscall table declaration for native ABI syscalls.
 * With EABI a couple syscalls are obsolete and defined as sys_ni_syscall.
 */
#define ABI(native, compat) native
#ifdef CONFIG_AEABI
#define OBSOLETE(syscall) sys_ni_syscall
#else
#define OBSOLETE(syscall) syscall
#endif

.type   sys_call_table, #object
ENTRY(sys_call_table)
#include "calls.S"
#undef ABI
#undef OBSOLETE

sys_mmap2:
	b	sys_mmap_pgoff

/* ---- */
sys_fork_wrapper:
	b	sys_fork

sys_execve_wrapper:
	b	sys_execve

sys_vfork_wrapper:
	b	sys_vfork

sys_clone_wrapper:
	b	sys_clone

sys_sigaltstack_wrapper:
	b	do_sigaltstack

sys_fstatfs64_wrapper:
	teq     r1, #88
	moveq   r1, #84
	b	sys_fstatfs64

sys_statfs64_wrapper:
	teq     r1, #88
	moveq   r1, #84
	b	sys_statfs64


#ifdef CONFIG_OABI_COMPAT

/*
 * These are syscalls with argument register differences
 */

sys_oabi_pread64:
		stmia	sp, {r3, r4}
		b	sys_pread64
ENDPROC(sys_oabi_pread64)

sys_oabi_pwrite64:
		stmia	sp, {r3, r4}
		b	sys_pwrite64
ENDPROC(sys_oabi_pwrite64)

sys_oabi_truncate64:
		mov	r3, r2
		mov	r2, r1
		b	sys_truncate64
ENDPROC(sys_oabi_truncate64)

sys_oabi_ftruncate64:
		mov	r3, r2
		mov	r2, r1
		b	sys_ftruncate64
ENDPROC(sys_oabi_ftruncate64)

sys_oabi_readahead:
		str	r3, [sp]
		mov	r3, r2
		mov	r2, r1
		b	sys_readahead
ENDPROC(sys_oabi_readahead)

/*
 * Let's declare a second syscall table for old ABI binaries
 * using the compatibility syscall entries.
 */
#define ABI(native, compat) compat
#define OBSOLETE(syscall) syscall

	.type	sys_oabi_call_table, #object
ENTRY(sys_oabi_call_table)
#include "calls.S"
#undef ABI
#undef OBSOLETE

#endif




	.macro  usr_ret, reg
#ifdef CONFIG_ARM_THUMB
	bx	\reg
#else
	mov	pc, \reg
#endif
	.endm

	.align 5
.global __upage_asm_start
__upage_asm_start:
__kuser_memory_barrier:                   @ 0x0fa0
	smp_dmb
	usr_ret lr

	.align 5

__kuser_cmpxchg:                          @ 0x0fc0

#if defined(CONFIG_NEEDS_SYSCALL_FOR_CMPXCHG)

	/*
	 * Poor you.  No fast solution possible...
	 * The kernel itself must perform the operation.
	 * A special ghost syscall is used for that (see traps.c).
	 */
	stmfd	sp!, {r7, lr}
	mov	r7, #0xff00		@ 0xfff0 into r7 for EABI
	orr	r7, r7, #0xf0
	swi	#0x9ffff0
	ldmfd	sp!, {r7, pc}

#elif __LINUX_ARM_ARCH__ < 6

#ifdef CONFIG_MMU

	/*
	 * The only thing that can break atomicity in this cmpxchg
	 * implementation is either an IRQ or a data abort exception
	 * causing another process/thread to be scheduled in the middle
	 * of the critical sequence.  To prevent this, code is added to
	 * the IRQ and data abort exception handlers to set the pc back
	 * to the beginning of the critical section if it is found to be
	 * within that critical section (see kuser_cmpxchg_fixup).
	 *
	 * L4 goes on to l4_atomic_cmpxchg
	 *
	 * r3: tmp
	 * r2: mem
	 * r1: newval
	 * r0: oldval
	 */
1:	ldr	r3, [r2]			@ load current val
	subs	r3, r3, r0			@ compare with oldval
2:	streq	r1, [r2]			@ store newval if eq
	rsbs	r0, r3, #0			@ set return val and C flag
	usr_ret	lr

	.global l4x_kuser_cmpxchg_critical_start
	.global l4x_kuser_cmpxchg_critical_end
	l4x_kuser_cmpxchg_critical_start = (1b - __kuser_cmpxchg + 0x0fc0)
	l4x_kuser_cmpxchg_critical_end   = (2b - __kuser_cmpxchg + 0x0fc0)

#else
#error "We never take this one"
#warning "NPTL on non MMU needs fixing"
	mov	r0, #-1
	adds	r0, r0, #0
	usr_ret	lr
#endif

#else

#ifdef CONFIG_SMP
	mcr	p15, 0, r0, c7, c10, 5	@ dmb
#endif
1:	ldrex	r3, [r2]
	subs	r3, r3, r0
	strexeq	r3, r1, [r2]
	teqeq	r3, #1
	beq	1b
	rsbs	r0, r3, #0
	/* beware -- each __kuser slot must be 8 instructions max */
#ifdef CONFIG_SMP
	b	__kuser_memory_barrier
#else
	usr_ret	lr
#endif

#endif
	.align 5

__kuser_get_tls:                          @ 0x0fe0
	ldr	r0, [pc, #(16 - 8)]
	usr_ret lr
	.word	0

	// haha, non-virtualizable...
#ifdef CONFIG_L4_VCPU
	mrc	p15, 0, r0, c13, c0, 2    @ tls-reg
	usr_ret lr
	.word	0
#else
	mrc	p15, 0, r0, c13, c0, 3    @ utcb-addr
	ldr	r0, [r0, #(127*4)]        @ tcr-user[2]
	usr_ret lr
#endif

	.word	0
	.word	0                         @ 0x0ff0, software TLS value


.global __upage_asm_end
__upage_asm_end:

