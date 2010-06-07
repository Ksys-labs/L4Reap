//---------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include "mem.h"
#include "pagetable.h"
#include "kmem_space.h"
#include "mapped_alloc.h"
#include "config.h"
#include "panic.h"
#include "kdb_ke.h"
#include "mem_unit.h"
#include "ram_quota.h"
#include "static_init.h"

#include <cstdio>
#include <cassert>
#include <cstring>


IMPLEMENT
void Vmem_alloc::init()
{
  // Allocate a generic zero page
  printf("Vmem_alloc::init()\n");

  if(Config::VMEM_ALLOC_TEST) 
    {
      printf("Vmem_alloc::TEST\n");

      printf("  allocate zero-filled page...");
      void *p = page_alloc((void*)(0xefc01000), ZERO_FILL );
      printf(" [%p] done\n",p);
      printf("  free zero-filled page...");
      page_free(p);
      printf(" done\n");

      printf("  allocate no-zero-filled page...");
      p = page_alloc((void*)(0xefc02000), NO_ZERO_FILL );
      printf(" [%p] done\n",p);
      printf("  free no-zero-filled page...");
      page_free(p);
      printf(" done\n");
    }
}

IMPLEMENT
void *Vmem_alloc::page_alloc(void *address, Zero_fill zf, unsigned mode)
{
  Address page;
  void *vpage;

  vpage = Mapped_allocator::allocator()->alloc(Config::PAGE_SHIFT);
  
  if (EXPECT_FALSE(!vpage)) 
    return 0;

  page = Kmem_space::kdir()->walk(vpage, 0, false, 0).phys(vpage);
  //printf("  allocated page (virt=%p, phys=%08lx\n", vpage, page);
  Mem_unit::inv_dcache(vpage, ((char*)vpage) + Config::PAGE_SIZE);

  // insert page into master page table
  Pte pte = Kmem_space::kdir()->walk(address, Config::PAGE_SIZE, true,
      Ram_quota::root);

  unsigned long pa = Page::CACHEABLE;
  if (mode & User)
    pa |= Page::USER_RW;
  else
    pa |= Page::KERN_RW;

  pte.set(page, Config::PAGE_SIZE, Mem_page_attr(pa), true);

  Mem_unit::dtlb_flush(address);

  if (zf == ZERO_FILL)
    Mem::memset_mwords((unsigned long *)address, 0, Config::PAGE_SIZE >> 2);

  return address;
}

IMPLEMENT
void Vmem_alloc::page_free(void *page)
{
  Pte pte = Kmem_space::kdir()->walk(page, 0, false,0);
  if (!pte.valid())
    return;

  // Invalidate the page because we remove this alias and use the
  // mapped_allocator mapping from now on.
  // Write back is not needed here, because we free the page.
  Mem_unit::inv_vdcache(page, ((char*)page) + Config::PAGE_SIZE);

  Address phys = pte.phys(page);
  pte.set_invalid(0, true);
  Mem_unit::dtlb_flush(page);

  Mapped_allocator::allocator()->free_phys(Config::PAGE_SHIFT, phys);
}

IMPLEMENT
void *Vmem_alloc::page_unmap(void *page)
{
  Pte pte = Kmem_space::kdir()->walk(page, 0, false,0);
  if (!pte.valid())
    return 0;

  Mem_unit::inv_dcache(page, ((char*)page) + Config::PAGE_SIZE);

  Address phys = pte.phys(page);
  pte.set_invalid(0, true);
  Mem_unit::dtlb_flush(page);

  return (void*)Mem_layout::phys_to_pmem(phys);
}
