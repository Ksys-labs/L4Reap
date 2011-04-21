	.file	"initfini.c"

/* Adam: Fixed binutils >= 2.21 issue, pull upstream later when available */
	
	.section .init
	.align 2
	.globl _init
	.type	_init, @function

	.L1:
	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	.L1, .-.L1
	//.size	_init, .-_init
	
	.section .fini
	.align 2
	.globl _fini
	.type	_fini, @function

	.L2:
	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	.L2, .-.L2
	//.size	_fini, .-_fini
