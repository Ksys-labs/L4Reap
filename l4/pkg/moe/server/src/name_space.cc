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
#include "globals.h"
#include "string.h"
#include "server_obj.h"

#include <l4/cxx/l4iostream>
#include <l4/cxx/minmax>

#include <cstring>
#include <cstdlib>
#include <cassert>

Moe::Name_space *root_name_space()
{
  static Moe::Name_space _root;
  return &_root;
}

namespace Moe {

static Dbg dbg(Dbg::Name_space, "ns");

Name_space::Name_space()
: L4Re::Util::Names::Name_space(dbg, Err())
{
  object_pool.cap_alloc()->alloc(this);
}

Name_space::~Name_space()
{
  object_pool.cap_alloc()->free(this);
}


static Slab_alloc<Name_space> *alloc()
{
  static Slab_alloc<Name_space> a;
  return &a;
}

void *
Name_space::operator new (size_t) throw()
{
  return alloc()->alloc();
}

void
Name_space::operator delete (void *p, size_t) throw()
{ alloc()->free((Name_space*)p); }

Entry *
Name_space::alloc_dynamic_entry(Names::Name const &name, unsigned flags)
{
  char *na = (char*)GC_MALLOC_ATOMIC(name.len());
  if (!na)
    return 0;

  memcpy(na, name.start(), name.len());
  Names::Name new_name(na, name.len());
  Entry *e = new Moe::Entry(new_name, Names::Obj(flags), true);
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
  delete static_cast<Moe::Entry*>(n);
}

int
Name_space::get_capability(L4::Ipc::Snd_fpage const &cap_fp, L4::Cap<void> *cap,
                           L4::Server_object **lo)
{
  L4::Cap<void> rcv_cap(Rcv_cap << L4_CAP_SHIFT);

  if (cap_fp.id_received())
    {
      L4::Server_object *o = object_pool.find(cap_fp.data());
      if (!o)
	return -L4_EINVAL;

      if (lo)
	*lo = o;

      *cap = o->obj_cap();
      return 0;
    }

  if (cap_fp.cap_received())
    {
      *cap = rcv_cap;
      return 0;
    }

  return -L4_EINVAL;
}

int
Name_space::save_capability(L4::Cap<void> *cap)
{
  L4::Cap<void> rcv_cap(Rcv_cap << L4_CAP_SHIFT);
  if (*cap != rcv_cap)
    return 0;

  L4::Cap<void> nc = object_pool.cap_alloc()->alloc<void>();
  if (!nc.is_valid())
    return -L4_ENOMEM;

  nc.move(rcv_cap);
  *cap = nc;
  return 0;
}

void
Name_space::free_capability(L4::Cap<void> cap)
{
  object_pool.cap_alloc()->free(cap);
}

}

namespace L4Re { namespace Util { namespace Names {

void
Name_space::dump(bool rec, int indent) const
{
  Name_space const *n;
  //L4::cout << "MOE: Name space dump (" << obj_cap() << ")\n";
  for (Const_iterator i = begin(); i != end(); ++i)
    {
      for (int x = 0; x < indent; ++x)
	L4::cout << "  ";

      L4::cout << "  " << i->name()  << " -> " << i->obj()->cap()
               << " o=" << (void*)i->obj()->obj()  << " f="
               << i->obj()->flags() << '\n';
      if (rec && i->obj()->is_valid() 
	  && (n = dynamic_cast<Name_space const *>(i->obj()->obj())))
	n->dump(rec, indent + 1);
    }
}


}}}
