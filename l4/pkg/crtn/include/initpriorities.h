/**
 * \file
 * \brief List of all init priorities.
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4RE__INIT_PRIORITIES__
#define __L4RE__INIT_PRIORITIES__

#define INIT_PRIO_EARLY_VAL                 101
#define INIT_PRIO_L4RE_UTIL_CAP_ALLOC       200

#define INIT_PRIO_THREADLIB_UTCB_BITMAP     1001
#define INIT_PRIO_THREADLIB_INIT_VAL        1002
#define INIT_PRIO_LIBC_BE_FILE_VAL          1005

#define INIT_PRIO_LIBIO_INIT_VAL            1100
#define INIT_PRIO_RTC_L4LIBC_INIT_VAL       1200
#define INIT_PRIO_LATE_VAL                  2000

#ifdef __ARM_EABI__

#define INIT_PRIO_EARLY                     00101
#define INIT_PRIO_THREADLIB_INIT            01002
#define INIT_PRIO_LIBC_BE_FILE              01005
#define INIT_PRIO_LIBIO_INIT                01100
#define INIT_PRIO_RTC_L4LIBC_INIT           01200
#define INIT_PRIO_LATE                      02000

#else

#define INIT_PRIO_EARLY                     65434
#define INIT_PRIO_THREADLIB_INIT            64533
#define INIT_PRIO_LIBC_BE_FILE              64530
#define INIT_PRIO_LIBIO_INIT                64435
#define INIT_PRIO_RTC_L4LIBC_INIT           64335
#define INIT_PRIO_LATE                      63535

/* Assertions on the rev priorities */
#if INIT_PRIO_EARLY + INIT_PRIO_EARLY_VAL != 65535
#error INIT_PRIO_EARLY mis-match
#endif
#if INIT_PRIO_THREADLIB_INIT + INIT_PRIO_THREADLIB_INIT_VAL != 65535
#error INIT_PRIO_THREADLIB mis-match
#endif
#if INIT_PRIO_LIBC_BE_FILE + INIT_PRIO_LIBC_BE_FILE_VAL != 65535
#error INIT_PRIO_LIBC_BE_FILE mis-match
#endif
#if INIT_PRIO_LIBIO_INIT + INIT_PRIO_LIBIO_INIT_VAL != 65535
#error INIT_PRIO_THREADLIB mis-match
#endif
#if INIT_PRIO_RTC_L4LIBC_INIT + INIT_PRIO_RTC_L4LIBC_INIT_VAL != 65535
#error INIT_PRIO_RTC_L4LIBC_INIT mis-match
#endif
#if INIT_PRIO_LATE + INIT_PRIO_LATE_VAL != 65535
#error INIT_PRIO_LATE mis-match
#endif
#endif

#endif
