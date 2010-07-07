/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "device.h"
#include <l4/cxx/avl_map>

#include <string>
#include <vector>

namespace Hw {

class Device_factory;
class Device;

class Dev_feature
{
public:
  virtual ~Dev_feature() {}
  virtual bool match_cid(cxx::String const &) const { return false; }
  virtual void dump(int) const {}
};

class Dev_if
{
public:
  virtual Device *host() const  = 0;
  virtual ~Dev_if() {}
};

class Discover_bus_if : public virtual Dev_if
{
public:
  virtual void scan_bus() = 0;
  virtual ~Discover_bus_if() {}
};


class Discover_res_if
{
public:
  virtual void discover_resources(Hw::Device *host) = 0;
  virtual void setup_resources(Hw::Device *host) = 0;
  virtual ~Discover_res_if() {}
};


class Device :
  public Generic_device,
  public Device_tree_mixin<Device>
{
private:
  unsigned long _ref_cnt;
  l4_umword_t _uid;
  l4_uint32_t _adr;

public:
  enum Status
  {
    Disabled,
    Active
  };

  enum  Error_codes
  {
    E_ok      = 0,
    E_no_prop = 1,
    E_inval   = 2,
  };

  struct Prop_val
  {
    enum
    {
      Int, String
    } type;
    union
    {
      struct
      {
	char const *s, *e;
      } str;
      l4_uint64_t integer;
    } val;

    Prop_val() : type(Int) {}
    Prop_val(l4_uint64_t i) : type(Int) { val.integer = i; }
    Prop_val(cxx::String const &s) : type(String)
    { val.str.s = s.start(); val.str.e = val.str.s + s.len(); }

    l4_uint64_t get_int() const { return val.integer; }
    cxx::String get_string() const { return cxx::String(val.str.s, val.str.e); }

  };

  unsigned long ref_count() const { return _ref_cnt; }
  void inc_ref_count() { ++_ref_cnt; }
  void dec_ref_count() { --_ref_cnt; }

  Device(l4_umword_t uid, l4_uint32_t adr)
  : _uid(uid), _adr(adr), _sta(Disabled),
    _discover_bus_if(0)
  {}

  Device(l4_uint32_t adr)
  : _uid((l4_umword_t)this), _adr(adr), _sta(Disabled),
    _discover_bus_if(0)
  {}

  Device()
  : _uid((l4_umword_t)this), _adr(~0), _sta(Disabled),
    _discover_bus_if(0)
  {}



  l4_umword_t uid() const { return _uid; }
  l4_uint32_t adr() const { return _adr; }

  virtual int set_property(cxx::String const &prop, Prop_val const &val);

  bool resource_allocated(Resource const *r) const;

  ~Device() {}

  Device *get_child_dev_adr(l4_uint32_t adr, bool create = false);
  Device *get_child_dev_uid(l4_umword_t uid, l4_uint32_t adr, bool create = false);

  Device *parent() const { return _dt.parent(); }
  Device *children() const { return _dt.children(); }
  Device *next() const { return _dt.next(); }
  int depth() const { return _dt.depth(); }


  typedef Device_tree<Device>::iterator iterator;
  using Device_tree_mixin<Device>::begin;
  using Device_tree_mixin<Device>::end;


  typedef std::vector<Dev_feature *> Feature_list;
  Feature_list const *features() const { return &_features; }
  void add_feature(Dev_feature *f)
  { _features.push_back(f); }

  Discover_bus_if *discover_bus_if() const { return _discover_bus_if; }
  void set_discover_bus_if(Discover_bus_if *discover_bus)
  { _discover_bus_if = discover_bus; }

  void add_resource_discoverer(Discover_res_if *dri)
  { _resource_discovery_chain.push_back(dri); }


  void plugin();

  virtual void init();
  virtual void discover_secondary_bus();

  Status status() const { return _sta; }


  void discover_devices();
  void discover_resources();
  void setup_resources();

  void dump(int indent) const;

  char const *name() const { return _name.c_str(); }
  char const *hid() const { return _hid.c_str(); }

  void set_name(char const *name) { _name = name; }
  void set_name(std::string const &name) { _name = name; }
  void set_hid(char const *hid) { _hid = hid; }
  void set_uid(l4_umword_t uid) { _uid = uid; }

  void add_cid(char const *cid) { _cid.push_back(cid); }

  bool match_cid(cxx::String const &cid) const;

private:
  typedef std::vector<std::string> Cid_list;
  typedef std::vector<Discover_res_if *> Discover_res_list;

protected:
  Status _sta;

private:
  std::string _name;
  std::string _hid;
  Cid_list _cid;

  Discover_res_list _resource_discovery_chain;
  Discover_bus_if *_discover_bus_if;
  Feature_list _features;
};

class Device_factory
{
public:
  static void dump();
  static Device *create(cxx::String const &name);
  static void register_factory(char const *name, Device_factory *f)
  { nm()[name] = f; }


  virtual Device *create() = 0;
  virtual ~Device_factory() {}

protected:
  typedef cxx::Avl_map<std::string, Device_factory *> Name_map;

  static Name_map &nm()
  {
    static Name_map _factories;
    return _factories;
  }
};

template< typename T >
class Device_factory_t : public Device_factory
{
public:
  Device_factory_t(char const *name)
  { register_factory(name, this); }
  Device *create() { return new T(); }
  virtual ~Device_factory_t() {}
};


}
