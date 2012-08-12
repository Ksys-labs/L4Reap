/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "debug.h"
#include "vbus_factory.h"

namespace Vi {

cxx::H_list<Dev_factory> Dev_factory::_for_type(true);

Dev_factory::Dev_factory(std::type_info const *type) : _type(type)
{
  if (!type)
    return;

  printf("Dev_factory: register factory for %s\n", type->name());
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

Device *
Dev_factory::create(std::string const &_class)
{
  Name_map &m = name_map();
  Name_map::iterator i = m.find(_class);
  if (i == m.end())
    {
      d_printf(DBG_WARN, "WARNING: cannot create virtual device: '%s'\n",
             _class.c_str());
      return 0;
    }

  return i->second->vcreate();
}

Device *
Dev_factory::create(Hw::Device *f, Tagged_parameter *filter, bool warn)
{
  if (!f)
    return 0;

  for (Iterator fa = _for_type.begin(); fa != _for_type.end(); ++fa)
    if (Device *d = fa->vcreate(f, filter))
      return d;

  if (warn)
    d_printf(DBG_WARN, "WARNING: cannot fabricate buddy object for '%s'\n",
             typeid(*f).name());
  return 0;
}

}
