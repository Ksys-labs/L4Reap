/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Henning Schild <hschild@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/factory.h>
#include <l4/sys/task.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/vm.h>
#include <l4/sys/thread.h>
#include <l4/sys/vcpu.h>

#include <l4/sys/irq.h>

#include <l4/util/util.h>
#include <l4/util/cpu.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/vcpu/vcpu.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#define STACKSIZE (8<<10)

static char stack[STACKSIZE];
static char hdl_stack[STACKSIZE];
static unsigned long idt[32 * 2] __attribute__((aligned(4096)));
static unsigned long gdt[32 * 2] __attribute__((aligned(4096)));

void vm_resume(void);
void handle_vmexit(void);
l4_vcpu_state_t *vcpu;
l4_vm_svm_vmcb_t *vmcb_s;

static void init_vmcb(l4_vm_svm_vmcb_t *vmcb_s) {

    vmcb_s->control_area.np_enable = 1;
    vmcb_s->control_area.guest_asid_tlb_ctl = 1;

    vmcb_s->state_save_area.es.selector = 0;
    vmcb_s->state_save_area.es.attrib = 0;
    vmcb_s->state_save_area.es.limit = 0;
    vmcb_s->state_save_area.es.base = 0ULL;

    //  vmcb[256 +  0] = 0;        // es; attrib sel

    vmcb_s->state_save_area.cs.selector = 0x8;
    vmcb_s->state_save_area.cs.attrib = 0xc9b;
    vmcb_s->state_save_area.cs.limit = 0xffffffff;
    vmcb_s->state_save_area.cs.base = 0ULL;

    vmcb_s->state_save_area.ss.selector = 0x10;
    vmcb_s->state_save_area.ss.attrib = 0xc93;
    vmcb_s->state_save_area.ss.limit = 0xffffffff;
    vmcb_s->state_save_area.ss.base = 0ULL;

    vmcb_s->state_save_area.ds.selector = 0x23;
    vmcb_s->state_save_area.ds.attrib = 0xcf3;
    vmcb_s->state_save_area.ds.limit = 0xffffffff;
    vmcb_s->state_save_area.ds.base = 0ULL;

    vmcb_s->state_save_area.fs.selector = 0;
    vmcb_s->state_save_area.fs.attrib = 0xcf3;
    vmcb_s->state_save_area.fs.limit = 0xffffffff;
    vmcb_s->state_save_area.fs.base = 0ULL;

    vmcb_s->state_save_area.gs.selector = 0;
    vmcb_s->state_save_area.gs.attrib = 0xcf3;
    vmcb_s->state_save_area.gs.limit = 0xffffffff;
    vmcb_s->state_save_area.gs.base = 0ULL;

    vmcb_s->state_save_area.gdtr.selector = 0;
    vmcb_s->state_save_area.gdtr.attrib = 0;
    vmcb_s->state_save_area.gdtr.limit = 0x3f;
    vmcb_s->state_save_area.gdtr.base = (unsigned long) gdt;

    vmcb_s->state_save_area.ldtr.selector = 0;
    vmcb_s->state_save_area.ldtr.attrib = 0;
    vmcb_s->state_save_area.ldtr.limit = 0;
    vmcb_s->state_save_area.ldtr.base = 0;

    vmcb_s->state_save_area.idtr.selector = 0;
    vmcb_s->state_save_area.idtr.attrib = 0;
    vmcb_s->state_save_area.idtr.limit = 0xff;
    vmcb_s->state_save_area.idtr.base = (unsigned long) idt;

    vmcb_s->state_save_area.tr.selector = 0x28;
    vmcb_s->state_save_area.tr.attrib = 0x8b;
    vmcb_s->state_save_area.tr.limit = 0x67;
    vmcb_s->state_save_area.tr.base = 0;

    vmcb_s->state_save_area.g_pat = 0x7040600010406ULL;
}

static int check_svm(void) {
    l4_umword_t ax, bx, cx, dx;

    if (!l4util_cpu_has_cpuid())
        return 1;

    l4util_cpu_cpuid(0x80000001, &ax, &bx, &cx, &dx);

    if (!(cx & 4)) {
        printf("CPU does not support SVM.\n");
        return 1;
    }

    l4util_cpu_cpuid(0x8000000a, &ax, &bx, &cx, &dx);

    printf("SVM revision: %lx\n", ax & 0xf);
    printf("Number of ASIDs: %lx\n", bx);

    return 0;
}

