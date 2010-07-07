/**
 * \file
 * \brief   Shared header file
 *
 * \date
 * \author  Adam Lackorzynski <adam@os.inf.tu-dresden.de> */

/*
 * (c) 2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef __RTC_H__
#define __RTC_H__

#include <sys/cdefs.h>
#include <l4/sys/l4int.h>


extern l4_uint32_t system_time_offs_rel_1970;
typedef int (*get_base_time_func_t)(void);

__BEGIN_DECLS


get_base_time_func_t init_ux(void);
get_base_time_func_t init_x86(void);

__END_DECLS

#endif /* __RTC_H__ */
