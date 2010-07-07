/**
 * \file	rtc/include/rtc.h
 * \brief	RTC library interface
 * 
 * \date	06/15/2001
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */


#ifndef L4_RTC_RTC_H
#define L4_RTC_RTC_H

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

enum {
  L4RTC_OPCODE_get_offset,
  L4RTC_OPCODE_get_linux_tsc_scaler,
};

EXTERN_C_BEGIN

/**
 * Deliver the numbers of seconds elapsed since 01.01.1970. This value is
 * needed by Linux.
 *
 * \return 0 on success, <0 on error
 * */
L4_CV int l4rtc_get_seconds_since_1970(l4_uint32_t *seconds);

/**
 * Deliver the offset between real time and system's uptime in seconds.
 * Some applications want to compute their time in other ways as done
 * in l4rtc_get_seconds_since_1970().
 *
 * \return 0 on success, <0 on error
 * */
L4_CV int l4rtc_get_offset_to_realtime(l4_uint32_t *offset);

/**
 * Deliver the scaler 2^32 / (tsc clocks per usec). This value is needed by
 * Linux.
 *
 * \return 0 on success, <0 on error
 */
L4_CV int l4rtc_get_linux_tsc_scaler(l4_uint32_t *scaler);

EXTERN_C_END

#endif

