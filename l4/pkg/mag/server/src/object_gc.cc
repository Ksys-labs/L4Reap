// vi:ft=cpp
/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "object_gc.h"
#include <cstdio>

namespace Mag_server {

namespace {
  class Trash : public L4::Server_object
  {
    int dispatch(l4_umword_t, L4::Ipc::Iostream &)
    {
      printf("GOT: stale request, drop it\n");
      return -L4_EINVAL;
    }
  };

  static Trash _trash;
}

void
Object_gc::gc_step()
{
  printf("GC: step this=%p _life = %p\n", this, _life);
  Object *n = _life;
  while (n)
    {
      Object *o = n;
      n = o->_n;

      if (!o->obj_cap() || !o->obj_cap().validate().label())
	{
	  printf("GC: object=%p\n", o);
	  L4::Thread::Modify_senders todo;
	  todo.add(~3UL, reinterpret_cast<l4_umword_t>((L4::Server_object*)o),
	           ~0UL, reinterpret_cast<l4_umword_t>((L4::Server_object*)&_trash));
	  L4::Cap<L4::Thread>()->modify_senders(todo);
	  o->dequeue();
	  o->destroy();
	  // drop the reference held in the IPC gate
	  cxx::Ref_ptr<Object> p(o, true);
	}
    }
}

void
Object_gc::gc_sweep()
{
  while (_sweep)
    {
      Object *o = _sweep;
      o->dequeue();
      printf("GC: delete object %p\n", o);
      delete o;
    }
}

}
