/**
 * \file   rtc/lib/client/librtc.cc
 * \brief  client stub
 *
 * \date   09/23/2003
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#if defined(ARCH_x86) || defined(ARCH_amd64)
#define RTC_AVAIL
#endif


#include <l4/sys/err.h>
#include <l4/rtc/rtc.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>

#include <cstdio>

#ifdef RTC_AVAIL
#include <l4/util/rdtsc.h>
#endif

//#include "rtc-client.h"

static l4_uint32_t s_offs_to_systime;
static l4_uint32_t linux_scaler;

/* We need to define this scaler here for use with l4_tsc_to_ns */
// l4_uint32_t l4_scaler_tsc_to_ns;

#ifdef RTC_AVAIL

static int server_tried;
static L4::Cap<void> server(L4_INVALID_CAP);

/**
 * A fast and cheap way to calculate without violate the 32-bit range */
static inline l4_uint32_t
muldiv (l4_uint32_t val, l4_uint32_t mul, l4_uint32_t div)
{
  l4_uint32_t dummy;

  asm volatile ("mull %3 ; divl %4\n\t"
               :"=a" (val), "=d" (dummy)
               : "0" (val),  "d" (mul),  "c" (div));
  return val;
}

int
l4rtc_if_get_offset_call(L4::Cap<void> server, l4_uint32_t *offset)
{
  L4::Ipc::Iostream _(l4_utcb());
  _ << l4_umword_t(L4RTC_OPCODE_get_offset);
  l4_msgtag_t res = _.call(server.cap());
  if (l4_ipc_error(res, l4_utcb()))
    return 1;
  _ >> *offset;
  return 0; // ok
}

int
l4rtc_if_get_linux_tsc_scaler_call(L4::Cap<void> server, l4_uint32_t *scaler)
{
  L4::Ipc::Iostream _(l4_utcb());
  _ << l4_umword_t(L4RTC_OPCODE_get_linux_tsc_scaler);
  l4_msgtag_t res = _.call(server.cap());
  if (l4_ipc_error(res, l4_utcb()))
    return 1;
  _ >> *scaler;
  return 0; // ok
}

/**
 * Connect to server and retreive general values */
static int
init_done(void)
{
  if (!server_tried)
    {
      server = L4Re::Env::env()->get_cap<void>("rtc");
      if (!server.is_valid())
        {
          printf("rtc not found\n");
          return -L4_EINVAL;
        }

      server_tried = 1;

      if (l4rtc_if_get_offset_call(server, &s_offs_to_systime))
	return -L4_EINVAL;

      if (l4rtc_if_get_linux_tsc_scaler_call(server, &linux_scaler))
	return -L4_EINVAL;

      l4_scaler_tsc_to_ns = muldiv(linux_scaler, 1000, 1<<5);
      l4_scaler_tsc_to_us =        linux_scaler;
      l4_scaler_ns_to_tsc = muldiv(1U<<27, 1U<<29, 125*linux_scaler);
    }

  return 0;
}

static inline void gettime(l4_uint32_t *s, l4_uint32_t *ns)
{
  l4_tsc_to_s_and_ns(l4_rdtsc(), s, ns);
}

#else
static int
init_done(void)
{
  return 0;
}

static inline void gettime(l4_uint32_t *s, l4_uint32_t *ns)
{
  *s = *ns = 0;
}
#endif

/**
 * Deliver the numbers of seconds elapsed since 01.01.1970. This value is
 * needed by Linux. */
int
l4rtc_get_seconds_since_1970(l4_uint32_t *seconds)
{
  l4_uint32_t s, ns;

  if (init_done())
    return -L4_EINVAL;

  gettime(&s, &ns);
  *seconds = s + s_offs_to_systime;
  return 0;
}

/**
 * Deliver the offset between real time and system's uptime in seconds.
 * Some applications want to compute their time in other ways as done
 * in l4rtc_get_seconds_since_1970(). */
int
l4rtc_get_offset_to_realtime(l4_uint32_t *offset)
{
  if (init_done())
    return -L4_EINVAL;

  *offset = s_offs_to_systime;
  return 0;
}

/**
 * Deliver the scaler 2^32 / (tsc clocks per usec). This value is needed by
 * Linux. */
int
l4rtc_get_linux_tsc_scaler(l4_uint32_t *scaler)
{
  if (init_done())
    return -L4_EINVAL;

  *scaler = linux_scaler;
  return 0;
}

