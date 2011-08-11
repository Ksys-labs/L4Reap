/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_task.h"
#include "slab_alloc.h"
#include "globals.h"
#include <l4/re/parent-sys.h>
#include <l4/re/protocols>


using L4Re::Dataspace;


int
App_task::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
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
	      object_pool.cap_alloc()->free(this);
	      if (val != 0)
	        L4::cout << "MOE: task " << obj << " exited with " << val
                         << '\n';

	      GC_gcollect();
	      GC_gcollect_and_unmap();
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

App_task::App_task()
  : _task(L4::Cap<L4::Task>::Invalid),
    _thread(L4::Cap<L4::Thread>::Invalid),
    _alloc(Allocator::root_allocator())
{
  object_pool.cap_alloc()->alloc(&_rm);
  object_pool.cap_alloc()->alloc(&log);
}

App_task::~App_task()
{
  object_pool.cap_alloc()->free(&_rm);
  object_pool.cap_alloc()->free(&log);
  object_pool.cap_alloc()->free(_thread);
  object_pool.cap_alloc()->free(_task);
  object_pool.cap_alloc()->free(&_sched);
}
