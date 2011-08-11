/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
/*
 * Message handler/dispatcher
 */

#include "debug.h"
#include "dispatcher.h"
#include "globals.h"

#include <l4/cxx/iostream> // for L4::cout

#include <l4/re/protocols>

#include <cstring> // for memcpy

static Dbg dbg(Dbg::Server, "svr");

int
Dispatcher::handle_exception(L4::Ipc::Iostream &ios)
{
  l4_exc_regs_t u = *l4_utcb_exc();

  Dbg(Dbg::Warn).printf("unhandled exception: pc=0x%lx\n", l4_utcb_exc_pc(&u));
  ios << 0UL << 0UL;
  return -L4_ENOREPLY;
}

int
Dispatcher::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  dbg.printf("request: tag=0x%lx proto=%ld obj=0x%lx\n", t.raw, t.label(), obj);

  switch (t.label())
    {
    case L4Re::Protocol::Rm:
      return Global::local_rm->handle_rm_request(ios);

    case L4Re::Protocol::Debug:
      {
        unsigned long func;
        ios >> func;
        Global::local_rm->debug_dump(func);
        return L4_EOK;
      }
    case L4_PROTO_EXCEPTION:
      return handle_exception(ios);

    case L4_PROTO_PAGE_FAULT:
      return Global::local_rm->handle_pagefault(ios);

    default:
      dbg.printf("unknown request: tag=0x%lx proto=%ld obj=0x%lx\n",
                  t.raw, t.label(), obj);
      ios << 0UL << 0UL;
      return -L4_ENOSYS;
    };
}
