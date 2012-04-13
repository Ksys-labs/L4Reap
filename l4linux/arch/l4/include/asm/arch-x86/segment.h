#ifndef __ASM_L4__ARCH_I386__SEGMENT_H__
#define __ASM_L4__ARCH_I386__SEGMENT_H__

/* Just modify one value */

#include <asm-x86/segment.h>

#ifndef __ASSEMBLY__
#include <l4/sys/segment.h>

extern unsigned l4x_fiasco_gdt_entry_offset;

#undef  GDT_ENTRY_TLS_MIN
#define GDT_ENTRY_TLS_MIN	l4x_fiasco_gdt_entry_offset
#endif

#endif /* ! __ASM_L4__ARCH_I386__SEGMENT_H__ */
