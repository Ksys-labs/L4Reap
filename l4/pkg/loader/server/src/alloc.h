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
#include <l4/cxx/list>

class Allocator : public L4::Server_object
{
private:
  long _sched_prio_limit;
  l4_umword_t _sched_cpu_mask;

public:
  explicit Allocator(unsigned prio_limit = 0)
  : _sched_prio_limit(prio_limit), _sched_cpu_mask(~0UL)
  {}

  virtual ~Allocator();

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
  int disp_factory(l4_umword_t, L4::Ipc::Iostream &ios);

  static Allocator *root_allocator();
};
