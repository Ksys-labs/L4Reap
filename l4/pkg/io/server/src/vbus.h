/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/avl_set>
#include <l4/cxx/ipc_server>

#include <l4/vbus/vbus_types.h>

#include "vdevice.h"
#include "device.h"

namespace Vi {

class System_bus : public Device, public Dev_feature, public L4::Server_object
{
public:
  System_bus();
  ~System_bus();

  // dispatch for the server object
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  int dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios);
  bool match_hw_feature(Hw::Dev_feature const *) const { return false; }

private:
  int request_resource(L4::Ipc_iostream &ios);
  int request_iomem(L4::Ipc_iostream &ios);

public:
  bool resource_allocated(Resource const *) const;

  void dump_resources() const;

private:
  struct Res_cmp
  {
    bool operator () (Adr_resource const *a, Adr_resource const *b) const
    {
      if (a->type() == b->type())
	return a->_data().end() < b->_data().start();
      return a->type() < b->type();
    }
  };
  //typedef std::set<Adr_resource*, Res_cmp> Resource_set;

public:
  typedef cxx::Avl_set<Adr_resource*, Res_cmp> Resource_set;

  Resource_set const *resource_set() const { return &_resources; }
  Resource_set *resource_set() { return &_resources; }

private:
  Resource_set _resources;
};

}
