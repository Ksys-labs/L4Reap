#ifndef __KVM_L4_SVM_H
#define __KVM_L4_SVM_H

#include <asm/svm.h>
#include <linux/kvm_host.h>

#include <l4/sys/types.h>
#include <l4/sys/vm.h>
#include <l4/sys/factory.h>
#include <l4/re/consts.h>
#include <l4/log/log.h>
#include <asm/generic/user.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>

#include "kvm-l4.h"

#define L4X_VMCB_LOG2_SIZE 12

static inline void l4x_svm_vmcb_seg_dump(struct kvm_vcpu *vcpu, struct vmcb_seg *seg, char *name)
{
	vcpu_printf(vcpu, "%s: selector = %04x, attrib = %04x limit = %08x, attrib = %016llx\n",
	            name, seg->selector, seg->attrib, seg->limit, seg->base);
}

static inline void l4x_svm_vmcb_dump(struct kvm_vcpu *vcpu)
{
	struct vcpu_svm *svm = to_svm(vcpu);
	struct vmcb_control_area c = svm->vmcb->control;
	struct vmcb_save_area s = svm->vmcb->save;

	vcpu_printf(vcpu, "************************ vmcb_dump ************************\n");
	vcpu_printf(vcpu, "****** save ****\n");
	l4x_svm_vmcb_seg_dump(vcpu, &s.es, "es");
	l4x_svm_vmcb_seg_dump(vcpu, &s.cs, "cs");
	l4x_svm_vmcb_seg_dump(vcpu, &s.ss, "ss");
	l4x_svm_vmcb_seg_dump(vcpu, &s.ds, "ds");
	l4x_svm_vmcb_seg_dump(vcpu, &s.fs, "fs");
	l4x_svm_vmcb_seg_dump(vcpu, &s.gs, "gs");
	l4x_svm_vmcb_seg_dump(vcpu, &s.gdtr, "gdtr");
	l4x_svm_vmcb_seg_dump(vcpu, &s.ldtr, "ldtr");
	l4x_svm_vmcb_seg_dump(vcpu, &s.idtr, "idtr");
	l4x_svm_vmcb_seg_dump(vcpu, &s.idtr, "tr");
	vcpu_printf(vcpu, "cpl = %02x, efer = %016llx\n", s.cpl, s.efer);
	vcpu_printf(vcpu, "cr0 = %016llx cr2 = %016llx\n", s.cr0, s.cr2);
	vcpu_printf(vcpu, "cr3 = %016llx cr4 = %016llx\n", s.cr3, s.cr4);
	vcpu_printf(vcpu, "dr6 = %016llx dr7 = %016llx\n", s.dr6, s.dr7);
	vcpu_printf(vcpu, "rflags = %016llx, rip = %016llx\n", s.rflags, s.rip);
	vcpu_printf(vcpu, "rsp = %016llx rax = %016llx\n", s.rsp, s.rax);
	vcpu_printf(vcpu, "star = %016llx lstar = %016llx\n", s.star, s.lstar);
	vcpu_printf(vcpu, "cstar = %016llx\n", s.cstar);
	vcpu_printf(vcpu, "sfmask = %016llx kernel_gs_base = %016llx\n", s.sfmask, s.kernel_gs_base);
	vcpu_printf(vcpu, "syse_cs = %016llx syse_esp = %016llx\n", s.sysenter_cs, s.sysenter_esp);
	vcpu_printf(vcpu, "syse_eip = %016llx\n", s.sysenter_eip);
	vcpu_printf(vcpu, "g_pat = %016llx dbgctl = %016llx\n", s.g_pat, s.dbgctl);
	vcpu_printf(vcpu, "br_from = %016llx br_to = %016llx\n", s.br_from, s.br_to);
	vcpu_printf(vcpu, "last_ex_from = %016llx last_ex_to = %016llx\n", s.last_excp_from, s.last_excp_to);
	vcpu_printf(vcpu, "**** control ***\n");
	vcpu_printf(vcpu, "inter_cr = %08x\n", c.intercept_cr);
	vcpu_printf(vcpu, "inter_dr = %08x\n", c.intercept_dr);
	vcpu_printf(vcpu, "inter_ex = %08x\n", c.intercept_exceptions);
	vcpu_printf(vcpu, "inter = %016llx\n", c.intercept);
	vcpu_printf(vcpu, "iopm_base_pa = %016llx msrpm_base_pa = %016llx\n", c.iopm_base_pa, c.msrpm_base_pa);
	vcpu_printf(vcpu, "tsc_offset = %016llx\n", c.tsc_offset);
	vcpu_printf(vcpu, "asid = %08x tlb_ctl = %02x\n", c.asid, c.tlb_ctl);
	vcpu_printf(vcpu, "int_ctl = %08x int_v = %08x int_s = %08x\n", c.int_ctl, c.int_vector, c.int_state);
	vcpu_printf(vcpu, "exit = %08x exit_hi = %08x\n", c.exit_code, c.exit_code_hi);
	vcpu_printf(vcpu, "exit_i1 = %016llx exit_i2 = %016llx\n", c.exit_info_1, c.exit_info_2);
	vcpu_printf(vcpu, "exit_int_i = %08x exit_int_i_err = %08x\n", c.exit_int_info, c.exit_int_info_err);
	vcpu_printf(vcpu, "nested_ctl = %016llx\n", c.nested_ctl);
	vcpu_printf(vcpu, "inj = %08x inj_err= %08x\n", c.event_inj, c.event_inj_err);
	vcpu_printf(vcpu, "ncr3 = %016llx\n", c.nested_cr3);
	vcpu_printf(vcpu, "lbr_ctl = %016llx\n", c.lbr_ctl);
	vcpu_printf(vcpu, "*********************** end_of_dump ***********************\n");
}

#endif //__KVM_L4_H
