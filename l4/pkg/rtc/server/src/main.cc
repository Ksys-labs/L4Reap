/**
 * \file    rtc/server/src/main.cc
 * \brief   Initialization and main server loop
 *
 * \date    09/23/2003
 * \author  Frank Mehnert <fm3@os.inf.tu-dresden.de>
 * \author  Adam Lackorzynski <adam@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/rtc/rtc.h>
#if defined ARCH_x86 || defined ARCH_amd64
#include <l4/util/rdtsc.h>
#endif
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/cxx/exceptions>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/sys/factory>
#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include <l4/sys/ipc_gate>

#include <cstdlib>
#include <cstdio>

#include "rtc.h"

l4_uint32_t system_time_offs_rel_1970;

static int
l4rtc_if_get_offset_component(l4_uint32_t *offset)
{
#if defined ARCH_x86 || defined ARCH_amd64
  *offset = system_time_offs_rel_1970;
  return 0;
#endif
  *offset = 0;
  return 1;
}

static int
l4rtc_if_get_linux_tsc_scaler_component(l4_uint32_t *scaler)
{
#if defined ARCH_x86 || defined ARCH_amd64
  *scaler = l4_scaler_tsc_linux;
  return 0;
#endif
  *scaler = 0;
  return 1;
}

class Rtc_dispatcher
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int
Rtc_dispatcher::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  if (t.label() != 0)
    return -L4_EBADPROTO;

  l4_umword_t opcode;
  ios >> opcode;

  switch (opcode)
    {
    case L4RTC_OPCODE_get_offset:
      l4_uint32_t offset;
      l4rtc_if_get_offset_component(&offset);
      ios << offset;
      return L4_EOK;
    case L4RTC_OPCODE_get_linux_tsc_scaler:
      l4_uint32_t scaler;
      l4rtc_if_get_linux_tsc_scaler_component(&scaler);
      ios << scaler;
      return L4_EOK;
    default:
      return -L4_ENOSYS;
    }
}

static L4::Server<> server(l4_utcb());

int
main()
{
  get_base_time_func_t get_base_time;

#if defined ARCH_x86 || defined ARCH_amd64
#ifdef ARCH_x86
  if (!(get_base_time = init_ux()))
#endif
    if (!(get_base_time = init_x86()))
#endif
      {
	printf("Initialization failed, exiting\n");
	exit(1);
      }

  try
    {
#if defined ARCH_x86 || defined ARCH_amd64
      l4_calibrate_tsc(l4re_kip());
#endif
      if (get_base_time())
        return 1;

      L4::Cap<L4::Ipc_gate> cap;
      cap = L4Re::chkcap(L4Re::Env::env()->get_cap<L4::Ipc_gate>("rtc"),
                         "Did not find server cap 'rtc'");
      L4Re::chksys(cap->bind_thread(L4Re::Env::env()->main_thread(), 0x1230),
                   "bind to rtc server gate");
    }
  catch (L4::Runtime_error const &e)
    {
      L4::cerr << e << "TERMINATED\n";
      abort();
    }

  server.loop(Rtc_dispatcher());
}
