/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ipc_server>
#include "quota.h"
#include "server_obj.h"

#include <l4/cxx/list>

namespace Moe {
class Dataspace;
}

class Allocator : public Moe::Server_object, public Moe::Q_object
{
private:
  cxx::List_item _head;
  Moe::Quota _quota;
  long _sched_prio_limit;
  l4_umword_t _sched_cpu_mask;

public:
  explicit Allocator(size_t limit, unsigned prio_limit = 0)
  : _quota(limit), _sched_prio_limit(prio_limit), _sched_cpu_mask(~0UL)
  {}

  Moe::Quota *quota() { return &_quota; }

  Moe::Dataspace *alloc(unsigned long size, unsigned long flags = 0,
                        unsigned long align = 0);

  virtual ~Allocator();

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
  int disp_factory(l4_umword_t, L4::Ipc::Iostream &ios);

  void *operator new (size_t size, Moe::Quota *q, size_t limit);
  void operator delete (void *m) throw();

  L4::Server_object *open(int argc, cxx::String const *argv);
  static Allocator *root_allocator();

};
