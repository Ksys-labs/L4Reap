/**
 * \file
 * \brief Event C interface.
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
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
 * \defgroup api_l4re_c_event Event interface
 * \ingroup api_l4re_c
 * \brief Event C interface.
 */

#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>

EXTERN_C_BEGIN

/**
 * \brief Event structure used in buffer.
 */
typedef struct
{
  long long time;         /**< Time stamp of the event */
  unsigned short type;    /**< Type of the event */
  unsigned short code;    /**< Code of the event */
  int value;              /**< Value of the event */
  l4_umword_t stream_id;  /**< Stream ID */
} l4re_event_t;

/**
 * \brief Get an event object.
 * \ingroup api_l4re_c_event
 *
 * \param server   Server to talk to.
 * \param ds       Buffer to event data.
 * \param irq      Event signal.
 *
 * \return 0 for success, <0 on error
 *
 * \see L4Re::Event::get
 */
L4_CV long
l4re_event_get(const l4_cap_idx_t server,
               const l4re_ds_t ds) L4_NOTHROW;

EXTERN_C_END
