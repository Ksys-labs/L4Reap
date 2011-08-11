/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/re/dataspace>
#include <l4/re/util/region_mapping>
#include <l4/cxx/ipc_server>

#include <l4/re/util/item_alloc>
#include <l4/re/util/cap_alloc>
#include <l4/re/error_helper>

#include "slab_alloc.h"

class Region_ops;

typedef L4Re::Util::Region_handler<L4::Cap<L4Re::Dataspace>, Region_ops> Region_handler;

class Region_ops
{
public:
  typedef L4::Ipc::Snd_fpage Map_result;
  static int map(Region_handler const *h, l4_addr_t addr,
                 L4Re::Util::Region const &r, bool writable,
                 L4::Ipc::Snd_fpage *result);

  static void unmap(Region_handler const *h, l4_addr_t vaddr,
                    l4_addr_t offs, unsigned long size);

  static void free(Region_handler const *h, l4_addr_t start, unsigned long size);

  static void take(Region_handler const *h);
  static void release(Region_handler const *h);
};


class Region_map
: public L4Re::Util::Region_map<Region_handler, Slab_alloc>,
  public L4::Server_object
{
private:
  typedef L4Re::Util::Region_map<Region_handler, Slab_alloc> Base;

public:
  static void global_init();

  Region_map();
  //void setup_wait(L4::Ipc::Istream &istr);
  int handle_pagefault(L4::Ipc::Iostream &ios);
  int handle_rm_request(L4::Ipc::Iostream &ios);
  virtual ~Region_map() {}

  void init();
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);

  void debug_dump(unsigned long function) const;
private:
  int reply_err(L4::Ipc::Iostream &ios);
};


