/* glibc's sysdeps/x86_64/elf/initfini.c used for reference [EPILOG] */

/* Adam: Fixed binutils >= 2.21 issue, pull upstream later when available */

.file	"initfini.c"

.section .init
.global  _init
.type    _init, %function
.L1:
	addq $8, %rsp
	ret
.size .L1,.-.L1


.section .fini
.global  _fini
.type    _fini, %function
.L2:
	addq $8, %rsp
	ret
.size .L2, .-.L2
