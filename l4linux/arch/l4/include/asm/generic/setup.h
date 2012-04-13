#ifndef __ASM_L4__GENERIC__SETUP_H__
#define __ASM_L4__GENERIC__SETUP_H__

#include <asm/thread_info.h>
#include <l4/sys/kip.h>

extern l4_kernel_info_t *l4lx_kinfo;

extern unsigned int l4x_kernel_taskno;

void l4x_setup_memory(char *cmdl,
                      unsigned long *main_mem_start,
                      unsigned long *main_mem_size,
                      unsigned long *isa_dma_mem_start,
                      unsigned long *isa_dma_mem_size);

unsigned long l4x_get_isa_dma_memory_end(void);

void l4x_free_initrd_mem(void);
void l4x_load_initrd(char *command_line);
void l4x_l4io_init(void);

void l4x_prepare_irq_thread(struct thread_info *ti, unsigned _cpu);

void __attribute__((noreturn)) l4x_exit_l4linux(void);

void l4x_thread_set_pc(l4_cap_idx_t thread, void *pc);

int atexit(void (*f)(void));

#endif /* ! __ASM_L4__GENERIC__SETUP_H__ */
