/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/ipc.h>
#include <stdio.h>
#include <l4/re/l4aux.h>
#include <l4/re/error_helper>
#include <l4/sys/scheduler>
#include <l4/sys/thread>
#include <l4/cxx/iostream>

#include <typeinfo>

#include "debug.h"
#include "region.h"
#include "global.h"
#include "log.h"
#include "obj_reg.h"
#include "alloc.h"

using L4Re::chksys;

static Dbg info(Dbg::Info);
static Dbg boot_info(Dbg::Boot);

L4::Cap<void> Glbl::rcv_cap;

namespace Gate_alloc {
  L4Re::Util::Object_registry registry;
}

class Loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
public:
  static void setup_wait(L4::Ipc::Istream &istr, bool before_reply)
  {
    (void)before_reply;
    istr.reset();
    istr << L4::Ipc::Small_buf(Glbl::rcv_cap.cap(),  L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br()->bdr = 0;
  }
};

l4re_aux_t* l4re_aux;
template< typename Reg >
class My_dispatcher
{
private:
  Reg r;

public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
  {
    l4_msgtag_t tag;
    ios >> tag;
    typename Reg::Value *o = 0;

    Dbg dbg(Dbg::Server);

    dbg.printf("tag=%lx (proto=%ld) obj=%lx", tag.raw,
               tag.label(), obj);

    if (tag.is_exception())
      {
	dbg.cprintf("\n");
	Dbg(Dbg::Warn).printf("unhandled exception...\n");
	return -L4_ENOREPLY;
      }
    else
      {
	o = r.find(obj & ~3UL);

	if (!o)
	  {
	    dbg.cprintf(": invalid object\n");
	    return -L4_ENOENT;
	  }

	dbg.cprintf(": object is a %s\n", typeid(*o).name());
	int res = o->dispatch(obj, ios);
	dbg.printf("reply = %d\n", res);
	return res;
      }

    Dbg(Dbg::Warn).printf("Invalid message (tag.label=%ld)\n", tag.label());
    return -L4_ENOSYS;
  }

};


static L4::Server<Loop_hooks> server(l4_utcb());

int
main(int argc, char **argv)
{
  try {
  Dbg::set_level(0); //Dbg::Info | Dbg::Boot);

  info.printf("Hello from loader\n");
  boot_info.printf("cmdline: ");
  for (int i = 0; i < argc; ++i)
    boot_info.cprintf("'%s'", argv[i]);
  boot_info.cprintf("\n");

  l4_umword_t *auxp = (l4_umword_t*)&argv[argc] + 1;
  while (*auxp)
    ++auxp;
  ++auxp;

  l4re_aux = 0;

  while (*auxp)
    {
      info.printf("aux: %lx\n", *auxp);
      if (*auxp == 0xf0)
	l4re_aux = (l4re_aux_t*)auxp[1];
      auxp += 2;
    }

  Glbl::rcv_cap = L4Re::Util::cap_alloc.alloc<void>();
  Region_map::global_init();


  L4Re::chkcap(Gate_alloc::registry.register_obj(Allocator::root_allocator(), "loader"));

  server.loop(My_dispatcher<L4::Basic_registry>());
  }
  catch (L4::Runtime_error const &e)
    {
      L4::cout << "Got exception\n";
      L4::cout << e << "\n";
    }
  return 1;
}
