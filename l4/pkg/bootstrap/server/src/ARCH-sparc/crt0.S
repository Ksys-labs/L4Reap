#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;
#define TRAP_ENTRY(H) rd %psr, %l0; b H; rd %wim, %l3; nop;

#define TRAP_ENTRY_INTERRUPT(int_level) \
        mov int_level, %l7; rd %psr, %l0; b _prom_leonbare_irq_entry; rd %wim, %l3;

.data
.space 8192
.align(0x2000)
.globl _stack
_stack:

.text
/*
 * Trap handler table -> must be aligned to page size
 * as specified by the SPARC v8 manual (p. 31).
 */
.globl _leon_traphandlers
.align(0x1000)
.space(4096)
_leon_traphandlers:

/*
 */
.globl _start
_start:
    /* Things we should do here:
	 *  1) set up a trap handler table and load its address into %tbr
	 *  2) wait 3 instructions
	 */

	/* set stack pointer */
	sethi %hi(_stack), %sp
	or %sp, %lo(_stack), %sp

	sethi %hi(_stack), %fp
	or %fp, %lo(_stack), %fp

	ba __main
	nop

.text
.globl main
main:
    ta 19
