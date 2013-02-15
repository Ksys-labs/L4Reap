/*
 * (c) 2008-2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_task.h"

#include <l4/re/parent-sys.h>
#include <l4/re/protocols>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>

#include <l4/cxx/iostream>

using L4Re::Util::cap_alloc;
using L4Re::Dataspace;
using L4Re::chkcap;
using L4Re::chksys;


#if 0
static Slab_alloc<App_task> *alloc()
{
  static Slab_alloc<App_task> a;
  return &a;
}
#endif

#if 0
void *App_task::operator new (size_t) throw()
{ return alloc()->alloc(); }

void App_task::operator delete (void *m) throw()
{ alloc()->free((App_task*)m); }
#endif

int
App_task::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  (void)obj;
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() != L4Re::Protocol::Parent)
    return -L4_EBADPROTO;

  L4::Opcode op;
  ios >> op;
  switch (op)
    {
      case L4Re::Parent_::Signal:
      {
	unsigned long sig;
	unsigned long val;
	ios >> sig >> val;

	switch (sig)
	  {
	  case 0: // exit
	    {
	      // kick the capability refernce
	      // long refs = remove_ref();
	      _state = Zombie;
	      _exit_code = val;

	      terminate();

              if (remove_ref() == 0)
                {
                  delete this;
                  return -L4_ENOREPLY;
                }

              if (l4_cap_idx_t o = observer())
                {
                  observer(0);
                  l4_ipc_send(o, l4_utcb(), l4_msgtag(0,0,0,0), L4_IPC_NEVER);
                }

	      return -L4_ENOREPLY;
	    }
	  default: break;
	  }
	return L4_EOK;
      }
    default:
      return -L4_ENOSYS;
    }
}

App_task::App_task(Ned::Registry *r,
                   L4::Cap<L4::Factory> alloc)
: _ref_cnt(0), _r(r),
  _task(chkcap(cap_alloc.alloc<L4::Task>(), "allocating task cap")),
  _thread(chkcap(cap_alloc.alloc<L4::Thread>(), "allocating thread cap")),
  _rm(chkcap(cap_alloc.alloc<L4Re::Rm>(), "allocating region-map cap")),
  _state(Initializing), _observer(0)
{
  chksys(alloc->create(_rm.get(), L4Re::Protocol::Rm), "allocating new region map");

  _r->register_obj(this);
}

void
App_task::terminate()
{
  _task = L4::Cap_base::Invalid;
  _thread = L4::Cap_base::Invalid;
  _rm = L4::Cap_base::Invalid;

  L4::Cap<void> c = obj_cap();
  _r->unregister_obj(this);
  L4Re::Util::cap_alloc.free(c);
}

App_task::~App_task()
{
  L4::Cap<void> c = obj_cap();
  _r->unregister_obj(this);
  L4Re::Util::cap_alloc.free(c);
}
