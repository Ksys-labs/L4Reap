/*
 * linux/include/asm-l4/arch-arm/arch/vmalloc.h
 */
#ifndef __ASM_L4__ARCH_ARM__ARCH__VMALLOC_H__
#define __ASM_L4__ARCH_ARM__ARCH__VMALLOC_H__

#include <asm/api/api.h>

#define VMALLOC_SIZE	128

#define VMALLOC_START	l4x_vmalloc_memory_start
#define VMALLOC_END	(l4x_vmalloc_memory_start + (VMALLOC_SIZE << 20))

#endif /* ! __ASM_L4__ARCH_ARM__ARCH__VMALLOC_H__ */
