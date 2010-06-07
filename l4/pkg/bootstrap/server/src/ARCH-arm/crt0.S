/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
/* -*- c -*- */

.section .text.init,#alloc,#execinstr
.type _start,#function
.globl _start
_start:
	/*
	 * It might be the case that we're not run at the position where we
	 * have been linked to. If this is the case we copy ourselves to the
	 * position we're linked to.
	 */
	adr	r4, run		/* Running version */
        ldr     r5, .LCrun      /* supposed to be version */
	cmp	r4, r5		/* If equal ... */
	beq	run		/* ... go to run */

	/* Figure how to move */
        ldr     r7, .LCend_bin
	subs	r8, r5, r4	/* r8 is the distance between the blocks */
	bpl	move_behind

	/* Copy before, copy forwards */
	/* First, copy our copy loop to the very beginning to avoid code
	 * overwrites */
	mov	r2, r5			/* r2: run address */
	ldr	r0, .LCstart_bin
	ldr	r1, 3f
	str	r1, [r0], #4
	ldr	r1, 32f
	str	r1, [r0], #4
	ldr	r1, 33f
	str	r1, [r0], #4
	ldr	r1, 34f
	str	r1, [r0], #4
	ldr	r1, 35f
	str	r1, [r0], #4
	ldr	pc, .LCstart_bin

3:	ldr     r6, [r4], #4
32:     str     r6, [r5], #4
33:     cmp     r5, r7
34:     blt     3b
35:     mov	pc, r2

	/* Copy behind, copy backwards */
move_behind:
	sub	r8, r7, r8	/* r8 points to the end of source image */
3:	ldr	r6, [r8, #-4]!	/* Take bytes */
	str	r6, [r7, #-4]!	/* Put bytes */
	cmp	r5, r7
	blt	3b
        ldr     pc, .LCrun


.LCrun:       .word run
.LCstart_bin: .word _start
.LCend_bin:   .word _module_data_end

run:
	mov	r1, #0x1000
	sub	r1, r1, #1                 /* r1 == 0xfff */
	mrc	p15, 0, r0, c0, c0, 0      /* Main ID */
	lsr	r0, #4
	and	r0, r0, r1

	/* Check for processors that understand CPU ID */
	mov	r2, #0xb00
	orr 	r2, #0x002
	cmp	r0, r2
	beq	do_cpuid

	mov	r2, #0xc00
	orr 	r2, #0x009
	cmp	r0, r2
	beq	do_cpuid

	b	do_bootstrap               /* None matched, normal startup */

do_cpuid:
	mrc	p15, 0, r0, c0, c0, 5      /* CPU ID */
	and     r0, r0, #0xf               /* CPU id */
	cmp	r0, #0                     /* CPU0 continues with bootstrap */
	beq	do_bootstrap	

/* CPU1+ wait for bootup */

	// I-cache on
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #(1 << 12)
	mcr p15, 0, r0, c1, c0, 0

	// IRQs off, SVC
	mrs r0, cpsr
	orr r0, #0xd3
	msr cpsr_c, r0

	// enable GIC CPU interface + prio mask for IRQs
	mov	r4, #0x1f000000
	add	r4, r4, #0x100
	mov	r0, #0x1
	str	r0, [r4, #0]
	mov	r0, #0xf0
	str	r0, [r4, #4]

1:
	mov	r5,#0x10000000
	ldr	r6, [r5, #0x30]
	cmp	r6, #0
	movne	pc, r6
	wfi
	ldr	r0, [r4, #12]
	str	r0, [r4, #16]
	b	1b

do_bootstrap:
	ldr 	r3, .LCcrt0_tramppage      /* Load address of tramppage var */
     	str 	sp, [r3]                   /* Store SP in variable          */
	ldr 	sp, .LCstack

	bl	__main
1:	b	1b

.LCcrt0_tramppage:      .word crt0_tramppage
.LCstack: .word crt0_stack_high

.section ".bss"

	.global	crt0_tramppage
crt0_tramppage:
	.space 4

	.global crt0_stack_low
        .align 3
crt0_stack_low:
	.space	8192
	.global crt0_stack_high
crt0_stack_high:
