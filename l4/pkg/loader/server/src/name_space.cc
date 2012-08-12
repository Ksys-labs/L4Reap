/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/iostream>
#include "name_space.h"
#include "debug.h"
#include "obj_reg.h"
#include "global.h"

#include <l4/cxx/minmax>
#include <l4/re/consts>

#include <cstring>
#include <cstdlib>
#include <cassert>

namespace Ldr {

static Dbg dbg(Dbg::Name_space, "ns");

Name_space::Name_space()
: L4Re::Util::Names::Name_space(dbg, Err())
{
  Gate_alloc::registry.register_obj(this);
}

Name_space::~Name_space()
{
  Gate_alloc::registry.unregister_obj(this);
}


Entry *
Name_space::alloc_dynamic_entry(Names::Name const &name, unsigned flags)
{
  char *na = (char*)malloc(name.len());
  if (!na)
    return 0;

  memcpy(na, name.start(), name.len());
  Names::Name new_name(na, name.len());
  Entry *e = new Ldr::Entry(new_name, Names::Obj(flags), true);
  if (e)
    return e;

  free(na);

  return 0;
}

void
Name_space::free_dynamic_entry(Names::Entry *n)
{
  free(const_cast<char*>(n->name().start()));
  assert (!n->next_link());
  delete static_cast<Ldr::Entry*>(n);
}

int
Name_space::get_capability(L4::Ipc::Snd_fpage const &cap_fp, L4::Cap<void> *cap,
                           L4::Server_object **so)
{
  if (cap_fp.id_received())
    {
      L4::Server_object *o = L4::Basic_registry::find(cap_fp.data());
      if (!o)
	return -L4_EINVAL;

      if (so)
	*so = o;

      *cap = o->obj_cap();
      return 0;
    }

  if (cap_fp.cap_received())
    {
      *cap = Glbl::rcv_cap;
      return 0;
    }

  return -L4_EINVAL;
}

int
Name_space::save_capability(L4::Cap<void> *cap)
{
  if (*cap != Glbl::rcv_cap)
    return 0;

  L4::Cap<void> nc = L4Re::Util::cap_alloc.alloc<void>();

  if (!nc.is_valid())
    return -L4_ENOMEM;

  nc.move(Glbl::rcv_cap);
  *cap = nc;
  return 0;
}

void
Name_space::free_capability(L4::Cap<void> cap)
{
  L4Re::Util::cap_alloc.free(cap);
}

}


// dump stuff
#include <l4/cxx/iostream>

namespace L4Re { namespace Util { namespace Names {

void
Name_space::dump(bool rec, int indent) const
{
  Name_space const *n;
  //L4::cout << "loader: Name space dump (" << obj_cap() << ")\n";
  for (Const_iterator i = begin(); i != end(); ++i)
    {
      for (int x = 0; x < indent; ++x)
	L4::cout << "  ";

      L4::cout << "  " << i->name() << " -> "
               << i->obj()->cap().cap()
               << " o=" << (void*)i->obj()->obj()  << " f="
               << i->obj()->flags() << '\n';
      if (rec && i->obj()->is_valid() 
	  && (n = dynamic_cast<Name_space const *>(i->obj()->obj())))
	n->dump(rec, indent + 1);
    }
}


}}}

