	.p2align(4)
	.globl	resume_vm_svm
resume_vm_svm:
	
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
