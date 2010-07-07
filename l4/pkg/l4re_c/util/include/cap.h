/**
 * \file
 * \brief   Capability allocator C interface
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

/**
 * \defgroup api_l4re_c_util_cap Capability allocator
 * \ingroup api_l4re_c
 * \brief Capability allocator C interface.
 */

#include <l4/sys/types.h>
#include <l4/sys/linkage.h>
#include <l4/sys/task.h>

EXTERN_C_BEGIN

/**
 * \brief Release a capability from a task (unmap).
 * \ingroup api_l4re_c_util
 *
 * Note that the given capability does not need to be handled by the
 * capability allocator in any way.
 */
L4_CV L4_INLINE
l4_msgtag_t
l4re_util_cap_release(l4_cap_idx_t cap);

/* Implementations */

L4_CV L4_INLINE
l4_msgtag_t
l4re_util_cap_release(l4_cap_idx_t cap)
{
  return l4_task_unmap(L4_BASE_TASK_CAP,
                       l4_obj_fpage(cap, 0, L4_FPAGE_RWX),
                       L4_FP_ALL_SPACES);
}

EXTERN_C_END
