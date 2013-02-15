/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <fnmatch.h>

#include "hw_device.h"
#include "cfg.h"

namespace Hw {


void
Device::init()
{
#if 0
  printf("Hw::Device::plug(this=%p, name='%s', hid='%s')\n",
         this, name(), hid());
#endif
  discover_resources();
  request_resources();
  _sta = Active;
}

void
Device::discover_secondary_bus()
{
#if 0
  printf("Hw::Device::discover_secondary_bus(this=%p, name='%s', hid='%s')\n",
         this, name(), hid());
#endif
  discover_devices();

  for (iterator c = begin(0); c != end(); ++c)
    (*c)->init();

  for (iterator c = begin(0); c != end(); ++c)
    if ((*c)->status())
      (*c)->discover_secondary_bus();
}

void
Device::plugin()
{
  init();
  discover_secondary_bus();
  allocate_pending_resources();
  setup_resources();
}

void
Device_factory::dump()
{
  for (Name_map::const_iterator i = nm().begin(); i != nm().end(); ++i)
    printf("HW: '%s'\n", (*i).first.c_str());
}

Device *
Device_factory::create(cxx::String const &name)
{
  Name_map::const_iterator i = nm().find(std::string(name.start(), name.end()));
  if (i != nm().end())
    return i->second->create();

  return 0;
}

bool
Device::resource_allocated(Resource const *r) const
{
  return r->parent();
}

void
Device::discover_devices()
{
  if (discover_bus_if())
    discover_bus_if()->scan_bus();
}

void
Device::discover_resources()
{
  for (Discover_res_list::const_iterator d = _resource_discovery_chain.begin();
       d != _resource_discovery_chain.end(); ++d)
    (*d)->discover_resources(this);
}

void
Device::setup_resources()
{
  for (Discover_res_list::const_iterator d = _resource_discovery_chain.begin();
       d != _resource_discovery_chain.end(); ++d)
    (*d)->setup_resources(this);

  // take care for children
  for (Hw::Device::iterator i = begin(0); i != end(); ++i)
    i->setup_resources();
}

Device *
Device::get_child_dev_adr(l4_uint32_t adr, bool create)
{
  for (Device *c = children(); c; c = c->next())
    if (c->adr() == adr)
      return c;

  if (!create)
    return 0;

  Device *c = new Device(adr);
  _dt.add_child(c, this);
  return c;
}

Device *
Device::get_child_dev_uid(l4_umword_t uid, l4_uint32_t adr, bool create)
{
  for (Device *c = children(); c; c = c->next())
    if (c->uid() == uid)
      return c;

  if (!create)
    return 0;

  Device *c = new Device(uid, adr);
  _dt.add_child(c, this);
  return c;
}

int
Device::set_property(cxx::String const &prop, Prop_val const &val)
{
  if (prop == "hid")
    {
      if (val.type != Prop_val::String)
	return -E_inval;

      cxx::String const v = val.get_string();
      _hid = std::string(v.start(), v.start() + v.len());
      return E_ok;
    }
  else if (prop == "name")
    {
      if (val.type != Prop_val::String)
	return -E_inval;

      cxx::String const v = val.get_string();
      _name = std::string(v.start(), v.start() + v.len());
      return E_ok;
    }
  else if (prop == "adr")
    {
      if (val.type != Prop_val::Int)
	return -E_inval;

      _adr = val.val.integer;
      return E_ok;
    }
  else
    return -E_no_prop;
}

bool
Device::match_cid(cxx::String const &cid) const
{
    {
      char cid_cstr[cid.len() + 1];
      __builtin_memcpy(cid_cstr, cid.start(), cid.len());
      cid_cstr[cid.len()]  = 0;
      if (!fnmatch(cid_cstr, hid(), 0))
        return true;
    }

  for (Cid_list::const_iterator i = _cid.begin(); i != _cid.end(); ++i)
    if (cid == (*i).c_str())
      return true;

  for (Feature_list::const_iterator i = _features.begin();
       i != _features.end(); ++i)
    if ((*i)->match_cid(cid))
      return true;

  return false;
}

void
Device::dump(int indent) const
{
  printf("%*.sHw::Device[%s]\n", indent, " ", name());
  if (Io_config::cfg->verbose() > 2)
    {
      for (Feature_list::const_iterator i = _features.begin();
           i != _features.end(); ++i)
        (*i)->dump(indent + 2);
    }
}

namespace {
  static Device_factory_t<Device> __hw_pf_factory("Device");
}
}
