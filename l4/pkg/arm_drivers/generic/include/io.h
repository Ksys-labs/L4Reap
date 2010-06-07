/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __ARM_DRIVERS__GENERIC__INCLUDE__IO_H__
#define __ARM_DRIVERS__GENERIC__INCLUDE__IO_H__

#include <sys/cdefs.h>
#include <l4/sys/types.h>

EXTERN_C_BEGIN

L4_INLINE
l4_umword_t io_read_mword(l4_addr_t addr);

L4_INLINE
void io_write_mword(l4_addr_t addr, l4_addr_t val);



L4_INLINE
l4_umword_t io_read_mword(l4_addr_t addr)
{
  return *(volatile l4_umword_t *)addr;
}


L4_INLINE
void io_write_mword(l4_addr_t addr, l4_umword_t val)
{
  *(volatile l4_umword_t *)addr = val;
}

EXTERN_C_END

#endif /* ! __ARM_DRIVERS__LCD__INCLUDE__LCD_H__ */