static int check_svm_npt(void) {
    l4_umword_t ax, bx, cx, dx;

    l4util_cpu_cpuid(0x8000000a, &ax, &bx, &cx, &dx);

    printf("NPT available: %s\n", dx & 1 ? "yes" : "no");

    return (!(dx & 1));
}

int vmexit = 0;

static int cnt_triggered, cnt_received;

static void handler(void) {
    //if(vmexit){
        //vmexit = 0;
        //handle_vmexit();
    //} else {
        // exception and ipc handling here
    cnt_received++;
    printf("received interrupt %d | %d %d\n", (int)vcpu->i.label,
           cnt_triggered, cnt_received);
    //}
    vm_resume();
}

void vm_resume(void) {
    unsigned long long old_rip;
    l4_msgtag_t tag;

//    if ((vmcb_s->state_save_area.rip >= marker)) {
//        //printf("set tf for rip=%llx\n", vmcb_s->state_save_area.rip);
//        vmcb_s->state_save_area.rflags |= 0x100; // set tf
//    }

    vmcb_s->control_area.intercept_exceptions |= 0xa; // intercept #1 & #3
    vmcb_s->control_area.intercept_exceptions |= 0xffffffff;
    vmcb_s->control_area.intercept_instruction1 |= 0x20; // intercept int1

    //vmcb_s->control_area.exitcode = 0x100 << i;
    vmcb_s->control_area.exitcode = 0;

    old_rip = vmcb_s->state_save_area.rip;

    tag = l4_thread_vcpu_resume_commit(L4_INVALID_CAP,
            l4_thread_vcpu_resume_start());
    if (l4_error(tag))
        printf("vm-resume failed: %s (%ld)\n", l4sys_errtostr(l4_error(tag)),
                l4_error(tag));

    // if l4_thread_vcpu_resume_commit returns without error
    // tell the handler that a vmexit occurred
    vmexit = 1;
    // and rewind the stack before we return into the handler
    // simulating an upcall
    handle_vmexit();
    vm_resume();
#if 0
    asm volatile (
            "movl %0, %%esp\n"
            "pushl %1\n"
            "ret\n"
            : : "r"(vcpu->entry_sp), "r"(handler) );
#endif
}

void handle_vmexit(void) {
    static unsigned int i = 0;
    ++i;
    printf("iteration=%d, exit code=%llx", i,
            vmcb_s->control_area.exitcode);

    if (vmcb_s->control_area.exitcode == 0x43)
        // int3 is treated as fault, not trap
        vmcb_s->state_save_area.rip += 1;

    if (vmcb_s->control_area.exitcode == 0x46) {
//        if (vmcb_s->state_save_area.rip >= test_end)
//            break;
        vmcb_s->state_save_area.rip += 2;
    }

    if (vmcb_s->control_area.exitcode == 0x400) {
        printf("host-level page fault; error code=%llx, gpa=%llx\n",
                vmcb_s->control_area.exitinfo1, vmcb_s->control_area.exitinfo2);
    }

    if (vmcb_s->control_area.exitcode == 0x4e) {
        printf("page fault; error code=%llx, pfa=%llx\n",
                vmcb_s->control_area.exitinfo1, vmcb_s->control_area.exitinfo2);
    }

    if (vmcb_s->control_area.exitcode == 0x81) {
        printf("VMMCALL\n");
        vmcb_s->state_save_area.rip += 3;
    }

    if (vmcb_s->control_area.exitcode == 0x4d) {
        printf("cs=%08x attrib=%x, limit=%x, base=%llx\n",
                vmcb_s->state_save_area.cs.selector,
                vmcb_s->state_save_area.cs.attrib,
                vmcb_s->state_save_area.cs.limit,
                vmcb_s->state_save_area.cs.base);
        printf("ss=%08x attrib=%x, limit=%x, base=%llx\n",
                vmcb_s->state_save_area.ss.selector,
                vmcb_s->state_save_area.ss.attrib,
                vmcb_s->state_save_area.ss.limit,
                vmcb_s->state_save_area.ss.base);
        printf("np_enabled=%lld\n", vmcb_s->control_area.np_enable);
        printf("cr0=%llx cr4=%llx\n", vmcb_s->state_save_area.cr0,
                vmcb_s->state_save_area.cr4);
        printf("interrupt_ctl=%llx\n", vmcb_s->control_area.interrupt_ctl);
        printf("rip=%llx, rsp=%llx, cpl=%d\n", vmcb_s->state_save_area.rip,
                vmcb_s->state_save_area.rsp, vmcb_s->state_save_area.cpl);
        printf("exitinfo1=%llx\n", vmcb_s->control_area.exitinfo1);
    }
}

