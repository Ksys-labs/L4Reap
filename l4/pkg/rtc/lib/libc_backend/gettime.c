/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/types.h>
#include <l4/libc_backends/clk.h>
#include "gettime.h"

int libc_backend_rt_clock_gettime(struct timespec *tp)
{
  l4_uint32_t s, ns;

  libc_backend_rtc_get_s_and_ns(&s, &ns);

  tp->tv_sec  = s + l4rtc_offset;
  tp->tv_nsec = ns;

  return 0;
}
