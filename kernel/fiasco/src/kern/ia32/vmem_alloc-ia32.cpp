IMPLEMENTATION[ia32,ux,amd64]:

#include <cassert>
#include <cstdio>
#include <cstring>
#include "config.h"
#include "kdb_ke.h"
#include "kmem.h"
#include "mapped_alloc.h"
#include "mem_layout.h"
#include "mem_unit.h"
#include "paging.h"
#include "static_init.h"
#include "initcalls.h"
#include "space.h"

IMPLEMENT FIASCO_INIT
void
Vmem_alloc::init()
{}

IMPLEMENT
void*
Vmem_alloc::page_alloc (void *address, Zero_fill zf, unsigned mode)
{
  void *vpage = 0;
  Address page;

  vpage = Mapped_allocator::allocator()->alloc(Config::PAGE_SHIFT);

  if (EXPECT_FALSE(!vpage))
    return 0;

  // insert page into master page table
  Pdir::Iter e = Kmem::kdir->walk(Virt_addr(address), 100,
                                  Mapped_allocator::allocator());
  if (EXPECT_FALSE(e.e->valid()))
    {
      kdb_ke("page_alloc: address already mapped");
      goto error;
    }

  if (e.shift() != Config::PAGE_SHIFT)
    goto error;

  if (zf == ZERO_FILL)
    memset(vpage, 0, Config::PAGE_SIZE);

  page = Mem_layout::pmem_to_phys((Address)vpage);

  *e.e = page | Pt_entry::Writable | Pt_entry::Dirty
    | Pt_entry::Valid | Pt_entry::Referenced | Pt_entry::global();
  page_map (address, 0, zf, page);

  if (mode & User)
    e.e->add_attr(Pt_entry::User);

  return address;

error:
  Mapped_allocator::allocator()->free(Config::PAGE_SHIFT, vpage); // 2^0 = 1 page
  return 0;
}

IMPLEMENT inline NEEDS ["paging.h", "mem_layout.h", "mem_unit.h"]
void *
Vmem_alloc::page_unmap (void *page)
{
  Address phys = Kmem::virt_to_phys(page);

  if (phys == (Address) -1)
    return 0;
  
  Address va = reinterpret_cast<Address>(page);
  void *res = (void*)Mem_layout::phys_to_pmem(phys);

  if (va < Mem_layout::Vmem_end)
    {
      // clean out page-table entry
      *(Kmem::kdir->walk(Virt_addr(va)).e) = 0;
      page_unmap (page, 0);
      Mem_unit::tlb_flush(va);
    }

  return res;
}


IMPLEMENT inline NEEDS ["paging.h", "config.h", "kmem.h", "mapped_alloc.h",
			"mem_unit.h"]
void
Vmem_alloc::page_free (void *page)
{
  Address phys = Kmem::virt_to_phys(page);

  if (phys == (Address) -1)
    return;

  // convert back to virt (do not use "page") to get canonic mapping
  Mapped_allocator::allocator()->free(Config::PAGE_SHIFT, // 2^0=1 pages
      Kmem::phys_to_virt(phys));

  Address va = reinterpret_cast<Address>(page);

  if (va < Mem_layout::Vmem_end)
    {
      // clean out page-table entry
      Kmem::kdir->walk(Virt_addr(va)).e->clear();
      page_unmap (page, 0);
      Mem_unit::tlb_flush(va);
    }
}


//----------------------------------------------------------------------------
IMPLEMENTATION[ia32,amd64]:

IMPLEMENT inline
void
Vmem_alloc::page_map (void * /*address*/, int /*order*/, Zero_fill /*zf*/,
		      Address /*phys*/)
{}

IMPLEMENT inline
void
Vmem_alloc::page_unmap (void * /*address*/, int /*order*/)
{}