static l4_vcpu_state_t *get_state_mem(l4_addr_t *extstate) {
    static int done;
    long r;
    l4_msgtag_t tag;
    static l4_addr_t ext_state;

    if (done) {
        *extstate = ext_state;
        return vcpu;
    }

    if ((r = l4vcpu_ext_alloc(&vcpu, &ext_state, L4_BASE_TASK_CAP,
            l4re_env()->rm))) {
        printf("Adding state mem failed: %ld\n", r);
        exit(1);
    }

    vcpu->state = L4_VCPU_F_FPU_ENABLED;
    vcpu->saved_state = L4_VCPU_F_USER_MODE | L4_VCPU_F_FPU_ENABLED | L4_VCPU_F_IRQ;

    vcpu->entry_ip = (l4_umword_t)handler;
    l4_umword_t * stack = (l4_umword_t*)(hdl_stack + STACKSIZE);
    --stack;
    *stack = 0;
    vcpu->entry_sp = (l4_umword_t)stack;


    tag = l4_thread_vcpu_control_ext(L4_INVALID_CAP, (l4_addr_t) vcpu);
    if (l4_error(tag)) {
        printf("Could not enable ext vCPU\n");
        exit(1);
    }

    done = 1;
    *extstate = ext_state;

    return vcpu;
}


static void run_test(int np_available) {
    l4_umword_t eflags;
    l4_msgtag_t tag;
    l4_cap_idx_t vm_task = l4re_util_cap_alloc();
//    int i;
    l4_umword_t ip; //, marker, test_end;
    l4_addr_t vmcx;
    get_state_mem(&vmcx);

    printf("run test, np_available=%d\n", np_available);

    if (l4_is_invalid_cap(vm_task)) {
        printf("No more caps.\n");
        return;
    }

    tag = l4_factory_create_vm(l4re_env()->factory, vm_task);
    if (l4_error(tag)) {
        printf("Failed to create new task\n");
        exit(1);
    }

    vmcb_s = (l4_vm_svm_vmcb_t *) vmcx;

    vcpu->user_task = vm_task;

    vcpu->r.dx = 1;
    vcpu->r.cx = 2;
    vcpu->r.bx = 3;
    vcpu->r.bp = 4;
    vcpu->r.si = 5;
    vcpu->r.di = 6;

    printf("clearing exit codes\n");

    //  asm volatile("    jmp 1f;           \n"
    //               "2:  nop               \n"
    //               "    nop               \n"
    //               "    ud2               \n"
    //               "    int3              \n"
    //               "3:  nop               \n"
    //               "    nop               \n"
    //               "    int3              \n"
    //               //"3:  nop               \n"
    //               //"    int3              \n"
    //               "    nop               \n"
    //               "    nop               \n"
    //               "    movl %%eax, %%ecx \n"
    //               "    addl %%edx, %%ecx \n"
    //               "    vmmcall           \n"
    //               "4:                    \n"
    //               "    ud2               \n"
    //               "1:  movl $2b, %0      \n"
    //               "    movl $3b, %1      \n"
    //               "    movl $4b, %2      \n"
    //               : "=r" (ip), "=r" (marker), "=r" (test_end));

    asm volatile("    jmp 1f;           \n"
            "2:  nop               \n"
            "    nop               \n"
            "    nop               \n"
            "    jmp 2b            \n"
            "1:  movl $2b, %0      \n"
            : "=r" (ip));

    init_vmcb(vmcb_s);

    vmcb_s->state_save_area.cpl = 0;
    vmcb_s->state_save_area.efer = 0x1000; // svme set

    vmcb_s->state_save_area.cr4 = 0x690;
    vmcb_s->state_save_area.cr3 = 0;
    // PG[31] = 0, WP[16] = 1, NE[5]  = 1, ET[4]  = 1
    // TS[3]  = 1, MP[1]  = 1, PE[0]  = 1
    vmcb_s->state_save_area.cr0 = 0x1003b;

    if (!np_available) {
        vmcb_s->state_save_area.cr0 |= 0x80000000; // PG = 1
        vmcb_s->control_area.np_enable &= ~1;
    }

    vmcb_s->state_save_area.dr7 = 0x300;
    vmcb_s->state_save_area.dr6 = 0;

    asm volatile("pushf     \n"
            "popl %0   \n"
            : "=r" (eflags));

    vmcb_s->state_save_area.rflags = eflags;
    vmcb_s->state_save_area.rip = ip;
    vmcb_s->state_save_area.rsp = (l4_umword_t) stack + STACKSIZE;

    unsigned ofs;
    for (ofs = 0; ofs < STACKSIZE; ofs += L4_PAGESIZE) {
        stack[ofs] = 0;
        unsigned char c = stack[ofs];
        unsigned dummy;

        asm volatile("nop" : "=a"(dummy) : "0" (c));

        tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP, l4_fpage(
                (((l4_umword_t)(stack)) + ofs) & L4_PAGEMASK, L4_PAGESHIFT,
                L4_FPAGE_RW), l4_map_control(((l4_umword_t) stack) + ofs, 0,
                L4_MAP_ITEM_MAP));
        printf("msgtag raw=%08x, ofs=%08x\n", (unsigned) tag.raw, ofs);
    }

    tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP, l4_fpage(ip & L4_PAGEMASK,
            L4_PAGESHIFT, L4_FPAGE_RW), l4_map_control(ip, 0, L4_MAP_ITEM_MAP));

    printf("msgtag raw=%08lx, ip=%08lx\n", tag.raw, ip);

    idt[26] = 0x80000; // #13 general protection fault
    idt[27] = 0x8e00;

    idt[28] = 0x80000; // #14 page fault
    idt[29] = 0x8e00;

    // code segment 0x08
    gdt[2] = 0xffff;
    gdt[3] = 0xcf9b00;

    // stack segment 0x10
    gdt[4] = 0xffff;
    gdt[5] = 0xcf9300;

    // data segment 0x20
    gdt[8] = 0xffff;
    gdt[9] = 0xcff300;

    // tss 0x28
    gdt[10] = 0x67;
    gdt[11] = 0x8b00;

    unsigned idt0 = (unsigned) idt;
    tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP, l4_fpage(idt0 & L4_PAGEMASK,
            L4_PAGESHIFT, L4_FPAGE_RW),
            l4_map_control(idt0, 0, L4_MAP_ITEM_MAP));
    printf("msgtag raw=%08x, idt=%08x\n", (unsigned) tag.raw, idt0);

    unsigned gdt0 = (unsigned) gdt;
    tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP, l4_fpage(gdt0 & L4_PAGEMASK,
            L4_PAGESHIFT, L4_FPAGE_RW),
            l4_map_control(gdt0, 0, L4_MAP_ITEM_MAP));
    printf("msgtag raw=%08x, gdt=%08x\n", (unsigned) tag.raw, gdt0);

    //  printf("sizes vmcb=%x, control=%x,  state=%x\n",
    //         sizeof(*vmcb_s),
    //         sizeof(vmcb_s->control_area), sizeof(vmcb_s->sitate_save_area));

    printf("start rip=%llx\n", vmcb_s->state_save_area.rip);

    vmcb_s->state_save_area.rax = 8;

    vm_resume();

}


