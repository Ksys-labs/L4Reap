#ifndef __ARCH_IOREMAP_C_INCLUDED__
#error Do not compile this file directly.
#endif

#include <asm/generic/io.h>
#include <l4/sys/kdebug.h>

#define MAX_IOREMAP_ENTRIES 20
struct ioremap_table {
	unsigned long real_map_addr;
	unsigned long ioremap_addr;
	unsigned long phys_addr;
	unsigned long size;
};

static struct ioremap_table io_table[MAX_IOREMAP_ENTRIES];
static int ioremap_table_initialized = 0;

static DEFINE_SPINLOCK(ioremap_lock);

static void reset_ioremap_entry_nocheck(int entry)
{
	io_table[entry] = (struct ioremap_table){0, 0, 0, 0};
}

static void init_ioremap_nocheck(void)
{
	int i;
	for (i = 0; i < MAX_IOREMAP_ENTRIES; i++)
		reset_ioremap_entry_nocheck(i);
	ioremap_table_initialized = 1;
}

static int set_ioremap_entry(unsigned long real_map_addr,
                             unsigned long ioremap_addr,
			     unsigned long phys_addr,
			     unsigned long size)
{
	int i;

	spin_lock(&ioremap_lock);

	if (!ioremap_table_initialized)
		init_ioremap_nocheck();

	for (i = 0; i < MAX_IOREMAP_ENTRIES; i++)
		if (io_table[i].real_map_addr == 0) {
			io_table[i] = (struct ioremap_table){real_map_addr,
			                                     ioremap_addr,
			                                     phys_addr,
			                                     size};
			spin_unlock(&ioremap_lock);
			return 0;
		}

	spin_unlock(&ioremap_lock);
	printk("no free entry in ioremaptable\n");
	BUG();
	return 1;
}

static int __lookup_ioremap_entry_phys(unsigned long phys_addr)
{
	int i;

	if (!ioremap_table_initialized)
		return -1;

	spin_lock(&ioremap_lock);

	for (i = 0; i < MAX_IOREMAP_ENTRIES; i++)
		if ((io_table[i].phys_addr <= phys_addr) &&
		    io_table[i].phys_addr + io_table[i].size > phys_addr)
			break;

	spin_unlock(&ioremap_lock);
	return i == MAX_IOREMAP_ENTRIES ? -1 : i;
}

unsigned long find_ioremap_entry(unsigned long phys_addr)
{
	int i;
	if ((i = __lookup_ioremap_entry_phys(phys_addr)) == -1)
		return 0;

	return io_table[i].ioremap_addr + (phys_addr - io_table[i].phys_addr);
}

static int remove_ioremap_entry_phys(unsigned long phys_addr)
{
	int i;
	if ((i = __lookup_ioremap_entry_phys(phys_addr)) == -1)
		return -1;

	spin_lock(&ioremap_lock);
	reset_ioremap_entry_nocheck(i);
	spin_unlock(&ioremap_lock);
	return 0;
}

#ifdef CONFIG_L4
static unsigned long lookup_phys_entry(unsigned long ioremap_addr,
                                       unsigned long *size)
{
	int i;

	if (!ioremap_table_initialized)
		return 0;

	spin_lock(&ioremap_lock);

	for (i = 0; i < MAX_IOREMAP_ENTRIES; i++)
		if (io_table[i].ioremap_addr == ioremap_addr) {
			*size = io_table[i].size;
			spin_unlock(&ioremap_lock);
			return io_table[i].phys_addr;
		}

	spin_unlock(&ioremap_lock);
	return 0;
}

static inline unsigned long get_iotable_entry_size(int i)
{
	return io_table[i].size;
}

static inline unsigned long get_iotable_entry_ioremap_addr(int i)
{
	return io_table[i].ioremap_addr;
}

static inline unsigned long get_iotable_entry_phys(int i)
{
	return io_table[i].phys_addr;
}

#else

static unsigned long lookup_ioremap_entry(unsigned long ioremap_addr)
{
	int i;
	unsigned long result = 0;

	if (!ioremap_table_initialized)
		return 0;

	spin_lock(&ioremap_lock);

	for (i = 0; i < MAX_IOREMAP_ENTRIES; i++)
		if (io_table[i].ioremap_addr == ioremap_addr) {
			result = io_table[i].real_map_addr;
			break;
		}

	spin_unlock(&ioremap_lock);
	return result;
}

static inline void remap_area_pte(pte_t * pte, unsigned long address, unsigned long size,
	unsigned long phys_addr, unsigned long flags)
{
	unsigned long end;
	unsigned long pfn;

	address &= ~PMD_MASK;
	end = address + size;
	if (end > PMD_SIZE)
		end = PMD_SIZE;
	if (address >= end)
		BUG();
	pfn = phys_addr >> PAGE_SHIFT;
	do {
		if (!pte_none(*pte)) {
			printk("remap_area_pte: page already exists\n");
			BUG();
		}
		set_pte(pte, pfn_pte(pfn, __pgprot(_PAGE_PRESENT | _PAGE_RW |
					_PAGE_DIRTY | _PAGE_ACCESSED | flags)));
		address += PAGE_SIZE;
		pfn++;
		pte++;
	} while (address && (address < end));
}

