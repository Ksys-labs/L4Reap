/*
 * Author(s): Adam Lackorzynski
 *            Alexander Warg
 *            Matthias Lange <mlange@sec.t-labs.tu-berlin.de>
 *
 * (c) 2008-2012 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/factory.h>
#include <l4/sys/task.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/vm.h>
#include <l4/sys/err.h>
#include <l4/sys/vcpu.h>
#include <l4/sys/thread.h>

#include <l4/util/util.h>
#include <l4/util/cpu.h>
#include <l4/util/kip.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/vcpu/vcpu.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vmcs.h"

#define STACKSIZE (8<<10)

static char stack[STACKSIZE];
static char hdl_stack[STACKSIZE];

static unsigned long idt[32 * 2] __attribute__((aligned(4096)));
static unsigned long gdt[32 * 2] __attribute__((aligned(4096)));
static l4_umword_t old_rip;

void test_func(void) __attribute__((aligned(4096)));

void vm_resume(void);
void handle_vmexit(void);
l4_vcpu_state_t *vcpu;
l4_addr_t *vmcs_s;

static l4_umword_t test_end;
static l4_cap_idx_t vm_task;

L4_INLINE
void vmwrite(void *vmcs, unsigned field, unsigned long long val)
{ l4_vm_vmx_write(vmcs, field, val); }

static void init_vmcs(void *vmcs)
{
  vmwrite(vmcs, VMX_GUEST_CS_SEL, 0x8);
  vmwrite(vmcs, VMX_GUEST_CS_ACCESS_RIGHTS, 0xd09b);
  vmwrite(vmcs, VMX_GUEST_CS_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_CS_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_SS_SEL, 0x10);
  vmwrite(vmcs, VMX_GUEST_SS_ACCESS_RIGHTS, 0xc093);
  vmwrite(vmcs, VMX_GUEST_SS_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_SS_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_DS_SEL, 0x20);
  vmwrite(vmcs, VMX_GUEST_DS_ACCESS_RIGHTS, 0xc0f3);
  vmwrite(vmcs, VMX_GUEST_DS_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_DS_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_ES_SEL, 0x0);
  vmwrite(vmcs, VMX_GUEST_ES_ACCESS_RIGHTS, 0x14003);
  vmwrite(vmcs, VMX_GUEST_ES_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_ES_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_FS_SEL, 0x0);
  vmwrite(vmcs, VMX_GUEST_FS_ACCESS_RIGHTS, 0x1c0f3);
  vmwrite(vmcs, VMX_GUEST_FS_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_FS_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_GS_SEL, 0x0);
  vmwrite(vmcs, VMX_GUEST_GS_ACCESS_RIGHTS, 0x1c0f3);
  vmwrite(vmcs, VMX_GUEST_GS_LIMIT, 0xffffffff);
  vmwrite(vmcs, VMX_GUEST_GS_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_GDTR_LIMIT, 0x3f);
  vmwrite(vmcs, VMX_GUEST_GDTR_BASE, (l4_umword_t)gdt);

  vmwrite(vmcs, VMX_GUEST_LDTR_SEL, 0x0);
  vmwrite(vmcs, VMX_GUEST_LDTR_ACCESS_RIGHTS, 0x10000);
  vmwrite(vmcs, VMX_GUEST_LDTR_LIMIT, 0);
  vmwrite(vmcs, VMX_GUEST_LDTR_BASE, 0);

  vmwrite(vmcs, VMX_GUEST_IDTR_LIMIT, 0xff);
  vmwrite(vmcs, VMX_GUEST_IDTR_BASE, (l4_umword_t)idt);

  vmwrite(vmcs, VMX_GUEST_TR_SEL, 0x28);
  vmwrite(vmcs, VMX_GUEST_TR_ACCESS_RIGHTS, 0x108b);
  vmwrite(vmcs, VMX_GUEST_TR_LIMIT, 67);
  vmwrite(vmcs, VMX_GUEST_TR_BASE, 0);

  vmwrite(vmcs_s, VMX_GUEST_CR0, 0x0001003b);

}

static int check_vmx(void)
{
  l4_umword_t ax, bx, cx, dx;

  if (!l4util_cpu_has_cpuid())
    return 1;

  l4util_cpu_cpuid(0x1, &ax, &bx, &cx, &dx);

  if (!(cx & (1<<5)))
    {
      printf("CPU does not support VMX.\n");
      return 1;
    }

  return 0;
}

static void handler(void)
{
  printf("Received interrupt %d\n", (int)vcpu->i.label);
  vm_resume();
}

void handle_vmexit(void)
{
  static unsigned int i = 0;
  ++i;

  l4_msgtag_t tag;
  l4_uint32_t interrupt_info;

  printf("iteration=%d, rip=0x%lx -> 0x%lx\n",
         i, old_rip,
         l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_RIP));

  l4_uint32_t exit_reason = l4_vm_vmx_read_32(vmcs_s, VMX_EXIT_REASON);
  if ((exit_reason & (1<<31)))
    printf("VM entry failure, reason %d\n", (exit_reason & 0xffff));
  else
    {
      switch (exit_reason)
        {
        case 0:
          printf("Exception or NMI at guest ip 0x%lx, checking interrupt info\n",
                 l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_RIP));
          interrupt_info = l4_vm_vmx_read_32(vmcs_s, VMX_EXIT_INTERRUPT_INFO);
          // check valid bit
          if (!(interrupt_info & (1<<31)))
            printf("Interrupt info not valid\n");

          printf("interrupt vector=%d, type=%d, error code valid=%d\n",
                 (interrupt_info & 0xFF), ((interrupt_info & 0x700) >> 8),
                 ((interrupt_info & 0x800) >> 11));

          if ((interrupt_info & (1 << 11))) // interrupt error code valid?
            printf("interrupt error=0x%x\n",
                   l4_vm_vmx_read_32(vmcs_s, VMX_EXIT_INTERRUPT_ERROR));

          printf("cr0=%lx\n", l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_CR0));
          printf("eax: %lx ebx: %lx esi: %lx\n", vcpu->r.ax, vcpu->r.bx, vcpu->r.si);

          if (((interrupt_info & 0x700)>>8) == 3 &&
              (interrupt_info & 0xff) == 14)
            {
              l4_umword_t fault_addr = l4_vm_vmx_read_nat(vmcs_s, VMX_EXIT_QUALIFICATION);
              printf("detected pagefault @ %lx\n", fault_addr);
              tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                                l4_fpage(fault_addr & L4_PAGEMASK, L4_PAGESHIFT, L4_FPAGE_RW),
                                l4_map_control(fault_addr, 0, L4_MAP_ITEM_MAP));
              if (l4_error(tag))
                printf("Error mapping page\n");
              break;
            }

          // increment rip to continue
          l4_umword_t l = l4_vm_vmx_read_32(vmcs_s, VMX_EXIT_INSTRUCTION_LENGTH);
          l4_umword_t ip = l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_RIP);
          printf("insn length: %lx new rip=%lx\n", l, ip);
          vmwrite(vmcs_s, VMX_GUEST_RIP, ip+l);
          break;
        case 1:
          printf("External interrupt\n");
          break;
        case 48: // EPT violation
          printf("EPT violation\n");
          l4_umword_t q = l4_vm_vmx_read_nat(vmcs_s, VMX_EXIT_QUALIFICATION);
          printf("  exit qualifiction: %lx\n", q);
          printf("  guest phys = %llx,  guest linear: %lx\n", l4_vm_vmx_read_64(vmcs_s, 0x2400), l4_vm_vmx_read_nat(vmcs_s, 0x640a));
          printf("  guest cr0 = %lx\n", l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_CR0));

            {
              l4_umword_t fault_addr = l4_vm_vmx_read_64(vmcs_s, 0x2400);
              printf("detected pagefault @ %lx\n", fault_addr);
              tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                                l4_fpage(fault_addr & L4_PAGEMASK, L4_PAGESHIFT, L4_FPAGE_RWX),
                                l4_map_control(fault_addr, 0, L4_MAP_ITEM_MAP));
              if (l4_error(tag))
                printf("Error mapping page\n");
            }
          break;
        default:
          printf("Exit reason %d\n", exit_reason);
          break;
        }
    }
}

void vm_resume(void)
{
  int r;
  l4_msgtag_t tag;

  tag = l4_thread_vcpu_resume_commit(L4_INVALID_CAP, l4_thread_vcpu_resume_start());
  r = l4_error(tag);
  if (r)
    printf("vm_resume failed: %s (%d)\n", l4sys_errtostr(r), r);

  handle_vmexit();
  old_rip = l4_vm_vmx_read_nat(vmcs_s, VMX_GUEST_RIP);

  if (old_rip <= test_end)
    vm_resume();
}


static l4_vcpu_state_t *get_state_mem(l4_addr_t *extstate)
{
  static int done;
  long r;
  l4_msgtag_t tag;
  static l4_addr_t ext_state;

  if (done)
    {
      *extstate = ext_state;
      return vcpu;
    }

  r = l4vcpu_ext_alloc(&vcpu, &ext_state, L4_BASE_TASK_CAP, l4re_env()->rm);
  if (r)
    {
      printf("Getting state mem failed: %ld\n", r);
      exit(1);
    }

  vcpu->state = L4_VCPU_F_FPU_ENABLED;
  vcpu->saved_state = L4_VCPU_F_USER_MODE | L4_VCPU_F_FPU_ENABLED | L4_VCPU_F_IRQ;

  vcpu->entry_ip = (l4_umword_t)handler;
  l4_umword_t *stack = (l4_umword_t *)(hdl_stack + STACKSIZE);
  --stack;
  *stack = 0;
  vcpu->entry_sp = (l4_umword_t)stack;

  tag = l4_thread_vcpu_control_ext(L4_INVALID_CAP, (l4_addr_t)vcpu);
  r = l4_error(tag);
  if (r)
    {
      printf("Could not enable ext vCPU: %ld\n", r);
      exit(1);
    }

  done = 1;

  *extstate = ext_state;

  for (unsigned i = 0x480; i < 0x48d; ++i)
    printf("VMX: CAP MSR[%3x]: %llx\n", i, l4_vm_vmx_get_caps(vcpu, i));

  for (unsigned i = 0x481; i < 0x485; ++i)
    printf("VMX: CAP MSR[%3x]: default1: %x\n", i, l4_vm_vmx_get_caps_default1(vcpu, i));

  return vcpu;
}

static void run_test(int ept_available)
{
  l4_umword_t eflags;
  l4_msgtag_t tag;
  vm_task = l4re_util_cap_alloc();
  l4_umword_t ip, marker;
  l4_addr_t vmcs;

  get_state_mem(&vmcs);

  printf("run test, ept_available=%d\n", ept_available);

  if (l4_is_invalid_cap(vm_task))
    {
      printf("No more caps.\n");
      return;
    }

  tag = l4_factory_create_vm(l4re_env()->factory, vm_task);
  if (l4_error(tag))
  {
    printf("Failed to create new task\n");
    exit(1);
  }

  vmcs_s = (l4_addr_t *)vmcs;

  vcpu->user_task = vm_task;

  // init registers
  vcpu->r.dx = 1;
  vcpu->r.cx = 2;
  vcpu->r.bx = 3;
  vcpu->r.bp = 4;
  vcpu->r.si = 5;
  vcpu->r.di = 6;

#if 1
  asm volatile("    jmp 1f;           \n"
               "2:  nop               \n"
               "    nop               \n"
               "    nop               \n"
			         "    addl %%edx,%%eax  \n"
//               "    ud2               \n"
			         "    addl %%edx,%%eax  \n"
               "    int3              \n"
               "3:  nop               \n"
               "    nop               \n"
			         "    addl %%edx,%%eax  \n"
               "    int3              \n"
               "    nop               \n"
               "    nop               \n"
               "    movl %%eax, %%ecx \n"
               "    addl %%edx, %%ecx \n"
               "4:                    \n"
               "    movl $1, %%eax    \n"
			         "    addl %%edx,%%eax  \n"
  //             "    ud2               \n"
               "1:  mov $2b, %0      \n"
               "    mov $3b, %1      \n"
               "    mov $4b, %2      \n"
               : "=r" (ip), "=r" (marker), "=r" (test_end));
#else
   asm volatile("    jmp 2f;           \n"
                "1:                    \n"
                "    nop               \n"
                "    nop               \n"
                "    movl $1, %1       \n"
                "    jmp 3f;           \n"
                "2:                    \n"
                "    movl $1b, %0      \n"
                "    movl $0, %1       \n"
                "3:                    \n"
              :"=r"(ip), "=r"(marker));


  if (marker) {
     test_func();
  }
#endif

  printf("ip=%lx\n", ip);

  init_vmcs((void *)vmcs);

  asm volatile("pushf     \n"
               "pop %0   \n"
               : "=r" (eflags));

  // clear interrupt
  eflags = (eflags & 0xfffffdff);
  eflags &= ~(0x1 << 17);

  vmwrite(vmcs_s, VMX_GUEST_RSP, (l4_umword_t)stack + STACKSIZE);
  vmwrite(vmcs_s, VMX_GUEST_RFLAGS, eflags);
  vmwrite(vmcs_s, VMX_GUEST_RIP, ip);
  vmwrite(vmcs_s, VMX_GUEST_CR0, 0x0001003b);
  vmwrite(vmcs_s, VMX_GUEST_CR4, 0x2690);
  vmwrite(vmcs_s, VMX_GUEST_DR7, 0x300);
  vmwrite(vmcs_s, VMX_VMCS_LINK_PTR, 0xffffffffffffffffULL);

  vmwrite(vmcs_s, VMX_EXCEPTION_BITMAP, 0xffffffff);
  vmwrite(vmcs_s, VMX_PF_ERROR_CODE_MATCH, 0);

  unsigned ofs;
  for (ofs = 0; ofs < STACKSIZE; ofs += L4_PAGESIZE)
    {
      stack[ofs] = 0;
      unsigned char c = stack[ofs];
      unsigned dummy;

      asm volatile("nop" : "=a"(dummy) : "0" (c));

      tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                        l4_fpage((((l4_umword_t)(stack)) + ofs) & L4_PAGEMASK,
                                 L4_PAGESHIFT, L4_FPAGE_RWX),
                        l4_map_control(((l4_umword_t)stack) +  ofs, 0,
                                       L4_MAP_ITEM_MAP));
    }

  tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                    l4_fpage(ip & L4_PAGEMASK, L4_PAGESHIFT, L4_FPAGE_RWX),
                    l4_map_control(ip, 0, L4_MAP_ITEM_MAP));

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

  unsigned idt0 = (unsigned long)idt;
  tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                    l4_fpage(idt0 & L4_PAGEMASK, L4_PAGESHIFT, L4_FPAGE_RW),
                    l4_map_control(idt0, 0, L4_MAP_ITEM_MAP));

  unsigned gdt0 = (unsigned long)gdt;
  tag = l4_task_map(vm_task, L4RE_THIS_TASK_CAP,
                    l4_fpage(gdt0 & L4_PAGEMASK, L4_PAGESHIFT, L4_FPAGE_RW),
                    l4_map_control(gdt0, 0, L4_MAP_ITEM_MAP));

  vm_resume();

  printf("eax=%lx, edx=%lx, ecx=%lx\n", vcpu->r.ax, vcpu->r.dx, vcpu->r.cx);
  printf("run vm stop, status=%s\n", ((vcpu->r.ax == 2) && (vcpu->r.cx == 4)) ? "success" : "failure");
}

__attribute__((aligned(4096))) int main(void)
{
  printf("VMX testing\n");

  if (check_vmx())
    {
      printf("No VT CPU. Bye.\n");
      return 1;
    }

  l4_touch_rw(stack, sizeof(stack));
  l4_touch_rw(hdl_stack, sizeof(hdl_stack));

  run_test(1);

  printf("VM test exited\n");

  l4_sleep_forever();

  return 0;
}

__attribute__((aligned(4096))) void test_func(void)
{
  unsigned long dummy;
  asm volatile("2:  nop               \n"
               "    nop \n"
               "    addl %%edx,%%eax  \n"
               "    ud2               \n"
               "    addl %%edx,%%eax  \n"
               "    int3              \n"
               "3:  nop               \n"
               "    nop               \n"
               "    addl %%edx,%%eax  \n"
               "    int3              \n"
               "    nop               \n"
               "    nop               \n"
               "    movl %%eax, %%ecx \n"
               "    addl %%edx, %%ecx \n"
               "4:                    \n"
               "    addl %%edx,%%eax  \n"
               "    ud2               \n"
               : "=r" (dummy)
               );

  asm volatile("1:  int3        \n"
               "    ud2         \n"
               "    jmp 1b      \n");
}
