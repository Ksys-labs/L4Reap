#ifndef __ASM_L4__GENERIC__CAP_ALLOC_H__
#define __ASM_L4__GENERIC__CAP_ALLOC_H__

#include <l4/sys/types.h>
#include <l4/re/c/util/cap_alloc.h>
#include <linux/spinlock.h>

extern spinlock_t l4x_cap_lock;

static inline l4_cap_idx_t l4x_cap_alloc(void)
{
	l4_cap_idx_t c;
	spin_lock(&l4x_cap_lock);
	c = l4re_util_cap_alloc();
	spin_unlock(&l4x_cap_lock);
	return c;
}

static inline void l4x_cap_free(l4_cap_idx_t c)
{
	spin_lock(&l4x_cap_lock);
	l4re_util_cap_free(c);
	spin_unlock(&l4x_cap_lock);
}


#endif /* ! __ASM_L4__GENERIC__CAP_ALLOC_H__ */
