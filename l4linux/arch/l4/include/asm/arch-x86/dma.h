#ifndef __ASM__ARCH_X86__DMA_H__

#include <asm-x86/dma.h>

unsigned long l4x_get_isa_dma_memory_end(void);

#undef MAX_DMA_PFN
#define MAX_DMA_PFN (l4x_get_isa_dma_memory_end() >> PAGE_SHIFT)

#endif /* ! __ASM__ARCH_X86__DMA_H__ */
