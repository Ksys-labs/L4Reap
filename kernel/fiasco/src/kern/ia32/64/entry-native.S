
#include "config_tcbsize.h"
#include "config_gdt.h"
#include "globalconfig.h"
#include "idt_init.h"
#include <low_level.h>
#include "regdefs.h"
#include "shortcut.h"
#include "tcboffset.h"

	.p2align 4
	.globl	entry_vec01_debug
entry_vec01_debug:
/* XXX we have to check single step bug */
1:	push	$(0)
	push	$(1)
	push	%rax
	push	%rcx
	push	%rdx
	push	%rbx
	mov	%cr2,%rax
	push	%rax
	push	%rbp
	push	%rsi
	push	%rdi
	push	%r8
	push	%r9
	push	%r10
	push	%r11
	push	%r12
	push	%r13
	push	%r14
	push	%r15
	jmp	slowtraps


	.p2align(4)
	.globl	entry_vec08_dbf
entry_vec08_dbf:
	push	$(0)
	push	$(8)
	push	%rax
	push	%rcx
	push	%rdx
	push	%rbx
	mov	%cr2,%rax
	push	%rax
	push	%rbp
	push	%rsi
	push	%rdi
	push	%r8
	push	%r9
	push	%r10
	push	%r11
	push	%r12
	push	%r13
	push	%r14
	push	%r15
	mov	%rsp,%rdi		// 1st arg: trap state
	call	thread_handle_double_fault
	jmp	entry_vec08_dbf


	.p2align(4)
	.globl	entry_vec0a_invalid_tss
entry_vec0a_invalid_tss:
	andq	$0xffffffffffffbfff, 24(%rsp)
	add	$8, %rsp			/* skip error code */
	iretq

/* PPro spurious interrupt bug: 
 * See "Pentium Pro Processor Specification Update / January 1999"
 * Erratum "Virtual Wire mode through local APIC may cause int 15"
 * This exception can be silently ignored */
	.p2align(4)
	.globl	entry_vec0f_apic_spurious_interrupt_bug
entry_vec0f_apic_spurious_interrupt_bug:
	incl	apic_spurious_interrupt_bug_cnt
	iretq


/* XXX has to be fixed */
/* APIC error interrupt */
	.p2align(4)
	.globl	entry_apic_error_interrupt
entry_apic_error_interrupt:
	cld
	SAVE_SCRATCH
	lea	SCRATCH_REGISTER_SIZE(%rsp), %rdi
	call	apic_error_interrupt
	RESTORE_SCRATCH
	iretq

/* Intel Architecture Software Developer's Manual Volume 3,
 * Advanced Programmable Interrupt Controller (APIC):
 * Spurious Interrupt: "If at the time the INTA cycle is issued, the
 * interupt that was to be dispensed has become masked (programmed by
 * software), the local APIC will deliver a spurious-interrupt vector." */
	.p2align(4)
	.globl	entry_apic_spurious_interrupt
entry_apic_spurious_interrupt:
	incl	apic_spurious_interrupt_cnt
	iretq

	.p2align(4)
	.global	entry_int_apic_ignore
entry_int_apic_ignore:
	push	%rcx
	push	%rdx
	mov	apic_io_base, %rcx
	mov	0xf0(%rcx), %edx
	movl	$0, 0xb0(%rcx)
	pop	%rdx
	pop	%rcx
	iretq

#if defined(CONFIG_JDB)

	.p2align(4)
	.global entry_syscall_log
	.global entry_sys_fast_ipc_log
entry_syscall_log:
entry_sys_fast_ipc_log:
	mov	%rsp,%r15		/* save user rsp */
	mov	syscall_rsp0, %rsp	/* get address of kernel stack */
	pop	%rsp			/* set kernel stack */			

	push	$(GDT_DATA_USER | SEL_PL_U)	/* fake user ss */
	push	%r15				/* save user rsp */
	push	%r11				/* save user rflags */
	push	$(GDT_CODE_USER | SEL_PL_U)	/* fake user cs */
	push	%rcx				/* save user rip */
	
	cld
	SAVE_STATE_SYSEXIT
	ESP_TO_TCB_AT %rbx
	RESET_THREAD_CANCEL_AT %rbx

	call	*syscall_table
