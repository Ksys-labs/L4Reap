/**
 * \internal
 * \file
 * \brief X86 virtualization interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Henning Schild <hschild@os.inf.tu-dresden.de>
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
 * \brief General purpose regisers for SVM, x86-32
 * \ingroup l4_vm_svm_api
 */
struct l4_vm_svm_gpregs
{
  l4_umword_t edx;
  l4_umword_t ecx;
  l4_umword_t ebx;
  l4_umword_t ebp;
  l4_umword_t esi;
  l4_umword_t edi;
  l4_umword_t dr0;
  l4_umword_t dr1;
  l4_umword_t dr2;
  l4_umword_t dr3;
};

#include <l4/sys/__vm-svm.h>

#endif /* ! __INCLUDE__ARCH_X86__VM_H__ */
