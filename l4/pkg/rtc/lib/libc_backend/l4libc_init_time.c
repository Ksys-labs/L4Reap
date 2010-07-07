/**
 * \file   rtc/lib/libc_backends/time/l4libc_init_time.c
 * \brief  init function
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <sys/time.h>
#include <errno.h>

#include <l4/rtc/rtc.h>
#include <l4/crtn/initpriorities.h>
#include <stdio.h>
#include <inttypes.h>

#include "gettime.h"

l4_uint32_t l4rtc_offset;

static __attribute__((used))
void l4libc_init_time(void)
{
    int ret;

    libc_backend_rtc_init();

    ret = l4rtc_get_offset_to_realtime(&l4rtc_offset);

    // error, assume offset 0
    if (ret)
      {
        printf("RTC server not found, assuming 1.1.1970, 0:00 ...\n");
        l4rtc_offset = 0;
      }
}
L4_DECLARE_CONSTRUCTOR(l4libc_init_time, INIT_PRIO_RTC_L4LIBC_INIT);
