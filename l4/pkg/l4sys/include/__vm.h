/**
 * \internal
 * \file
 * \brief X86 virtualization interface.
 */
/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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

#include <l4/sys/__vm-svm.h>
#include <l4/sys/__vm-vmx.h>

/**
 * \group Return location where to store GP-regs.
 * \ingroup l4_vm_api
 *
 * \return Location of GP-regs.
 * \see l4_vm_run
 *
 * Note that the function returns a location within the UTCB, i.e. between
 * calling this function and l4_vm_run() the UTCB must not be used.
 */
L4_INLINE
l4_vm_gpregs_t *
l4_vm_gpregs(void) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_vm_api
 */
L4_INLINE
l4_vm_gpregs_t *
l4_vm_gpregs_u(l4_utcb_t *u) L4_NOTHROW;

/**
 * \brief Run a VM
 * \ingroup l4_vm_api
 *
 * \param vm         Capability selector for VM
 * \param vmcb_fpage VMCB
 * \param gpregs     General purpose registers
 *
 * The general purpose registers are stored in the UTCB before calling this
 * function with the function l4_vm_gpregs().
 */
L4_INLINE l4_msgtag_t
l4_vm_run(l4_cap_idx_t vm, l4_fpage_t vmcx_fpage) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_vm_api
 */
L4_INLINE l4_msgtag_t
l4_vm_run_u(l4_cap_idx_t vm_task, l4_fpage_t const vmcb_fpage,
            l4_utcb_t *u) L4_NOTHROW;


/**
 * \internal
 * \brief Operations on task objects.
 * \ingroup l4_vm_api
 */
enum
{
  L4_VM_RUN_OP    = L4_TASK_VM_OPS + 0    /* Run a VM */
};


/****** Implementations ****************/

L4_INLINE
l4_vm_gpregs_t *
l4_vm_gpregs_u(l4_utcb_t *u) L4_NOTHROW
{
  union cast
  {
    l4_vm_gpregs_t r;
    l4_umword_t a[sizeof(l4_vm_gpregs_t) / sizeof(l4_umword_t)];
  };
  return &((union cast *)&l4_utcb_mr_u(u)->mr[1])->r;
}

L4_INLINE l4_msgtag_t
l4_vm_run_u(l4_cap_idx_t vm_task, l4_fpage_t const vmcx_fpage,
            l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t tag;
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  enum { GPREGS_WORDS = sizeof(l4_vm_gpregs_t) / sizeof(l4_umword_t) };
  v->mr[0]  = L4_VM_RUN_OP;

  v->mr[1 + GPREGS_WORDS] = l4_map_control(0, 0, 0);
  v->mr[2 + GPREGS_WORDS] = vmcx_fpage.raw;

  tag = l4_ipc_call(vm_task, u,
                    l4_msgtag(L4_PROTO_TASK, 1 + GPREGS_WORDS, 1, 0),
                    L4_IPC_NEVER);

  return tag;
}

L4_INLINE
l4_vm_gpregs_t *
l4_vm_gpregs(void) L4_NOTHROW
{
  return l4_vm_gpregs_u(l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_vm_run(l4_cap_idx_t task, l4_fpage_t vmcx_fpage) L4_NOTHROW
{
  return l4_vm_run_u(task, vmcx_fpage, l4_utcb());
}
