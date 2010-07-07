/**
 * \internal
 * \file
 * \brief X86 virtualization interface.
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

#include <l4/sys/ipc.h>
#include <l4/sys/task.h>

/**
 * \defgroup l4_vm_svm_api VM API for SVM
 * \brief Virtual machine API for SVM.
 * \ingroup l4_vm_api
 */


/**
 * \brief VMCB structure for SVM VMs
 * \ingroup l4_vm_svm_api
 */
struct l4_vm_svm_vmcb_control_area
{
  l4_uint16_t intercept_rd_crX;
  l4_uint16_t intercept_wr_crX;

  l4_uint16_t intercept_rd_drX;
  l4_uint16_t intercept_wr_drX;

  l4_uint32_t intercept_exceptions;

  l4_uint32_t intercept_instruction0;
  l4_uint32_t intercept_instruction1;

  l4_uint8_t _reserved0[44];

  l4_uint64_t iopm_base_pa;
  l4_uint64_t msrpm_base_pa;
  l4_uint64_t tsc_offset;
  l4_uint64_t guest_asid_tlb_ctl;
  l4_uint64_t interrupt_ctl;
  l4_uint64_t interrupt_shadow;
  l4_uint64_t exitcode;
  l4_uint64_t exitinfo1;
  l4_uint64_t exitinfo2;
  l4_uint64_t exitintinfo;
  l4_uint64_t np_enable;

  l4_uint8_t _reserved1[16];

  l4_uint64_t eventinj;
  l4_uint64_t n_cr3;
  l4_uint64_t lbr_virtualization_enable;

  l4_uint8_t _reserved2[832];
} __attribute__((packed));

/**
 * \brief State save area segment selector struct
 * \ingroup l4_vm_svm_api
 */
struct l4_vm_svm_vmcb_state_save_area_seg
{
  l4_uint16_t selector;
  l4_uint16_t attrib;
  l4_uint32_t limit;
  l4_uint64_t base;
} __attribute__((packed));

/**
 * \brief State save area structure for SVM VMs
 * \ingroup l4_vm_svm_api
 */
struct l4_vm_svm_vmcb_state_save_area
{
  struct l4_vm_svm_vmcb_state_save_area_seg es;
  struct l4_vm_svm_vmcb_state_save_area_seg cs;
  struct l4_vm_svm_vmcb_state_save_area_seg ss;
  struct l4_vm_svm_vmcb_state_save_area_seg ds;
  struct l4_vm_svm_vmcb_state_save_area_seg fs;
  struct l4_vm_svm_vmcb_state_save_area_seg gs;
  struct l4_vm_svm_vmcb_state_save_area_seg gdtr;
  struct l4_vm_svm_vmcb_state_save_area_seg ldtr;
  struct l4_vm_svm_vmcb_state_save_area_seg idtr;
  struct l4_vm_svm_vmcb_state_save_area_seg tr;

  l4_uint8_t _reserved0[43];

  l4_uint8_t cpl;

  l4_uint32_t _reserved1;

  l4_uint64_t efer;

  l4_uint8_t _reserved2[112];

  l4_uint64_t cr4;
  l4_uint64_t cr3;
  l4_uint64_t cr0;
  l4_uint64_t dr7;
  l4_uint64_t dr6;
  l4_uint64_t rflags;
  l4_uint64_t rip;

  l4_uint8_t _reserved3[88];

  l4_uint64_t rsp;

  l4_uint8_t _reserved4[24];

  l4_uint64_t rax;
  l4_uint64_t star;
  l4_uint64_t lstar;
  l4_uint64_t cstar;
  l4_uint64_t sfmask;
  l4_uint64_t kernelgsbase;
  l4_uint64_t sysenter_cs;
  l4_uint64_t sysenter_esp;
  l4_uint64_t sysenter_eip;
  l4_uint64_t cr2;

  l4_uint8_t _reserved5[32];

  l4_uint64_t g_pat;
  l4_uint64_t dbgctl;
  l4_uint64_t br_from;
  l4_uint64_t br_to;
  l4_uint64_t lastexcpfrom;
  l4_uint64_t last_excpto;

  l4_uint8_t _reserved6[2408];
} __attribute__((packed));


/**
 * \brief Control structure for SVM VMs
 * \ingroup l4_vm_svm_api
 */
struct l4_vm_svm_vmcb
{
  struct l4_vm_svm_vmcb_control_area    control_area;
  struct l4_vm_svm_vmcb_state_save_area state_save_area;
};

/**
 * \brief Run a VM
 * \ingroup l4_vm_svm_api
 *
 * \param vm         Capability selector for VM
 * \param vmcb_fpage VMCB
 * \param gpregs     General purpose registers
 *
 * \note SVM only for now
 */
L4_INLINE l4_msgtag_t
l4_vm_run_svm(l4_cap_idx_t vm, l4_fpage_t vmcb_fpage,
              struct l4_vm_svm_gpregs *gpregs) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_vm_svm_api
 */
L4_INLINE l4_msgtag_t
l4_vm_run_svm_u(l4_cap_idx_t vm_task, l4_fpage_t const vmcb_fpage,
                struct l4_vm_svm_gpregs *gpregs, l4_utcb_t *u) L4_NOTHROW;


/**
 * \internal
 * \brief Operations on task objects.
 * \ingroup l4_vm_svm_api
 */
enum
{
  L4_VM_RUN_OP    = L4_TASK_VM_OPS + 0    /* Run a VM */
};


/****** Implementations ****************/

L4_INLINE l4_msgtag_t
l4_vm_run_svm_u(l4_cap_idx_t vm_task, l4_fpage_t const vmcb_fpage,
                struct l4_vm_svm_gpregs *gpregs, l4_utcb_t *u) L4_NOTHROW
{
  l4_msgtag_t tag;
  l4_msg_regs_t *v = l4_utcb_mr_u(u);
  enum { GPREGS_WORDS = sizeof(*gpregs) / sizeof(l4_umword_t), };
  v->mr[0]  = L4_VM_RUN_OP;

  __builtin_memcpy(&v->mr[1], gpregs, sizeof(*gpregs));
  v->mr[1 + GPREGS_WORDS] = l4_map_control(0, 0, 0);
  v->mr[2 + GPREGS_WORDS] = vmcb_fpage.raw;

  tag = l4_ipc_call(vm_task, u,
                    l4_msgtag(L4_PROTO_TASK, 1 + GPREGS_WORDS, 1, 0),
                    L4_IPC_NEVER);

  __builtin_memcpy(gpregs, &v->mr[1], sizeof(*gpregs));

  return tag;
}

L4_INLINE l4_msgtag_t
l4_vm_run_svm(l4_cap_idx_t task, l4_fpage_t vmcb_fpage,
              struct l4_vm_svm_gpregs *gpregs) L4_NOTHROW
{
  return l4_vm_run_svm_u(task, vmcb_fpage, gpregs, l4_utcb());
}
