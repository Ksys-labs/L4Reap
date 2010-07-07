/**
 * \file    rtc/server/src/x86.c
 * \brief   Get current data and time from CMOS
 *
 * \date    09/23/2003
 * \author  Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

/* most stuff taken from Linux 2.4.22 */

#include <stdio.h>

#include <l4/util/rdtsc.h>
#include <l4/util/port_io.h>
#include <l4/re/env>
#include <l4/util/util.h>
#include <l4/io/io.h>

#include "rtc.h"

#define RTC_SECONDS		0
#define RTC_SECONDS_ALARM	1
#define RTC_MINUTES		2
#define RTC_MINUTES_ALARM	3
#define RTC_HOURS		4
#define RTC_HOURS_ALARM		5
#define RTC_DAY_OF_WEEK		6
#define RTC_DAY_OF_MONTH	7
#define RTC_MONTH		8
#define RTC_YEAR		9

#define RTC_REG_A		10
#define RTC_REG_B		11
#define RTC_REG_C		12
#define RTC_REG_D		13

#define RTC_FREQ_SELECT		RTC_REG_A
# define RTC_UIP		0x80
# define RTC_DIV_CTL		0x70
#  define RTC_REF_CLCK_4MHZ	0x00
#  define RTC_REF_CLCK_1MHZ	0x10
#  define RTC_REF_CLCK_32KHZ	0x20
#  define RTC_DIV_RESET1	0x60
#  define RTC_DIV_RESET2	0x70
# define RTC_RATE_SELECT	0x0F

#define RTC_CONTROL		RTC_REG_B
# define RTC_SET		0x80
# define RTC_PIE		0x40
# define RTC_AIE		0x20
# define RTC_UIE		0x10
# define RTC_SQWE		0x08
# define RTC_DM_BINARY		0x04
# define RTC_24H		0x02
# define RTC_DST_EN		0x01

#define RTC_PORT(x)		(0x70 + (x))
#define RTC_ALWAYS_BCD	1	/* RTC operates in binary mode */

#define sleep_as_iodelay_port80() l4_sleep(0)

#define CMOS_READ(addr) ({		\
	l4_uint8_t val;			\
	l4util_out8(addr, RTC_PORT(0)); \
	sleep_as_iodelay_port80();	\
	val = l4util_in8(RTC_PORT(1));	\
	sleep_as_iodelay_port80();	\
	val;				\
	})

#define CMOS_WRITE(val, addr) ({	\
	l4util_out8(addr, RTC_PORT(0)); \
	sleep_as_iodelay_port80();	\
	l4util_out8(val,  RTC_PORT(1));	\
	sleep_as_iodelay_port80();	\
	})

#define BCD_TO_BIN(val)		((val)=((val)&15) + ((val)>>4)*10)
#define BIN_TO_BCD(val)		((val)=(((val)/10)<<4) + (val)%10)

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08) */
static inline l4_uint32_t
mktime (l4_uint32_t year, l4_uint32_t mon, l4_uint32_t day,
	l4_uint32_t hour, l4_uint32_t min, l4_uint32_t sec)
{
  if (0 >= (int) (mon -= 2))
    {
      /* 1..12 -> 11,12,1..10 */
      mon += 12; /* puts Feb last since it has leap day */
      year -= 1;
    }

  return ((( (year/4 - year/100 + year/400 + 367*mon/12 + day)
	    + year*365 - 719499
	   )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
         )*60 + sec; /* finally seconds */
}

/*
 * Get current time from CMOS and initialize values.
 */
static int
get_base_time_x86(void)
{
  l4_uint32_t year, mon, day, hour, min, sec;
  l4_uint32_t seconds_since_1970;
  l4_cpu_time_t current_tsc;
  l4_uint32_t current_s, current_ns;
  long i;

  //l4util_cli();
  //fiasco_watchdog_disable();

  if ((i = l4io_request_ioport(RTC_PORT(0), 2)))
    {
      printf("Could not get required port %x and %x, error: %lx\n",
             RTC_PORT(0), RTC_PORT(0) + 1, i);
      return 1;
    }


  /* The Linux interpretation of the CMOS clock register contents:
   * When the Update-In-Progress (UIP) flag goes from 1 to 0, the
   * RTC registers show the second which has precisely just started.
   * Let's hope other operating systems interpret the RTC the same way. */

  /* read RTC exactly on falling edge of update flag */
  for (i=0 ; i<1000000 ; i++)	/* may take up to 1 second... */
    if (CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP)
      break;

  for (i=0 ; i<1000000 ; i++)	/* must try at least 2.228 ms */
    if (!(CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
      break;

  do
    {
      current_tsc = l4_rdtsc();

      sec  = CMOS_READ(RTC_SECONDS);
      min  = CMOS_READ(RTC_MINUTES);
      hour = CMOS_READ(RTC_HOURS);
      day  = CMOS_READ(RTC_DAY_OF_MONTH);
      mon  = CMOS_READ(RTC_MONTH);
      year = CMOS_READ(RTC_YEAR);

    } while (sec != CMOS_READ(RTC_SECONDS));

  if (!(CMOS_READ(RTC_CONTROL) & RTC_DM_BINARY) || RTC_ALWAYS_BCD)
    {
      BCD_TO_BIN(sec);
      BCD_TO_BIN(min);
      BCD_TO_BIN(hour);
      BCD_TO_BIN(day);
      BCD_TO_BIN(mon);
      BCD_TO_BIN(year);
    }

  //l4util_sti();
  //fiasco_watchdog_enable();

  if ((year += 1900) < 1970)
    year += 100;

  l4_tsc_to_s_and_ns(current_tsc, &current_s, &current_ns);

  seconds_since_1970        = mktime(year, mon, day, hour, min, sec);
  system_time_offs_rel_1970 = seconds_since_1970 - current_s;


  /* Free I/O space at RMGR (cli/sti mapped L4_WHOLE_IOADDRESS_SPACE) */
//  rmgr_free_fpage(l4_iofpage(0, L4_WHOLE_IOADDRESS_SPACE, 0));
  /* Unmap I/O space. RMGR should do it but can't because I/O mappings
   * are not stored in Fiasco's mapping database */
//  l4_fpage_unmap(l4_iofpage(0, L4_WHOLE_IOADDRESS_SPACE, 0),
//			    L4_FP_FLUSH_PAGE | L4_FP_ALL_SPACES);

  printf("Date:%02d.%02d.%04d Time:%02d:%02d:%02d\n",
	  day, mon, year, hour, min, sec);

  return 0;
}


get_base_time_func_t
init_x86(void)
{
  return get_base_time_x86;
}

