/**
 * Function resume_vm_svm, arguments:
 *  - eax: physical VMCB address
 *  - edx: Pointer to registers (layout specific...)
 *
 * Note: To be called with interrupts disabled!
 */

	.p2align(4)
	.globl	resume_vm_svm
resume_vm_svm:

	// save callee saved regs
	pushl %edi
	pushl %esi
	pushl %ebx
	pushl %ebp

	pushl %edx              // store pointer to register struct

	clgi			// super CLI

	sti
	nop

	movl 4(%edx), %ecx	// restore guest GP registers
	movl 8(%edx), %ebx
	movl 12(%edx), %ebp
	movl 16(%edx), %esi
	movl 20(%edx), %edi
	movl (%edx), %edx
	
	// TODO: Debugregs etc.
		
	// eax used as implicit operand for vm* instructions
	vmload
	vmrun		
	vmsave

	pushl %edx
	movl 4(%esp), %edx     // get previously saved register struct pointer

	movl %ecx, 4(%edx)     // save guest GP registers
	movl %ebx, 8(%edx)
	movl %ebp, 12(%edx)
	movl %esi, 16(%edx)
	movl %edi, 20(%edx)

	movl (%esp), %ecx      // guest EDX to ECX
	movl %ecx, (%edx)      // EDX has offset 0 in structure

	// TODO: restore task register, clear busy flag

	cli
	nop
	stgi
						
	addl $8, %esp         // adjust stack after two pushs

	// restore callee saved registers
	popl %ebp
	popl %ebx
	popl %esi
	popl %edi
		
	ret
