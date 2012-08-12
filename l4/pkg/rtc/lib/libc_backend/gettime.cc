/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/types.h>
#include <l4/libc_backends/clk.h>
#include <l4/rtc/rtc.h>
#include <l4/crtn/initpriorities.h>

#include <cstdio>

#include "gettime.h"

namespace {

struct Rtc_be
{
  l4_uint32_t offset;
  Rtc_be()
  {
    libc_backend_rtc_init();

    int ret = l4rtc_get_offset_to_realtime(&offset);

    // error, assume offset 0
    if (ret)
      {
        printf("RTC server not found, assuming 1.1.1970, 0:00 ...\n");
        offset = 0;
      }
  }
};

static Rtc_be _rtc_be __attribute__((init_priority(INIT_PRIO_RTC_L4LIBC_INIT)));

}

int libc_backend_rt_clock_gettime(struct timespec *tp)
{
  l4_uint32_t s, ns;

  libc_backend_rtc_get_s_and_ns(&s, &ns);

  tp->tv_sec  = s + _rtc_be.offset;
  tp->tv_nsec = ns;

  return 0;
}
