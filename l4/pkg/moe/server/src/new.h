/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once


#include <l4/cxx/slab_alloc.h>
#include <l4/cxx/std_tmpl.h>

#include "page_alloc.h"

template< int Obj_size >
class Slab_alloc_fn : public cxx::Base_slab<Obj_size, L4_PAGESIZE, 2, Single_page_alloc>
{
};

template<int size>
Slab_alloc_fn<size> &__get_slab_allocator(cxx::Int_to_type<size> const &)
{
  static Slab_alloc_fn<size> s;
  return s;
}

inline 
void *operator new (size_t size)
{
  if (size <= sizeof(unsigned long))
    return __get_slab_allocator(cxx::Int_to_type<sizeof(unsigned long)>()).alloc();
}