static inline int remap_area_pmd(pmd_t * pmd, unsigned long address, unsigned long size,
	unsigned long phys_addr, unsigned long flags)
{
	unsigned long end;

	address &= ~PGDIR_MASK;
	end = address + size;
	if (end > PGDIR_SIZE)
		end = PGDIR_SIZE;
	phys_addr -= address;
	if (address >= end)
		BUG();
	do {
		pte_t * pte = pte_alloc_kernel(pmd, address);
		if (!pte)
			return -ENOMEM;
		remap_area_pte(pte, address, end - address, address + phys_addr, flags);
		address = (address + PMD_SIZE) & PMD_MASK;
		pmd++;
	} while (address && (address < end));
	return 0;
}

static int remap_area_pages(unsigned long address, unsigned long phys_addr,
				 unsigned long size, unsigned long flags)
{
	int error;
	pgd_t * dir;
	unsigned long end = address + size;

	phys_addr -= address;
	dir = pgd_offset(&init_mm, address);
	flush_cache_all();
	if (address >= end)
		BUG();
	do {
		pud_t *pud;
		pmd_t *pmd;

		error = -ENOMEM;
		pud = pud_alloc(&init_mm, dir, address);
		if (!pud)
			break;
		pmd = pmd_alloc(&init_mm, pud, address);
		if (!pmd)
			break;
		if (remap_area_pmd(pmd, address, end - address,
					 phys_addr + address, flags))
			break;
		error = 0;
		address = (address + PGDIR_SIZE) & PGDIR_MASK;
		dir++;
	} while (address && (address < end));
	flush_tlb_all();
	return error;
}

#endif




static inline void __iomem *
__l4x_ioremap(unsigned long phys_addr, size_t size, unsigned long flags)
{
	void __iomem * addr;
	l4_addr_t reg_start, reg_len;
	unsigned long last_addr;
	unsigned long offset;
	int i;
	L4XV_V(f);

	/*
	 * Don't allow wraparound or zero size
	 */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/*
	 * If userland applications like X generate page faults on
	 * I/O memory region we do not know how big the region really is.
	 * l4io is requesting at least 8M virtual address space for every
	 * l4io_request_mem_region call so that we cannot get a continuous
	 * region with multiple page faults to the same region and different
	 * pages. That's why we first request the size of the region and
	 * then request the whole region at once.
	 */
	//printk("%s: Requested region at %08lx [0x%x Bytes]\n",
	//       __func__, phys_addr, size);

	if ((i = __lookup_ioremap_entry_phys(phys_addr)) != -1) {
		/* Found already existing entry */
		offset = phys_addr - get_iotable_entry_phys(i);
		if (get_iotable_entry_size(i) - offset >= size)
			/* size is within this area, return */
			return (void __iomem *)
				(get_iotable_entry_ioremap_addr(i) + offset);
	}

	L4XV_L(f);

	if (!l4io_has_resource(L4IO_RESOURCE_MEM, phys_addr,
	                       phys_addr + size - 1)) {
		printk("ERROR: IO-memory (%lx+%zx) not available\n",
		       phys_addr, size);
		L4XV_U(f);
		return NULL;
	}

	if ((i = l4io_search_iomem_region(phys_addr, size, &reg_start, &reg_len))) {
		printk("ioremap: No region found for %lx: %d\n", phys_addr, i);
		L4XV_U(f);
		return NULL;
	}

	if ((i = l4io_request_iomem(reg_start, reg_len,
	                            0, (l4_addr_t *)&addr))) {
		printk("ERROR: l4io_request_iomem error(%lx+%lx): %d\n",
		       reg_start, reg_len, i);
		L4XV_U(f);
		return NULL;
	}
	L4XV_U(f);
	offset = 0;

	/* Save whole region */
	set_ioremap_entry((unsigned long)addr,
	                  (unsigned long)addr,
	                  reg_start,
	                  reg_len);

	offset += phys_addr - reg_start;

	printk("%s: Mapping physaddr %08lx [0x%zx Bytes, %08lx+%06lx] "
	       "to %08lx+%06lx\n",
	       __func__, phys_addr, size, reg_start, reg_len,
	       (unsigned long)addr, offset);

	return (void __iomem *) (offset + (char *)addr);
}

static inline void
l4x_iounmap(volatile void __iomem *addr)
{
	unsigned long size;
	unsigned long phys_addr;
	L4XV_V(f);

	if (addr <= high_memory)
		return;

	if ((phys_addr = lookup_phys_entry((unsigned long)addr, &size)) == 0) {
		printk("%s: Error unmapping addr %p\n", __func__, addr);
		return;
	}

	if (remove_ioremap_entry_phys(phys_addr) == -1)
		printk("%s: could not find address to unmap\n", __func__);

	L4XV_L(f);
	if (l4io_release_iomem((l4_addr_t)addr, size))
		printk("iounmap: error calling l4io_release_mem_region, not freed");
	L4XV_U(f);
}
