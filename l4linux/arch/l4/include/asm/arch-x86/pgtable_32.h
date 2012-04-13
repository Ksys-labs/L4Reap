#ifndef _I386_PGTABLE_H
#define _I386_PGTABLE_H

#include <asm/pgtable_32_types.h>

/*
 * The Linux memory management assumes a three-level page table setup. On
 * the i386, we use that, but "fold" the mid level into the top-level page
 * table, so that we physically have the same two-level page table as the
 * i386 mmu expects.
 *
 * This file contains the functions and defines necessary to modify and use
 * the i386 page table tree.
 */
#ifndef __ASSEMBLY__
#include <asm/processor.h>
#include <asm/fixmap.h>
#include <linux/threads.h>
#include <asm/paravirt.h>

#include <linux/bitops.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include <asm/api/api.h>

struct mm_struct;
struct vm_area_struct;

extern pgd_t swapper_pg_dir[1024];
extern pgd_t initial_page_table[1024];

static inline void pgtable_cache_init(void) { }
static inline void check_pgt_cache(void) { }
void paging_init(void);

extern void set_pmd_pfn(unsigned long, unsigned long, pgprot_t);


/*
 * Define this if things work differently on an i386 and an i486:
 * it will (on an i486) warn about kernel memory accesses that are
 * done without a 'access_ok(VERIFY_WRITE,..)'
 */
#undef TEST_ACCESS_OK

#ifdef CONFIG_X86_PAE
# include <asm/pgtable-3level.h>
#else
# include <asm/pgtable-2level.h>
#endif

#undef set_pte
#undef set_pte_at
#undef pte_clear

/*
 * L4Linux uses this hook to synchronize the Linux page tables with
 * the real hardware page tables kept by the L4 kernel.
 */
extern unsigned long l4x_set_pte(struct mm_struct *mm, unsigned long addr, pte_t pteptr, pte_t pteval);
extern void          l4x_pte_clear(struct mm_struct *mm, unsigned long addr, pte_t ptep);

static inline void __l4x_set_pte(struct mm_struct *mm, unsigned long addr,
                                 pte_t *pteptr, pte_t pteval)
{
	if ((pte_val(*pteptr) & (_PAGE_PRESENT | _PAGE_MAPPED)) == (_PAGE_PRESENT | _PAGE_MAPPED))
		pteval.pte_low = l4x_set_pte(mm, addr, *pteptr, pteval);
	*pteptr = pteval;
}

static inline void set_pte(pte_t *pteptr, pte_t pteval)
{
	__l4x_set_pte(NULL, 0, pteptr, pteval);
}

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
                              pte_t *ptep , pte_t pte)
{
	__l4x_set_pte(mm, addr, ptep, pte);
}

static inline void pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	if ((pte_val(*ptep) & (_PAGE_PRESENT | _PAGE_MAPPED)) == (_PAGE_PRESENT | _PAGE_MAPPED))
		l4x_pte_clear(mm, addr, *ptep);
	ptep->pte_low = 0;
}

static inline void pte_clear_unmap(pte_t *ptep, int unmap)
{
	if (unmap)
		pte_clear(NULL, 0, ptep);
	else
		ptep->pte_low = 0;
}

#if defined(CONFIG_HIGHPTE)
#define pte_offset_map(dir, address)					\
	((pte_t *)kmap_atomic(pmd_page(*(dir))) +		\
	 pte_index((address)))
#define pte_unmap(pte) kunmap_atomic((pte))
#else
#define pte_offset_map(dir, address)					\
	((pte_t *)page_address(pmd_page(*(dir))) + pte_index((address)))
#define pte_unmap(pte) do { } while (0)
#endif

/* Clear a kernel PTE and flush it from the TLB */
#define kpte_clear_flush(ptep, vaddr)		\
do {						\
	pte_clear(&init_mm, (vaddr), (ptep));	\
	__flush_tlb_one((vaddr));		\
} while (0)

/*
 * The i386 doesn't have any external MMU info: the kernel page
 * tables contain all the necessary information.
 */
#define update_mmu_cache(vma, address, ptep) do { } while (0)

#endif /* !__ASSEMBLY__ */

/*
 * kern_addr_valid() is (1) for FLATMEM and (0) for
 * SPARSEMEM and DISCONTIGMEM
 */
#ifdef CONFIG_FLATMEM
#define kern_addr_valid(addr)	((addr) >= PAGE_SIZE)
#else
#define kern_addr_valid(kaddr)	(0)
#endif

#endif /* _ASM_X86_PGTABLE_32_H */
