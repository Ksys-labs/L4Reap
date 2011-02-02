/**
 * \file
 * \brief  User-lock implementation for SPARC
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
#pragma once

#include <l4/sys/utcb.h>
// dumb, however atomic sequences are defined in kdebug.h
#include <l4/sys/kdebug.h>

#include <l4/sys/compiler.h>
#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_usem_down_to(l4_cap_idx_t lock, l4_u_semaphore_t *sem, l4_timeout_s timeout) L4_NOTHROW
{
	__asm__ __volatile__("ta 0x815\n");
	return l4_msgtag(1,1,1,1);
}

L4_INLINE l4_msgtag_t
l4_usem_up(l4_cap_idx_t lock, l4_u_semaphore_t *sem) L4_NOTHROW
{
	__asm__ __volatile__ ("ta 0x816\n");
	return l4_msgtag(1,1,1,1);
}

