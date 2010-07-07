/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/exceptions>
#include <l4/sys/task.h>
#include <l4/sys/factory.h>
#include <l4/sys/types.h>

#include <l4/re/util/bitmap_cap_alloc>

#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream>

#include "server_obj.h"

#include <cstring>
#include <cassert>
#include <cstdio>

#define DEBUG_CAP_ALLOC 0

enum
{
  Rcv_cap = 0x100,
};

class Cap_alloc;

class Object_pool : public L4::Basic_registry
{
public:
  explicit Object_pool(Cap_alloc *ca) : _cap_alloc(ca) {}
  Cap_alloc *cap_alloc() const { return _cap_alloc; }
private:
  Cap_alloc *_cap_alloc;
};

class Cap_alloc
{
public:
  enum
  {
    Non_gc_caps = 1024,
    Gc_caps     = 4096,

    Non_gc_cap_0 = Rcv_cap,
    Gc_cap_0     = Non_gc_cap_0 + Non_gc_caps,
  };

private:
  // caps mainly used for things from outside (registered in name spaces)
  // this are usually not a lot
  L4Re::Util::Cap_alloc<Non_gc_caps> _non_gc;

  // caps used for local objects, these caps are scanned for garbage
  // collection
  L4Re::Util::Cap_alloc<Gc_caps> _gc;

public:

  L4::Cap<L4::Kobject> alloc()
  {
     L4::Cap<L4::Kobject> cap = _non_gc.alloc<L4::Kobject>();
#if DEBUG_CAP_ALLOC
     L4::cerr << "AC->" << L4::n_hex(cap.cap()) << "\n";
#endif
     return cap;
  }

  template< typename T >
  L4::Cap<T> alloc() { return L4::cap_cast<T>(alloc()); }

  L4::Cap<L4::Kobject> alloc(L4::Server_object *o)
  {
    L4::Cap<L4::Kobject> cap = _gc.alloc<L4::Kobject>();
    //printf("so @ %lx (%lx)\n", cap.cap(), (cap.cap() >> L4_CAP_SHIFT) - Gc_cap_0);
#if  DEBUG_CAP_ALLOC
    L4::cerr << "ACO->" << L4::n_hex(cap.cap()) << "\n";
#endif
    if (!cap.is_valid())
      throw(L4::Out_of_memory());

    l4_umword_t id = l4_umword_t(o);
    l4_factory_create_gate(L4_BASE_FACTORY_CAP, cap.cap(),
                           L4_BASE_THREAD_CAP, id);
    o->obj_cap(cap);
    return cap;
  }

  bool free(L4::Cap<void> const &cap)
  {
    if (!cap.is_valid())
      return false;

    _non_gc.free(cap, L4_BASE_TASK_CAP);
    return true;
  }

  bool free(L4::Server_object *o)
  {
    if (!o)
      return false;

    if (!o->obj_cap().is_valid())
      return false;

    _gc.free(o->obj_cap());

    o->obj_cap(L4::Cap<void>::Invalid);

    return true;
  }

  bool is_allocated(L4::Cap<void> c)
  { return _gc.is_allocated(c); }

  Cap_alloc() : _non_gc(Non_gc_cap_0), _gc(Gc_cap_0)
  {
    L4::Cap<void> rcv_cap = _non_gc.alloc();
    (void)rcv_cap;
    assert (L4::Cap<void>(Rcv_cap << L4_CAP_SHIFT) == rcv_cap);
  }

  long hint() const { return _gc.hint(); }

};


