#ifndef __ASM_L4__GENERIC__MEMORY_H__
#define __ASM_L4__GENERIC__MEMORY_H__

#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <asm/l4x/exception.h>

#include <l4/sys/types.h>

//#define DEBUG_LOOKUP_PTABS 1
//#define DEBUG_PARSE_PTABS_READ 1
//#define DEBUG_PARSE_PTABS_WRITE 1

#define RP_WRITABLE		0
#define RP_NOT_WRITABLE		1
#define RP_NOT_MAPPED		2
#define RP_RESERVED_COLOR	3
#define RP_4MB			22

#define PF_EUSER		4
#define PF_EKERNEL		0
#ifdef ARCH_arm
#define PF_EWRITE		(1 << 11)
#else
#define PF_EWRITE		2
#endif
#define PF_EREAD		0
#define PF_EPROTECTION		1
#define PF_ENOTPRESENT		0

/* __bss_stop is not defined in asm-generic/sections.h */
extern char __bss_stop[];

int l4x_do_page_fault(unsigned long address, struct pt_regs *regs, unsigned long error_code);

static inline pte_t *lookup_pte(pgd_t *page_dir, unsigned long address)
{
	/*
	 * this function is basically the same as lookup_address from
	 * pageattr.c and this function looks much easier, so we can
	 * probably adapt this
	 */
	/*
	 * find the page table entry within the page table hierarchy
	 */
	pte_t *pte = NULL;
	pgd_t *pgd = page_dir + pgd_index(address);

#ifdef DEBUG_LOOKUP_PTABS
	if ((int)page_dir < 0x1000) {
		printk("%s: page_dir=%x\n", __func__, (int)page_dir);
		enter_kdebug("page_dir<4096");
	}
#endif
#ifdef DEBUG_LOOKUP_PTABS
	printk("%s: lookup of %lx, pdir = %p", __func__, address, pgd);
#endif
	if (pgd_present(*pgd)) {
		pud_t *pud = pud_offset(pgd, address);
		pmd_t *pmd = pmd_offset(pud, address);
#ifdef DEBUG_LOOKUP_PTABS
		printk(" pmd = %p (%lu) %08lx", pmd, pmd_present(*pmd), pmd_val(*pmd));
#endif
		if (pmd_present(*pmd)) {
#ifdef ARCH_x86
			if (pmd_large(*pmd))
				pte = (pte_t *)pmd;
			else
#endif
				pte = pte_offset_kernel(pmd, address);
		}
	}
#ifdef DEBUG_LOOKUP_PTABS
	printk(" pte = %p\n", pte);
#endif
	return pte;
}

static inline int l4x_pte_read(pte_t pte)
{
#ifdef CONFIG_X86
	return pte_val(pte) & _PAGE_USER;
#else
	return pte_val(pte) & L_PTE_USER;
#endif
}

static inline unsigned long parse_ptabs_read(unsigned long address,
                                             unsigned long *offset)
{
	pte_t *ptep = lookup_pte((pgd_t *)current->mm->pgd, address);

#ifdef DEBUG_PARSE_PTABS_READ
	printk("ppr: pdir: %p, address: %lx, ptep: %p pte: %lx *ptep present: %lu\n", 
	       (pgd_t *)current->active_mm->pgd, address, ptep, pte_val(*ptep), pte_present(*ptep));
#endif

	if ((ptep == NULL) || !pte_present(*ptep)) {
		struct pt_regs regs;
		l4x_make_up_kernel_regs(&regs);
		if (l4x_do_page_fault(address, &regs,
		                      PF_EKERNEL|PF_EREAD|PF_ENOTPRESENT) == -1)
			return -EFAULT;

		if (ptep == NULL)
			ptep = lookup_pte((pgd_t *)current->mm->pgd, address);
		if (!pte_present(*ptep) || !l4x_pte_read(*ptep))
			panic("parse_ptabs_read: pte page still not present\n");
	}
	*ptep   = pte_mkyoung(*ptep);
	*offset = address & ~PAGE_MASK;
	return pte_val(*ptep) & PAGE_MASK;
}

static inline unsigned long parse_ptabs_write(unsigned long address,
                                              unsigned long *offset)
{
	pte_t *ptep = lookup_pte((pgd_t *)current->mm->pgd, address);
	struct pt_regs regs;

	l4x_make_up_kernel_regs(&regs);

#ifdef DEBUG_PARSE_PTABS_WRITE
	printk("ppw: pdir: %p, address: %lx, ptep: %p\n",
	       (pgd_t *)current->mm->pgd, address, ptep);
#endif

	if ((ptep == NULL) || !pte_present(*ptep)) {
		if (l4x_do_page_fault(address, &regs,
		                     PF_EKERNEL|PF_EWRITE|PF_ENOTPRESENT) == -1)
			return -EFAULT;
	} else if (!pte_write(*ptep)) {
		if (l4x_do_page_fault(address, &regs,
		                     PF_EKERNEL|PF_EWRITE|PF_EPROTECTION) == -1)
			return -EFAULT;
	}

	if (ptep == NULL)
		ptep = lookup_pte((pgd_t *)current->mm->pgd, address);

#ifdef DEBUG_PARSE_PTABS_WRITE
	printk("%s %d pte_val = 0x%lx\n", __func__, __LINE__, ptep ?  pte_val(*ptep) : 0);
	if (ptep)
		printk("pte_present(*ptep) = %lx pte_write(*ptep) = %x\n",
		       pte_present(*ptep), pte_write(*ptep));
#endif

	if ((ptep == NULL) || !pte_present(*ptep) || !pte_write(*ptep))
		panic("parse_ptabs_write: pte page still not present or writable\n");

	*ptep   = pte_mkdirty(pte_mkyoung(*ptep));
	*offset = address & ~PAGE_MASK;
	return pte_val(*ptep) & PAGE_MASK;
}

#endif /* ! __ASM_L4__GENERIC__MEMORY_H__ */