in_slow_ipc5:
	RESTORE_STATE_SYSEXIT
	
	pop	%rcx				/* load user rsp */
	add	$8,%rsp				/* through away cs */
	pop	%r11				/* load user rflags */
	pop	%r15				/* load user rsp */
	add	$8,%rsp				/* through away ss */
	
	mov	%r15,%rsp			/* restore user rsp */
	sysretq

	.globl	in_slow_ipc5

#endif // CONFIG_JDB


#if (defined (CONFIG_JDB_LOGGING) || !defined(CONFIG_ASSEMBLER_IPC_SHORTCUT))
	.p2align(4)
	.global entry_syscall_c
	.global entry_sys_fast_ipc_c
entry_syscall_c:
entry_sys_fast_ipc_c:
	mov	%rsp,%r15		/* save user rsp */
	mov	syscall_rsp0,%rsp	/* get address of kernel stack */
	pop	%rsp			/* set kernel stack */

	push	$(GDT_DATA_USER | SEL_PL_U)	/* fake user ss */
	push	%r15				/* save user rsp */
	push	%r11				/* save user rflags */
	push	$(GDT_CODE_USER | SEL_PL_U)	/* fake user cs */
	push	%rcx				/* save user rip */

	cld
	SAVE_STATE_SYSEXIT
	ESP_TO_TCB_AT %rbx
	RESET_THREAD_CANCEL_AT %rbx
	
	call    ipc_short_cut_wrapper
in_sc_ipc2:
	RESTORE_STATE_SYSEXIT
	
	pop	%rcx				/* load user rsp */
	add	$8,%rsp				/* through away cs */
	pop	%r11				/* load user rflags */
	pop	%r15				/* load user rsp */
	add	$8,%rsp				/* through away ss */
	
	mov 	%r15,%rsp			/* restore user rsp */
	sysretq
	
	.globl	in_sc_ipc2
#endif

	.bss
	.align	8
	.globl syscall_rsp0
syscall_rsp0:
	.space	8				/* address of esp0 in tss */
	.text

	.p2align(4)
	.globl	leave_from_sysenter_by_iret
leave_from_sysenter_by_iret:
	ESP_TO_TCB_AT %rbx
	RESET_THREAD_IPC_MASK_AT %rbx
	RESTORE_STATE
	pop	%rax
	orq	$EFLAGS_IF, 8(%rsp)
	iretq

#ifdef CONFIG_SVM
	.p2align(4)
	.globl	resume_vm
resume_vm:
	
	// save callee saved regs
	push	%rbx
	push	%r12
	push	%r13
	push	%r14
	push	%r15
	push	%rbp

	mov 	%rdi, %rax	// first arg goes to rax for vm*
	push	%rsi		// store pointer to register struct

	clgi

	sti
	nop

	// restore guest GP regs
	mov	  8(%rsi), %rdx
	mov	 16(%rsi), %rcx
	mov	 24(%rsi), %rdi
	mov	 32(%rsi), %r8
	mov	 40(%rsi), %r9
	mov	 48(%rsi), %rbx
	mov	 56(%rsi), %rbp
	mov	 64(%rsi), %r10
	mov	 72(%rsi), %r11
	mov	 80(%rsi), %r12
	mov	 88(%rsi), %r13
	mov	 96(%rsi), %r14
	mov	104(%rsi), %r15
	mov        (%rsi), %rsi

	// TODO: Debugregs

	// rax used as implicit operand for vm* instructions
	vmload
	vmrun
	vmsave

	push	%rsi
	mov	8(%rsp), %rsi	// get previously saved register struct pointer

	mov	%rdx,   8(%rsi) // save guest GP registers
	mov	%rcx,  16(%rsi)
	mov	%rdi,  24(%rsi)
	mov	%r8,   32(%rsi)
	mov	%r9,   40(%rsi)
	mov	%rbx,  48(%rsi)
	mov	%rbp,  56(%rsi)
	mov	%r10,  64(%rsi)
	mov	%r11,  72(%rsi)
	mov	%r12,  80(%rsi)
	mov	%r13,  88(%rsi)
	mov	%r14,  96(%rsi)
	mov	%r15, 104(%rsi)

	mov	(%rsp), %rdx      // guest rsi to rdx
	mov	%rdx, (%rsi)      // store in regs structure

	cli
	nop
	stgi

	add	$16, %rsp
	
	// restore callee saved registers
	pop	%rbp
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbx

	ret
#endif


	.bss
	.space	4096
	.global	dbf_stack_top
dbf_stack_top:

