/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <gc.h>


template< typename T >
struct Slab_alloc
{
  T *alloc()
  {
    T *t =(T*)GC_MALLOC(sizeof(T));
    if (!t) throw L4::Out_of_memory();
    return t;
  }
  void free(void*) {}

};
