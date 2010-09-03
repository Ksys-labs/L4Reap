/**
 * Function resume_vm_vmx, arguments:
 *  - eax: Pointer to registers (layout specific...)
 */
	.p2align(4)
	.globl resume_vm_vmx
	.globl vm_vmx_exit_vec
resume_vm_vmx:
	// save callee saved regs
	push	%edi
	push	%esi
	push	%ebx
	push	%ebp
	push	%ds
	push	%es
	push	%gs
	push	%fs

	push	%eax	        // store pointer to register struct
	mov	$0x6c14, %eax	// save esp in vmcs
	vmwrite	%esp, %eax
	mov	(%esp), %esp

	// load guest GPs
	pop	%eax
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi

	vmresume
	jnz	1f
	vmlaunch
1: // error path
	mov	$0x6c14, %eax
	vmread	%eax, %esp
	pushf
	pop	%eax
	add	$20, %esp      // pushed regs pointer and gs+fs+ds+es
	pop	%ebp
	pop	%ebx
	pop	%esi
	pop	%edi
	ret

vm_vmx_exit_vec:
	push	%eax
	mov	4(%esp), %eax   // get previously saved register struct pointer

	// eax is saved below
	mov	%edx, 4(%eax)   // save guest GP registers
	mov	%ecx, 8(%eax)
	mov	%ebx, 12(%eax)
	mov	%ebp, 16(%eax)
	mov	%esi, 20(%eax)
	mov	%edi, 24(%eax)

	pop	%ecx            // guest EAX to ECX
	mov	%ecx, 0(%eax)

	add	$4, %esp	// adjust stack after regs pointer push on stack
	
	// restore callee saved registers
	pop	%fs
	pop	%gs
	pop	%es
	pop	%ds
	pop	%ebp
	pop	%ebx
	pop	%esi
	pop	%edi

	xor	%eax, %eax
	ret
