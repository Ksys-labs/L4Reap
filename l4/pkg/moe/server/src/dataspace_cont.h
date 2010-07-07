/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace.h"

namespace Moe {

class Dataspace_cont : public Dataspace
{
public:
  Dataspace_cont(void *start, unsigned long size, unsigned long flags = 0);

  Address address(l4_addr_t offset,
                  Ds_rw rw, l4_addr_t hot_spot = 0,
                  l4_addr_t min = 0, l4_addr_t max = ~0) const;

  unsigned long page_shift() const throw() { return L4_LOG2_PAGESIZE; }

  void unmap(bool ro = false) const throw();

  int phys(l4_addr_t offset, l4_addr_t &phys_addr, l4_size_t &phys_size) throw();

  virtual ~Dataspace_cont() {}


protected:
  void start(void *start) { _start = (char*)start; }
  void *start() { return _start; }

private:
  char *_start;
};

};

