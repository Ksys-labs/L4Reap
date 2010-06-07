/*
 * (c) 2009 Technische Universität Dresden
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

#include <l4/sys/types.h>
#include <l4/sys/__vcpu-arch.h>

/**
 * \defgroup l4_vcpu_api vCPU API
 * \ingroup  l4_thread_api
 * \brief vCPU API
 */

/**
 * \brief State of a vCPU
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_state_t
{
  l4_vcpu_regs_t       r;
  l4_vcpu_ipc_regs_t   i;

  l4_umword_t          state;
  l4_umword_t          saved_state;

  l4_umword_t          sticky_flags;

  l4_cap_idx_t         user_task;

  l4_umword_t          entry_sp;
  l4_umword_t          entry_ip;
  l4_umword_t          reserved_sp;
} l4_vcpu_state_t;

/**
 * \brief State flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_state_flags
{
  L4_VCPU_F_IRQ         = 0x01,
  L4_VCPU_F_PAGE_FAULTS = 0x02,
  L4_VCPU_F_EXCEPTIONS  = 0x04,
  L4_VCPU_F_DEBUG_EXC   = 0x08,
  L4_VCPU_F_USER_MODE   = 0x20,
  L4_VCPU_F_FPU_ENABLED = 0x80,
};

/**
 * \brief Sticky flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_sticky_flags
{
  L4_VCPU_SF_IRQ_PENDING = 0x01,
};
