#ifndef __ASM_L4__GENERIC__FERRET_H__
#define __ASM_L4__GENERIC__FERRET_H__

#ifdef CONFIG_L4_FERRET
#include <l4/ferret/sensors/histogram_producer.h>
#include <l4/ferret/sensors/list_producer.h>
#include <l4/ferret/sensors/list_init.h>
#include <l4/ferret/client.h>
#include <l4/ferret/maj_min.h>

#include <asm/api/config.h>

int l4x_ferret_init(void);

#ifdef CONFIG_L4_FERRET_KERNEL
extern ferret_list_local_t *l4x_ferret_kernel;
#endif /* CONFIG_L4_FERRET_KERNEL */

#ifdef CONFIG_L4_FERRET_SYSCALL_COUNTER
extern ferret_histo_t *l4x_ferret_syscall_ctr;
#endif

#ifdef CONFIG_L4_FERRET_USER

#define L4X_FERRET_USER_START ((l4_umword_t)(UPAGE_USER_ADDRESS + PAGE_SIZE))

extern ferret_list_local_t *l4x_ferret_user;
extern size_t l4x_ferret_user_size;

static inline int l4x_ferret_handle_pf(l4_umword_t pfa,
                                       l4_umword_t *dw0, l4_umword_t *dw1)
{
	if (l4x_ferret_user_size
	    && L4X_FERRET_USER_START <= pfa
	    && l4_trunc_page(pfa)
	           < L4X_FERRET_USER_START + l4x_ferret_user_size) {
		*dw0 &= PAGE_MASK;
                *dw1  = l4_fpage((((l4_umword_t)l4x_ferret_user->glob) +
		                  l4_trunc_page(pfa) - L4X_FERRET_USER_START)
		                  & L4_PAGEMASK,
		                 L4_LOG2_PAGESIZE,
		                 L4_FPAGE_RW, L4_MAP_ITEM_MAP).fpage;
		return 1;
	}
	return 0;
}

#endif /* CONFIG_L4_FERRET_USER */
#endif /* CONFIG_L4_FERRET */
#endif /* ! __ASM_L4__GENERIC__FERRET_H__ */
