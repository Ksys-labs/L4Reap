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
#include "log.h"
#include "alloc.h"
#include "region.h"
#include "sched_proxy.h"

#include <l4/cxx/ipc_server>
#include <l4/sys/capability>

#include <cstring>

class App_task : public L4::Server_object
{

private:
  L4::Cap<L4::Task> _task;
  L4::Cap<L4::Thread> _thread;


  Region_map _rm;
  Allocator *_alloc;

public:
  Sched_proxy _sched;
  Moe::Log   log;

  App_task();
  Allocator *allocator() const { return _alloc; }
  void set_allocator(Allocator *a) { _alloc = a; }

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);


  Region_map *rm() { return &_rm; }

  void task_cap(L4::Cap<L4::Task> const &c) { _task = c; }
  void thread_cap(L4::Cap<L4::Thread> const &c) { _thread = c; }

  L4::Cap<L4::Task> task_cap() const { return _task; }
  L4::Cap<L4::Thread> thread_cap() const { return _thread; }

  virtual ~App_task();
};
