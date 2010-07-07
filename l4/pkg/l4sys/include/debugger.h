#pragma once
/**
 * \file
 * \brief Debugger related definitions.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/sys/compiler.h>
#include <l4/sys/utcb.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_debugger_api Kernel Debugger
 * \ingroup api_calls_fiasco
 * \brief Kernel debugger related functionality.
 * \attention This API is subject to change!
 *
 * <c>\#include <l4/sys/debugger.h></c>
 */

/**
 * The string name of kernel object.
 * \ingroup l4_debugger_api
 *
 * \param cap     Capability
 * \param name    Name
 */
L4_INLINE l4_msgtag_t
l4_debugger_set_object_name(l4_cap_idx_t cap, const char *name) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_debugger_set_object_name_u(l4_cap_idx_t cap, const char *name, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get the globally unique ID of the object behind a capability.
 * \ingroup l4_debugger_api
 *
 * \param cap    Capability
 *
 * \return ~0UL on non-valid capability, ID otherwise
 *
 * This is a debugging factility, do not use it in real code.
 */
L4_INLINE unsigned long
l4_debugger_global_id(l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE unsigned long
l4_debugger_global_id_u(l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW;

enum
{
  L4_DEBUGGER_NAME_SET_OP  = 0UL,
  L4_DEBUGGER_GLOBAL_ID_OP = 1UL,
};


/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/kernel_object.h>

L4_INLINE l4_msgtag_t
l4_debugger_set_object_name_u(unsigned long cap,
                              const char *name, l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned int i;
  char *s = (char *)&l4_utcb_mr_u(utcb)->mr[1];
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_NAME_SET_OP;
  for (i = 0;
       *name && i < (L4_UTCB_GENERIC_DATA_SIZE - 2) * sizeof(l4_umword_t) - 1;
       ++i, ++name, ++s)
    *s = *name;
  *s = 0;
  i = (i + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t);
  return l4_invoke_debugger(cap, l4_msgtag(0, i, 0, 0), utcb);
}

L4_INLINE unsigned long
l4_debugger_global_id_u(l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_GLOBAL_ID_OP;
  if (l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 1, 0, 0), utcb), utcb))
    return ~0UL;
  return l4_utcb_mr_u(utcb)->mr[0];
}





L4_INLINE l4_msgtag_t
l4_debugger_set_object_name(unsigned long cap,
                            const char *name) L4_NOTHROW
{
  return l4_debugger_set_object_name_u(cap, name, l4_utcb());
}

L4_INLINE unsigned long
l4_debugger_global_id(l4_cap_idx_t cap) L4_NOTHROW
{
  return l4_debugger_global_id_u(cap, l4_utcb());
}
