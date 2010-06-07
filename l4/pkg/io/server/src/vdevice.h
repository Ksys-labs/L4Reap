/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/cxx/avl_set>
//#include <set>
#include <string>
#include <vector>
#include <l4/cxx/ipc_stream>
#include <l4/vbus/vbus_types.h>
#include <cstdio> // dbg

#include "device.h"

namespace Hw {
class Dev_feature;
}

class Adr_resource;

namespace Vi {

class Dev_feature
{
public:
  virtual ~Dev_feature() {}
  virtual bool match_hw_feature(Hw::Dev_feature const *) const = 0;
  virtual int dispatch(l4_umword_t obj, l4_uint32_t func, L4::Ipc_iostream &ios) = 0;
};


class Device : public Generic_device, public Device_tree_mixin<Device>
{
public:
  typedef Device_tree_mixin<Device>::iterator iterator;
  using Device_tree_mixin<Device>::begin;
  using Device_tree_mixin<Device>::end;

  // disptach helper for server object
  int vdevice_dispatch(l4_umword_t obj, l4_uint32_t func, L4::Ipc_iostream &ios);

  typedef std::vector<Dev_feature*> Feature_list;

  Feature_list const *features() const { return &_features; }

  void add_feature(Dev_feature *f)
  { _features.push_back(f); }

  template< typename FT >
  FT *find_feature()
  {
    for (Feature_list::const_iterator i = _features.begin();
	 i != _features.end(); ++i)
      if (FT *r = dynamic_cast<FT*>(*i))
	return r;

    return 0;
  }


  virtual l4_uint32_t adr() const
  { return l4_uint32_t(~0); }

  virtual ~Device()
  { __devs.erase(l4vbus_device_handle_t(this)); }

  char const *name() const
  { return _name.c_str(); }

  bool name(cxx::String const &n)
  {
    _name = std::string(n.start(), n.end());
    return true;
  }

  bool resource_allocated(Resource const *) const;

  virtual void finalize_setup() {}

  Device *parent() const { return _dt.parent(); }
  Device *children() const { return _dt.children(); }
  Device *next() const { return _dt.next(); }
  int depth() const { return _dt.depth(); }

protected:
  // helper functions
  int get_by_hid(L4::Ipc_iostream &ios);
  int vbus_get_device(L4::Ipc_iostream &ios);
  Device *get_dev_by_id(l4vbus_device_handle_t id);


  Device() : _name("(noname)")
  { __devs.insert(l4vbus_device_handle_t(this)); }

  Device *get_root()
  {
    Device *d;
    for (d = this; d->parent(); d = d->parent())
      ;
    return d;
  }

  std::string _name;

  typedef cxx::Avl_set<l4vbus_device_handle_t> Dev_set;
  //typedef std::set<l4vbus_device_handle_t> Dev_set;
  static Dev_set __devs;

private:
  Feature_list _features;
};

}
