/**
 * \file
 * \brief Small RTC server test
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/rtc/rtc.h>
#include <l4/util/util.h>
#include <stdio.h>

int main(void)
{
  l4_uint32_t value;

  if (l4rtc_get_offset_to_realtime(&value))
    printf("Error: l4rtc_get_offset_to_realtime\n");
  else
    printf("offset-to-realtime: %d\n", value);

  if (l4rtc_get_linux_tsc_scaler(&value))
    printf("Error: l4rtc_get_linux_tsc_scaler\n");
  else
    printf("linux-tsc-scaler: %d\n", value);

  while (1)
    {
      if (l4rtc_get_seconds_since_1970(&value))
        printf("Error: l4rtc_get_seconds_since_1970\n");
      else
        printf("time: %d\n", value);
      l4_sleep(400);
    }

  return 0;
}
