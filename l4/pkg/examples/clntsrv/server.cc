/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>

#include "shared.h"

static L4Re::Util::Registry_server<> server;

class Calculation_server : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int
Calculation_server::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  // We're only talking the calculation protocol
  if (t.label() != Protocol::Calc)
    return -L4_EBADPROTO;

  L4::Opcode opcode;
  ios >> opcode;

  switch (opcode)
    {
    case Opcode::func_neg:
      l4_uint32_t val;
      ios >> val;
      val = -val;
      ios << val;
      return L4_EOK;
    case Opcode::func_sub:
      l4_uint32_t val1, val2;
      ios >> val1 >> val2;
      val1 -= val2;
      ios << val1;
      return L4_EOK;
    default:
      return -L4_ENOSYS;
    }
}

int
main()
{
  static Calculation_server calc;

  // Register calculation server
  if (!server.registry()->register_obj(&calc, "calc_server").is_valid())
    {
      printf("Could not register my service, is there a 'calc_server' in the caps table?\n");
      return 1;
    }

  printf("Welcome to the calculation server!\n"
         "I can do substractions and negations.\n");

  // Wait for client requests
  server.loop();

  return 0;
}
