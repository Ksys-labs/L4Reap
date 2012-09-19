#ifndef __ASM_L4__L4X_ARM__DMA_H__
#define __ASM_L4__L4X_ARM__DMA_H__

#include <asm/memory.h>

#ifdef CONFIG_L4_DMAPOOL
int l4x_dmapool_mem_add(unsigned long virt, unsigned long phys, size_t size);
int l4x_dmapool_is_in_virt_dma_space(unsigned long va, size_t sz);
#else
static inline int l4x_dmapool_is_in_virt_dma_space(unsigned long va, size_t sz)
{
	return 0;
}
#endif


void l4x_arm_consistent_init(unsigned long base);

int l4x_virt_addr_is_in_dma_range(unsigned long va, size_t sz);

#endif /* ! __ASM_L4__L4X_ARM__DMA_H__ */
