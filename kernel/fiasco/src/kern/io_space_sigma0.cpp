INTERFACE [io]:

#include "io_space.h"

template< typename SPACE >
class Io_space_sigma0 : public Generic_io_space<SPACE>
{
  typedef Generic_io_space<SPACE> _B;
public:
  typedef typename _B::Addr Addr;
  typedef typename _B::Size Size;
  typedef typename _B::Phys_addr Phys_addr;
};

IMPLEMENTATION [io]:

//
// Utilities for map<Generic_io_space> and unmap<Generic_io_space>
//

PUBLIC template< typename SPACE >
bool
Io_space_sigma0<SPACE>::v_fabricate(Addr address, Phys_addr* phys,
                                    Size* size, unsigned* attribs = 0)
{
  // special-cased because we don't do lookup for sigma0
  *phys = address.trunc(Size(_B::Map_superpage_size));
  *size = Size(_B::Map_superpage_size);
  if (attribs) *attribs = _B::Page_writable | _B::Page_user_accessible;
  return true;
}

