#ifndef __ASM_L4__ARCH_I386__FIXMAP_H__
#define __ASM_L4__ARCH_I386__FIXMAP_H__

#include <asm-x86/fixmap.h>

#include <asm/generic/upage.h>

#ifdef CONFIG_X86_32
extern unsigned long l4x_fixmap_space_start;

/*
 * Have a slightly other version of fix_to_virt, leave everything in place
 * except intercept VDSO conversions.
 */
static inline unsigned long __l4x__fix_to_virt(const unsigned int x)
{
	if (x == FIX_VDSO)
		return UPAGE_USER_ADDRESS;
	if (x == FIX_VDSO - 1)
		return UPAGE_USER_ADDRESS_END;

	/* Original __fix_to_virt macro code */
	return (FIXADDR_TOP - ((x) << PAGE_SHIFT));
}

#undef __fix_to_virt
#define __fix_to_virt(x) __l4x__fix_to_virt(x)
#endif

#endif /* ! __ASM_L4__ARCH_I386__FIXMAP_H__ */
