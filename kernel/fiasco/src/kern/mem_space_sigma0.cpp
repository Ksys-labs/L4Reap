INTERFACE:

#include "mem_space.h"

class Mem_space_sigma0 : public Mem_space
{
};


IMPLEMENTATION:

#include "config.h"


PUBLIC inline
Mem_space_sigma0::Mem_space_sigma0(Ram_quota *q)
: Mem_space(q)
{}

PUBLIC inline
Address
Mem_space_sigma0::virt_to_phys_s0 (void *a) const // pgtble lookup
{
  return (Address)a;
}


PUBLIC
bool
Mem_space_sigma0::v_fabricate(Vaddr address,
                              Phys_addr* phys, Size* size,
                              unsigned* attribs = 0)
{
  // special-cased because we don't do ptab lookup for sigma0
  *size = has_superpages() ? Size(Config::SUPERPAGE_SIZE) : Size(Config::PAGE_SIZE);
  *phys = address.trunc(*size);

  if (attribs)
    *attribs = Page_writable | Page_user_accessible | Page_cacheable;

  return true;
}

PUBLIC inline virtual
Page_number
Mem_space_sigma0::map_max_address() const
{ return Page_number::create(1UL << (MWORD_BITS - Page_shift)); }

