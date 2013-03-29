#ifndef __ASM_L4__ARCH_ARM__ARCH__IRQS_H__
#define __ASM_L4__ARCH_ARM__ARCH__IRQS_H__

/* #define NR_IRQS        (NR_IRQS_HW + 10) */

#ifdef  CONFIG_L4_PLAT_NONE
#undef NR_IRQS
#undef NR_IRQS_HW
#define NR_IRQS		220
#define NR_IRQS_HW	210
#endif

#if defined(CONFIG_L4_PLAT_OVERO) || defined(CONFIG_L4_PLAT_IGEP)
#undef NR_IRQS
#undef NR_IRQS_HW
#define NR_IRQS		410
#define NR_IRQS_HW	385
#endif

#endif /* ! __ASM_L4__ARCH_ARM__ARCH__IRQS_H__ */
