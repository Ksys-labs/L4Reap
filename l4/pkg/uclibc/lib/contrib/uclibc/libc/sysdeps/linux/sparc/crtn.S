	.file	"initfini.c"
	
/* Adam: Fixed binutils >= 2.21 issue, pull upstream later when available */

	.section .init
	.align 4
	.global _init
	.type	_init, %function
	.proc	020
	.L1:
	ret
	restore
	.size	.L1, .-.L1
	.size	_init, .-_init
	
	.section .fini
	.align 4
	.global _fini
	.type	_fini, %function
	.proc	020
	.L2:
	ret
	restore
	.size	.L2, .-.L2
	.size	_fini, .-_fini
