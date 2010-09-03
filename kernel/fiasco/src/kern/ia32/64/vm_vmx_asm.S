/**
 * Function resume_vm_vmx, arguments:
 *  - rdi: Pointer to registers (layout specific...)
 */
	.p2align(4)
	.globl resume_vm_vmx
	.globl vm_vmx_exit_vec
resume_vm_vmx:
	// save callee saved regs
	push	%rbx
	push	%r12
	push	%r13
	push	%r14
	push	%r15
	push	%rbp
	push	%gs
	push	%fs

	push	%rdi               // store pointer to register struct
	mov	$0x6c14, %eax      // save rsp in vmcs
	vmwrite	%rsp, %rax
	mov	(%rsp), %rsp

	// load guest GP registers
	pop	%rax
	pop	%rsi
	pop	%rdx
	pop	%rcx
	pop	%rdi
	pop	%r8
	pop	%r9
	pop	%rbx
	pop	%rbp
	pop	%r10
	pop	%r11
	pop	%r12
	pop	%r13
	pop	%r14
	pop	%r15

	vmresume
	jnz 1f
	vmlaunch
1:	// error path
	mov	$0x6c14, %eax
	vmread	%rax, %rsp
	pushf
	pop	%rax
	add	$24, %rsp        // pushed regs pointer and gs+fs
	pop	%rbp
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbx
	ret

vm_vmx_exit_vec:
	push	%rdi
	mov	8(%rsp), %rdi     // get previously saved register struct pointer

	// save guest GP registers
	mov	%rax,   0(%rdi)
	mov	%rsi,   8(%rdi)
	mov	%rdx,  16(%rdi)
	mov	%rcx,  24(%rdi)
	// rdi is saved below
	mov	%r8,   40(%rdi)
	mov	%r9,   48(%rdi)
	mov	%rbx,  56(%rdi)
	mov	%rbp,  64(%rdi)
	mov	%r10,  72(%rdi)
	mov	%r11,  80(%rdi)
	mov	%r12,  88(%rdi)
	mov	%r13,  96(%rdi)
	mov	%r14, 104(%rdi)
	mov	%r15, 112(%rdi)

	pop	%rax               // pop previously pushed guest rdi
	mov	%rax,  32(%rdi)

	add	$8, %rsp           // adjust stack after regs pointer push on stack

        // restore callee saved registers
	pop	%fs
	pop	%gs
	pop	%rbp
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbx

	xor	%rax, %rax
	ret