//  printf("rip=%08llx, rax=%llx, edx=%lx, ecx=%lx\n",
//         vmcb_s->state_save_area.rip,
//         vmcb_s->state_save_area.rax,
//         vcpu->r.dx, vcpu->r.cx);
//
//  printf("run vm stop, status=%s\n", vcpu->r.cx == 9 ? "success" : "failure");
//
//  l4_task_unmap(L4RE_THIS_TASK_CAP,
//                l4_obj_fpage(vm_task, 0, L4_FPAGE_RWX),
//                L4_FP_ALL_SPACES);
//}

#include <l4/sys/ktrace.h>
l4_cap_idx_t timer_irq;
void * timer_thread(void * );
void * timer_thread(void * data) {
    (void)data;
    l4_sleep(1000);
    fiasco_tbuf_log("FIRST");
    while(1){
        l4_sleep(200);
        fiasco_tbuf_log("Trig");
        printf("trigger\n");
        cnt_triggered++;
        l4_irq_trigger(timer_irq);
    }
}

#include <unistd.h>

int main(void)
{
    timer_irq = l4re_util_cap_alloc();
    if (l4_error(l4_factory_create_irq(l4re_env()->factory, timer_irq)))
      printf("irq creation failed\n");

    if (l4_error(l4_irq_attach(timer_irq, 0x120, L4_INVALID_CAP)))
        printf("irq attach failed\n");

    printf("VM testing\n");

    if (check_svm())
      {
        printf("No SVM CPU. Bye.\n");
        return 1;
      }

    l4_touch_rw(stack, sizeof(stack));
    l4_touch_rw(hdl_stack, sizeof(hdl_stack));

    pthread_t pthread_timer_thread;

    pthread_create(&pthread_timer_thread, NULL, timer_thread, NULL);

    run_test(0);

    if (!check_svm_npt())
        run_test(1);

    return 0;
}
