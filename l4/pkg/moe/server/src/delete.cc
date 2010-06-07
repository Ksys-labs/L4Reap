/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/iostream>
#include <cstddef>


void operator delete(void *p) throw()
{
  L4::cerr << "FATAL: called delete " << p 
    << " =============================\n";
}

void * operator new(size_t s) throw()
{ 
  L4::cerr << "FATAL: called new " << s 
    << " ==============================\n";
  return 0;
}
