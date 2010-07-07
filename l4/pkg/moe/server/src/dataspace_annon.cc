/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_annon.h"
#include "page_alloc.h"
#include "slab_alloc.h"

#include <l4/cxx/exceptions>

#include <cstring>

typedef Moe::Q_alloc<Moe::Dataspace_annon, Slab_alloc> Allocator;

static Allocator *alloc()
{
  static Allocator a;
  return &a;
}

void *Moe::Dataspace_annon::operator new (size_t, Quota *q)
{
  return alloc()->alloc(q);
}

void Moe::Dataspace_annon::operator delete (void *m) throw()
{ alloc()->free((Moe::Dataspace_annon*)m); }


Moe::Dataspace_annon::Dataspace_annon(unsigned long _size, bool w,
                                      unsigned char page_shift)
: Moe::Dataspace_cont(0, 0, w), _page_shift(page_shift)
{
  unsigned long r_size = (_size + page_size() - 1) & ~(page_size() -1);
  Quota_guard g(quota(), r_size);

  void *m = Single_page_alloc_base::_alloc(r_size, page_size());
  memset(m, 0, r_size);
  start(m);
  size(_size);
  g.done();
}

Moe::Dataspace_annon::~Dataspace_annon()
{
  void *adr = start();
  if (adr)
    {
      unsigned long r_size = (size() + page_size() - 1) & ~(page_size() -1);
      Single_page_alloc_base::_free(adr, r_size);
      quota()->free(r_size);
    }
}
