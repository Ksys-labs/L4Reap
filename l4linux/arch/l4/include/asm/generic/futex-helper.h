#ifndef __INCLUDE__ASM__GENERIC__FUTEX_HELPER_H__
#define __INCLUDE__ASM__GENERIC__FUTEX_HELPER_H__

#include <asm/generic/memory.h>

#define L4X_FUTEX_TRANSLATE_UADDR_NOCHECK(uaddr) \
	do { \
		unsigned long page, offset; \
		if ((page = parse_ptabs_write((unsigned long)uaddr, &offset)) == -EFAULT) \
			return -EFAULT; \
		uaddr = (u32 __user *)(page + offset); \
	} while (0)

#define L4X_FUTEX_TRANSLATE_UADDR(uaddr) \
	do { \
		if (current->mm) \
			L4X_FUTEX_TRANSLATE_UADDR_NOCHECK(uaddr); \
	} while (0)

#endif /* ! __INCLUDE__ASM__GENERIC__FUTEX_HELPER_H__ */
