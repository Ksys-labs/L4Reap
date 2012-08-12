/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>

#include <stdio.h>

#include "shared.h"

static int
func_neg_call(L4::Cap<void> const &server, l4_uint32_t *result,
              l4_uint32_t val)
{
  L4::Ipc::Iostream s(l4_utcb());
  s << l4_umword_t(Opcode::func_neg) << val;
  int r = l4_error(s.call(server.cap(), Protocol::Calc));
  if (r)
    return r; // failure
  s >> *result;
  return 0; // ok
}

static int
func_sub_call(L4::Cap<void> const &server, l4_uint32_t *result,
              l4_uint32_t val1, l4_uint32_t val2)
{
  L4::Ipc::Iostream s(l4_utcb());
  s << l4_umword_t(Opcode::func_sub) << val1 << val2;
  int r = l4_error(s.call(server.cap(), Protocol::Calc));
  if (r)
    return r; // failure
  s >> *result;
  return 0; // ok
}

int
main()
{
  L4::Cap<void> server = L4Re::Env::env()->get_cap<void>("calc_server");
  if (!server.is_valid())
    {
      printf("Could not get server capability!\n");
      return 1;
    }

  l4_uint32_t val1 = 8;
  l4_uint32_t val2 = 5;

  printf("Asking for %d - %d\n", val1, val2);

  if (func_sub_call(server, &val1, val1, val2))
    {
      printf("Error talking to server\n");
      return 1;
    }
  printf("Result of substract call: %d\n", val1);
  printf("Asking for -%d\n", val1);
  if (func_neg_call(server, &val1, val1))
    {
      printf("Error talking to server\n");
      return 1;
    }
  printf("Result of negate call: %d\n", val1);

  return 0;
}
