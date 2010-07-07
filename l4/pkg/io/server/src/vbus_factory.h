/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "main.h"

#include <l4/cxx/avl_map>
#include <string>
#include <cstdio>
#include <typeinfo>

#include "hw_device.h"
#include "vdevice.h"

namespace Vi {

template< typename VI, typename HW >
class Generic_type_factory
{
public:
  virtual VI *vcreate(HW *f) = 0;
  virtual ~Generic_type_factory() {}

  typedef cxx::Avl_map<std::type_info const *, Generic_type_factory *> Type_map;

protected:
  static Type_map &type_map()
  {
    static Type_map _tm;
    return _tm;
  }

public:
  static VI *create(HW *f);
};

typedef Generic_type_factory<Dev_feature, Hw::Dev_feature> Feature_factory;

class Dev_factory
: public Generic_type_factory<Device, Hw::Device>
{
public:
  virtual Device *vcreate() = 0;

  typedef cxx::Avl_map<std::string, Dev_factory *> Name_map;

protected:
  static Name_map &name_map()
  {
    static Name_map _name_map;
    return _name_map;
  }

public:
  using Generic_type_factory<Device, Hw::Device>::create;
  static Device *create(std::string const &_class);
};


template< typename VI,  typename HW_BASE, typename HW, typename BASE >
class Generic_factory_t : public BASE
{
public:
  Generic_factory_t()
  { BASE::type_map()[&typeid(HW)] = this; }


  VI *vcreate(HW_BASE *dev)
  {
#if 0
    if (dev->ref_count())
      printf("WARNING: device '%s' already assigned to an other virtual bus.\n",
             dev->name());
#endif

    VI *d = 0;
    if (HW* h = dynamic_cast<HW*>(dev))
      d = new VI(h);
//    dev->inc_ref_count();
    return d;
  }

  VI *vcreate()
  { return 0; }

};

template< typename VI, typename HW >
class Feature_factory_t
: public Generic_factory_t<VI, Hw::Dev_feature, HW, Feature_factory >
{};

template< typename V_DEV, typename HW_DEV = void >
class Dev_factory_t :  public Dev_factory
{
public:
  typedef HW_DEV Hw_dev;
  typedef V_DEV  V_dev;

  Dev_factory_t()
  {
    type_map()[&typeid(Hw_dev)] = this;
  }


  virtual Device *vcreate(Hw::Device *dev)
  {
    if (dev->ref_count())
      printf("WARNING: device '%s' already assigned to an other virtual bus.\n",
             dev->name());

    Device *d = new V_dev(static_cast<Hw_dev*>(dev));
    dev->inc_ref_count();
    return d;
  }

  virtual Device *vcreate()
  { return 0; }

};

template< typename V_DEV >
class Dev_factory_t<V_DEV, void> :  public Dev_factory
{
public:
  typedef void  Hw_dev;
  typedef V_DEV V_dev;

  explicit Dev_factory_t(std::string const &_class)
  {
    name_map()[_class] = this;
  }


  virtual Device *vcreate(Hw::Device *)
  { return 0; }

  virtual Device *vcreate()
  {
    Device *d = new V_dev();
    return d;
  }

};


template< typename VI, typename HW >
VI *
Generic_type_factory<VI, HW>::create(HW *f)
{
  if (!f)
    return 0;

  Type_map &tm = type_map();
  typename Type_map::const_iterator i = tm.find(&typeid(*f));
  if (i == tm.end())
    {
      printf("WARNING: cannot fabricate buddy object for '%s'\n",
             typeid(*f).name());
      return 0;
    }

    return i->second->vcreate(f);
}

}

