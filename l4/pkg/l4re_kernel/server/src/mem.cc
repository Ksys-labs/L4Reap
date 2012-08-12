/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#define _GNU_SOURCE 1  // needed for mremap
#include <sys/mman.h>
#include <errno.h>

#include "page_alloc.h"

#include <l4/cxx/iostream>

void * mmap(void * /*start*/, size_t length, int /*prot*/,
            int flags, int /*fd*/, off_t offset) throw()
{
  char *addr;

  // some checks
  if (offset < 0)
    {
      errno = -EINVAL;
      return MAP_FAILED;
    }
  if (! (flags & MAP_ANON))
    {
      L4::cerr << "mmap() called without MAP_ANON flag, not supported!\n";
      errno = -EINVAL;
      return MAP_FAILED;
    }

  length = (length + (L4_PAGESIZE -1)) & ~(L4_PAGESIZE-1);
  if (length != L4_PAGESIZE)
    {
      errno = ENOMEM;
      return MAP_FAILED;
    }

  addr = Single_page_alloc<char>().alloc();

  if (!addr)
    {
      errno = ENOMEM;
      return MAP_FAILED;
    }

  return addr;
}

int munmap(void * /*start*/, size_t /*length*/) throw()
{
  L4::cout << "munmap() called: unimplemented!\n";
  errno = EINVAL;
  return -1;
}

void * mremap(void * /*old_address*/, size_t /*old_size*/, size_t /*new_size*/,
	      int /*may_move*/, ...) throw()
{
  L4::cout << "mremap() called: unimplemented!\n";
  errno = EINVAL;
  return MAP_FAILED;
}
