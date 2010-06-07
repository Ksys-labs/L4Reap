/**
 * \file
 * \brief  Functions to acquire thread time fast (without kernel entry)
 *
 * \date   Martin Pohlack  <mp26@os.inf.tu-dresden.de> */

/*
 * (c) 2005-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */


#ifndef __L4UTIL__INCLUDE__ARCH_X86__THREAD_TIME_H__
#define __L4UTIL__INCLUDE__ARCH_X86__THREAD_TIME_H__

#include <l4/sys/kip.h>
#include <l4/util/rdtsc.h>

EXTERN_C_BEGIN

/**
 * \brief Acquire accumulated runtime  from the kernel info page for currently
 *        running thread
 *
 * @param kinfo pointer to mapped kernel info page
 *
 * \return Accumulated thread time (cycles)
 */
L4_INLINE l4_cpu_time_t l4util_thread_time(const l4_kernel_info_t * kinfo);
L4_INLINE l4_cpu_time_t l4util_thread_time(const l4_kernel_info_t * kinfo)
{
    l4_cpu_time_t switch_time, thread_time, now;

    do
      {
        now          = l4_rdtsc();
        /* fixme: make 64 bit reads atomic ??? */
        thread_time  = kinfo->thread_time;
        switch_time  = kinfo->switch_time;
      }
    while (now < switch_time);

    return now - switch_time + thread_time;
}

EXTERN_C_END


#endif /* ! __L4UTIL__INCLUDE__ARCH_X86__THREAD_TIME_H__ */
