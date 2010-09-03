/**
 * \internal
 * \file
 * \brief X86 virtualization interface.
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
#ifndef __INCLUDE__ARCH_X86__VM_H__
#define __INCLUDE__ARCH_X86__VM_H__

#include <l4/sys/types.h>

/**
 * \brief General purpose regisers, x86-64
 * \ingroup l4_vm_api
 */
typedef struct l4_vm_gpregs_t
{
  l4_umword_t rax;
  l4_umword_t rsi;
  l4_umword_t rdx;
  l4_umword_t rcx;
  l4_umword_t rdi;
  l4_umword_t r8;
  l4_umword_t r9;
  l4_umword_t rbx;
  l4_umword_t rbp;
  l4_umword_t r10;
  l4_umword_t r11;
  l4_umword_t r12;
  l4_umword_t r13;
  l4_umword_t r14;
  l4_umword_t r15;
  l4_umword_t dr0;
  l4_umword_t dr1;
  l4_umword_t dr2;
  l4_umword_t dr3;
} l4_vm_gpregs_t;

#include <l4/sys/__vm.h>

#endif /* ! __INCLUDE__ARCH_X86__VM_H__ */
