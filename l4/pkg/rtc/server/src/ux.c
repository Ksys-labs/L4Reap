/**
 * \file    rtc/server/src/ux.c
 * \brief   Get current time
 *
 * \date    09/26/2003
 * \author  Adam Lackorzynski <adam@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/lxfuxlibc/lxfuxlc.h>

#include <l4/sys/thread.h>
#include <l4/re/env.h>
#include <l4/util/rdtsc.h>
#include <l4/util/kip.h>

#include <time.h>
#include <stdio.h>

#include "rtc.h"

static void printit(void)
{
  time_t t = lx_time(NULL);
  struct tm *r;

  r = gmtime(&t);

  printf("Date:%02d.%02d.%04d Time:%02d:%02d:%02d\n",
         r->tm_mday, r->tm_mon + 1, r->tm_year + 1900,
         r->tm_hour, r->tm_min, r->tm_sec);
}

static int
get_base_time_ux(void)
{
  l4_uint32_t current_s, current_ns;

  l4_tsc_to_s_and_ns(l4_rdtsc(), &current_s, &current_ns);
  system_time_offs_rel_1970 = lx_time(NULL) - current_s;

  printit();

  return 0;
}

get_base_time_func_t init_ux(void)
{
  if (l4util_kip_kernel_is_ux(l4re_kip()))
    {
      l4_thread_control_start();
      l4_thread_control_ux_host_syscall(1);
      l4_thread_control_commit(l4re_env()->main_thread);
      return get_base_time_ux;
    }

  return NULL;
}
