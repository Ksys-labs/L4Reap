#ifndef __ASM_L4__GENERIC__TAMED_H__
#define __ASM_L4__GENERIC__TAMED_H__

#ifndef CONFIG_L4_VCPU
void l4x_tamed_init(void);
void l4x_tamed_set_mapping(int cpu, int nr);
void l4x_tamed_start(unsigned vcpu);
int  l4x_tamed_print_cli_stats(char *buffer);

#ifdef CONFIG_HOTPLUG_CPU
void l4x_tamed_shutdown(unsigned vcpu);
#endif
#endif

#endif /* ! __ASM_L4__GENERIC__TAMED_H__ */
