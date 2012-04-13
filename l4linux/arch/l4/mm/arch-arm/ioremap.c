#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/io.h>

#include <asm/cputype.h>
#include <asm/cacheflush.h>
#include <asm/mmu_context.h>
#include <asm/pgalloc.h>
#include <asm/tlbflush.h>
#include <asm/sizes.h>

#include <asm/mach/map.h>
#include "mm.h"

/*
 * Used by ioremap() and iounmap() code to mark (super)section-mapped
 * I/O regions in vm_struct->flags field.
 */
#define VM_ARM_SECTION_MAPPING	0x80000000

#define __ARCH_IOREMAP_C_INCLUDED__
#include "../io.c"

void __check_kvm_seq(struct mm_struct *mm)
{
}

/*
 * Remap an arbitrary physical address space into the kernel virtual
 * address space. Needed when the kernel wants to access high addresses
 * directly.
 *
 * NOTE! We need to allow non-page-aligned mappings too: we will obviously
 * have to convert them into an offset in a page-aligned mapping, but the
 * caller shouldn't need to know that small detail.
 */


void __iomem *
__arm_ioremap(unsigned long phys_addr, size_t size, unsigned int flags)
{
	return __l4x_ioremap(phys_addr, size, flags);
}
EXPORT_SYMBOL(__arm_ioremap);

void __iounmap(volatile void __iomem *addr)
{
	l4x_iounmap(addr);
}
EXPORT_SYMBOL(__iounmap);
