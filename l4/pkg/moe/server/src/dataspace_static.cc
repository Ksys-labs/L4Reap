/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_static.h"
#include "dataspace.h"
#include "slab_alloc.h"

#include <l4/cxx/exceptions>

static Slab_alloc<Moe::Dataspace_static> *alloc()
{
  static Slab_alloc<Moe::Dataspace_static> a;
  return &a;
}

void *Moe::Dataspace_static::operator new (size_t)
{
  return alloc()->alloc();
}

void Moe::Dataspace_static::operator delete (void *m) throw()
{ alloc()->free((Moe::Dataspace_static*)m); }

