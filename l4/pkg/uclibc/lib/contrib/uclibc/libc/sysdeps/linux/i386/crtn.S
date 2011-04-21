.file "initfini.c"

/* Adam: Fixed binutils >= 2.21 issue, pull upstream later when available */

.section .init
.global _init
.type   _init,%function
.L1:
	popl	%ebx
	popl	%ebp
	ret
.size .L1,.-.L1
//.size _init,.-_init



.section .fini
.global _fini
.type   _fini,%function
.L2:
	popl	%ebx
	popl	%ebp
	ret
.size .L2,.-.L2
//.size _fini,.-_fini



.section .gnu.linkonce.t.__get_pc_thunk_bx,"ax",@progbits
.global __get_pc_thunk_bx
.hidden	__get_pc_thunk_bx
.type   __get_pc_thunk_bx,%function
__get_pc_thunk_bx:
	movl	(%esp), %ebx
	ret
