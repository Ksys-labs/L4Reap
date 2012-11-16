/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/re/error_helper>

#include <l4/cxx/ipc_server>

#include "server.h"


L4::Cap<void> rcv_cap = L4Re::Util::cap_alloc.alloc<void>();

class Loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
public:
  static void setup_wait(L4::Ipc::Istream &istr, L4::Ipc_svr::Reply_mode)
  {
    istr.reset();
    istr << L4::Ipc::Small_buf(rcv_cap.cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br()->bdr = 0;
  }

};

static L4Re::Util::Registry_server<Loop_hooks> server;

L4Re::Util::Object_registry *registry = server.registry();

int server_loop()
{
  server.loop();
  return 0;
}

