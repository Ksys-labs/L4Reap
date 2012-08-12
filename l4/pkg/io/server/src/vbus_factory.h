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
#include "debug.h"

#include <l4/cxx/avl_map>
#include <l4/cxx/hlist>
#include <string>
#include <typeinfo>

#include "hw_device.h"
#include "vdevice.h"

#include "tagged_parameter.h"

namespace Vi {

template< typename VI, typename HW >
class Generic_type_factory
: public cxx::H_list_item
{
private:
  typedef Generic_type_factory<VI, HW> Self;
  typedef cxx::H_list<Self> List;
  typedef typename List::Iterator Iterator;

  static List _for_type;

public:
  virtual VI *vcreate(HW *f) = 0;
  virtual ~Generic_type_factory() {}

  static VI *create(HW *f, bool warn = true);

protected:
  explicit Generic_type_factory(std::type_info const *type);

private:
  std::type_info const *_type;
};

template< typename VI, typename HW >
cxx::H_list<Generic_type_factory<VI,HW> > Generic_type_factory<VI,HW>::_for_type(true);

typedef Generic_type_factory<Dev_feature, Hw::Dev_feature> Feature_factory;
typedef Generic_type_factory<Resource, Resource> Resource_factory;

class Dev_factory : public cxx::H_list_item
{
public:
  virtual Device *vcreate() = 0;
  virtual Device *vcreate(Hw::Device *f, Tagged_parameter *filter) = 0;

//  typedef cxx::Avl_map<std::type_info const *, Dev_factory *> Type_map;
  typedef cxx::Avl_map<std::string, Dev_factory *> Name_map;
  typedef cxx::H_list<Dev_factory> List;
  typedef List::Iterator Iterator;

  static List _for_type;
  std::type_info const *_type;

protected:
  explicit Dev_factory(std::type_info const *type);

  static Name_map &name_map()
  {
    static Name_map _name_map;
    return _name_map;
  }

public:
  static Device *create(std::string const &_class);
  static Device *create(Hw::Device *f, Tagged_parameter *filter, bool warn = true);

private:
  Dev_factory(Dev_factory const &);
  void operator = (Dev_factory const &);
};


template< typename VI,  typename HW_BASE, typename HW, typename BASE >
class Generic_factory_t : public BASE

{
public:

  Generic_factory_t() : BASE(&typeid(HW)) {}

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

template< typename VI, typename HW >
class Resource_factory_t
: public Generic_factory_t<VI, Resource, HW, Resource_factory >
{};

template< typename V_DEV, typename HW_DEV = void >
class Dev_factory_t :  public Dev_factory
{
public:
  typedef HW_DEV Hw_dev;
  typedef V_DEV  V_dev;

  Dev_factory_t() : Dev_factory(&typeid(Hw_dev))
  { }


  virtual Device *vcreate(Hw::Device *dev, Tagged_parameter *filter)
  {
    if (dev->ref_count())
      printf("WARNING: device '%s' already assigned to an other virtual bus.\n",
             dev->name());

    if (!dynamic_cast<HW_DEV const*>(dev))
      return 0;

    Device *d = new V_dev(static_cast<Hw_dev*>(dev), filter);
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

  explicit Dev_factory_t(std::string const &_class) : Dev_factory(0)
  {
    name_map()[_class] = this;
  }


  virtual Device *vcreate(Hw::Device *, Tagged_parameter *)
  { return 0; }

  virtual Device *vcreate()
  {
    Device *d = new V_dev();
    return d;
  }

};

template< typename VI, typename HW >
Generic_type_factory<VI, HW>::Generic_type_factory(std::type_info const *type)
: _type(type)
{
  if (!type)
    return;

  printf("GTF: register factory for %s\n", type->name());

  Iterator i = _for_type.end();
  for (Iterator c = _for_type.begin(); c != _for_type.end(); ++c)
    {
      void *x = 0;
      // use the compiler catch logic to figure out if TYPE
      // is a base class of c->_type, if it is we must put
      // this behind c in the list.
      if (type->__do_catch(c->_type, &x, 0))
	i = c;
    }

  _for_type.insert(this, i);
}

template< typename VI, typename HW >
VI *
Generic_type_factory<VI, HW>::create(HW *f, bool warn)
{
  if (!f)
    return 0;

  for (Iterator c = _for_type.begin(); c != _for_type.end(); ++c)

    {
      VI *v = c->vcreate(f);
      if (v)
	return v;
    }

  if (warn)
    d_printf(DBG_WARN, "WARNING: cannot fabricate buddy object for '%s'\n",
             typeid(*f).name());
  return 0;
}

}

