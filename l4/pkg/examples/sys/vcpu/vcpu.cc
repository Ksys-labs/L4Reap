/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/sys/ipc.h>
#include <l4/sys/thread>
#include <l4/sys/factory>
#include <l4/sys/utcb.h>
#include <l4/sys/kdebug.h>
#include <l4/util/util.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/sys/debugger.h>

#include <l4/re/error_helper>

#include <l4/sys/task>
#include <l4/sys/irq>
#include <l4/sys/vcpu.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>


using L4Re::chksys;

static L4::Cap<L4::Irq> irq;

static char thread_stack[8 << 10];
static char hdl_stack[8 << 10];

static L4::Cap<L4::Thread> vcpu;
static L4::Cap<L4::Task> vcpu_task;

typedef l4_vcpu_state_t SSA;

static SSA *vcpu_state;
static l4_umword_t volatile *vcpu_psr;
static unsigned long gs;
static unsigned long ds;



extern char my_super_code[];

#if defined __amd64__ || defined __i386__
asm
(
  ".p2align 12\n"
  ".global my_super_code \n"
  "my_super_code: \n"
  "1: add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; add $4, %eax; ud2a; add $4, %eax; add $4, %eax;add $4, %eax; add $4, %eax; jmp 1b \n"
  );


static void print_upc()
{
  printf("upcall %lx %lx %lx\n", vcpu_state->r.trapno, vcpu_state->r.err, vcpu_state->r.pfa);
}

static void print_state()
{
  l4_umword_t psr = *vcpu_psr;
  printf("EIP=%08lx ESP=%08lx PSR=%08lx\n"
         "EAX=%08lx EBX=%08lx ECX=%08lx EDX=%08lx\n"
         "ESI=%08lx EDI=%08lx EBP=%08lx\n"
#ifndef __amd64__
	 "DS=%04lx ES=%04lx FS=%04lx GS=%04lx\n"
#endif
	 "TAG=%08lx SRC=%08lx SPSR=%08lx\n",
         vcpu_state->r.ip,
         vcpu_state->r.sp,
	 psr,
         vcpu_state->r.ax,
         vcpu_state->r.bx,
         vcpu_state->r.cx,
         vcpu_state->r.dx,
         vcpu_state->r.si,
         vcpu_state->r.di,
         vcpu_state->r.bp,
#ifndef __amd64__
         vcpu_state->r.ds,
         vcpu_state->r.es,
         vcpu_state->r.fs,
         vcpu_state->r.gs,
#endif
         vcpu_state->i.tag.raw,
         vcpu_state->i.label,
	 vcpu_state->saved_state);
}

static bool is_page_fault(SSA const *v)
{ return v->r.trapno == 0xe; }


static void setup_user_state(SSA *v)
{
  asm volatile ("mov %%gs, %0" : "=r"(gs));
  asm volatile ("mov %%ds, %0" : "=r"(ds));
  v->saved_state = 0x27;
#ifndef __amd64__
  v->r.gs = ds;
  v->r.fs = ds;
  v->r.es = ds;
  v->r.ds = ds;
#endif
  v->r.ss = ds;
  v->r.ip = 0x10000;
  v->r.sp = 0x40000;
}

#elif defined __arm__
asm
(
  ".p2align 12\n"
  ".global my_super_code \n"
  "my_super_code: \n"
  "1: add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4; swi 0; mrc 8, 0, r0, cr7, cr14\n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   add r0, r0, #4 \n"
  "   b 1b           \n"
  );


static void print_upc()
{
  printf("upcall %lx %lx\n", vcpu_state->r.err, vcpu_state->r.pfa);
}

static void print_state()
{
  l4_umword_t psr = *vcpu_psr;
  printf("R[00]: %08lx %08lx %08lx %08lx\n"
         "R[04]: %08lx %08lx %08lx %08lx\n"
         "R[08]: %08lx %08lx %08lx %08lx\n"
         "R[12]: %08lx %08lx %08lx %08lx\n"
	 "PSR=%08lx\n"
	 "TAG=%08lx SRC=%08lx SPSR=%08lx\n",
         vcpu_state->r.r[0],
         vcpu_state->r.r[1],
         vcpu_state->r.r[2],
         vcpu_state->r.r[3],
         vcpu_state->r.r[4],
         vcpu_state->r.r[5],
         vcpu_state->r.r[6],
         vcpu_state->r.r[7],
         vcpu_state->r.r[8],
         vcpu_state->r.r[9],
         vcpu_state->r.r[10],
         vcpu_state->r.r[11],
         vcpu_state->r.r[12],
         vcpu_state->r.sp,
         vcpu_state->r.lr,
         vcpu_state->r.ip,
         psr,
         vcpu_state->i.tag.raw,
         vcpu_state->i.label,
	 vcpu_state->saved_state);
}

static bool is_page_fault(SSA const *v)
{ return (v->r.err & 0xf00000) == 0x300000; }

static void setup_user_state(SSA *v)
{
  v->saved_state = 0x27;
  v->r.ip = 0x10000;
  v->r.sp = 0x40000;
}
#endif


static void handler(void)
{

#if defined __amd64__ || defined __i386__
  asm volatile (
      "mov %0, %%es \t\n"
      "mov %0, %%ds \t\n"
      "mov %1, %%gs \t\n" : : "r"(ds), "r"(gs));
#endif
//upc:
  print_upc();
#if 0
  l4_umword_t psr = *vcpu_psr;
#endif
  print_state();

#if 0
  *vcpu_psr |= 1;
  if (psr & 2)
    {
      vcpu_state->tag = l4_ipc_wait(l4_utcb(), &vcpu_state->label, L4_IPC_NEVER); //L4_IPC_RECV_TIMEOUT_0);
      goto upc;

    }
#endif

  l4_addr_t a = (l4_addr_t)my_super_code;

  if (is_page_fault(vcpu_state))
    {
      vcpu_task->map(L4Re::This_task, l4_fpage(a, 12, L4_FPAGE_RWX), 0x10000);
    }

  vcpu_state->saved_state |= L4_VCPU_F_PAGE_FAULTS;

  printf("resume\n");
  L4::Cap<L4::Thread> self;
  self.invalidate();
  self->vcpu_resume_commit(self->vcpu_resume_start());
  while(1)
    ;
}

static void vcpu_thread(void)
{
  //*vcpu_psr |= 2;
  printf("HELLO VCPU\n");
  memset(hdl_stack, 0, sizeof(hdl_stack));
  setup_user_state(vcpu_state);

  L4::Cap<L4::Thread> self;
  printf("IRET\n");
  vcpu_state->user_task = vcpu_task.cap();
  self->vcpu_resume_commit(self->vcpu_resume_start());
  printf("IRET uh?!\n");

  *vcpu_psr |= 3;
  while (1)
    {
    }

  l4_msgtag_t x;
  while (1) {
    x = l4_ipc_call(0x1234 << L4_CAP_SHIFT, l4_utcb(), l4_msgtag(0, 0, 0, 0), L4_IPC_NEVER);
    l4_sleep(1000);
    outstring("An int3 -- you should see this\n");
    outnstring("345", 3);
  }

}

int main(void)
{
  l4_msgtag_t tag;
  l4_utcb_t *u = l4_utcb();
  l4_exc_regs_t exc;
  l4_umword_t mr0, mr1;

  printf("vCPU example\n");

  l4_debugger_set_object_name(l4re_env()->main_thread, "vcputest");

  // new task
  vcpu_task = L4Re::Util::cap_alloc.alloc<L4::Task>();
  if (!vcpu_task.is_valid())
    return 2;

  chksys(L4Re::Env::env()->factory()->create_task(vcpu_task, l4_fpage_invalid()), "create task");
  l4_debugger_set_object_name(vcpu_task.cap(), "vcpu task");

  /* new thread */
  vcpu = L4Re::Util::cap_alloc.alloc<L4::Thread>();
  if (!vcpu.is_valid())
    return 1;

  l4_touch_rw(thread_stack, sizeof(thread_stack));

  chksys(L4Re::Env::env()->factory()->create_thread(vcpu), "create thread");
  l4_debugger_set_object_name(vcpu.cap(), "vcpu thread");

  // use two consecurity UTCBs
  l4_utcb_t *vcpu_utcb  = (l4_utcb_t *)l4re_env()->first_free_utcb;
  vcpu_state = reinterpret_cast<SSA*>((l4_umword_t)vcpu_utcb + L4_UTCB_OFFSET);
  vcpu_state->entry_sp = (l4_umword_t)hdl_stack + sizeof(hdl_stack);
  vcpu_state->entry_ip = (l4_umword_t)handler;
  vcpu_psr = &vcpu_state->state;

  printf("VCPU: utcb = %p, vcpu_state = %p\n", vcpu_utcb, vcpu_state);

  // control thread
  L4::Thread::Attr attr;
  attr.pager(L4::cap_reinterpret_cast<L4::Thread>(L4Re::Env::env()->rm()));
  attr.exc_handler(L4Re::Env::env()->main_thread());
  attr.vcpu_enable(1);
  attr.bind(vcpu_utcb, L4Re::This_task);

  chksys(vcpu->control(attr), "control");

  // launch thread
  tag = vcpu->ex_regs((l4_umword_t)vcpu_thread,
                      (l4_umword_t)thread_stack + sizeof(thread_stack),
                      0);


  chksys(irq->attach(200, vcpu));

  if (l4_msgtag_has_error(tag))
    return 3;
  l4_sleep(10);
  for (int i = 0; i < 20; ++i)
    {
      l4_ipc_send(vcpu.cap(), u, l4_msgtag(10 + i, 0, 0, 0), L4_IPC_NEVER);
    }
  l4_sleep(10);
  for (int i = 0; i < 20; ++i)
    {
      l4_ipc_send(vcpu.cap(), u, l4_msgtag(40 + i, 0, 0, 0), L4_IPC_NEVER);
    }

  /* Pager/Exception loop */
  if (l4_msgtag_has_error(tag = l4_ipc_receive(vcpu.cap(), u, L4_IPC_NEVER)))
    {
      printf("l4_ipc_receive failed");
      return 1;
    }

  memcpy(&exc, l4_utcb_exc(), sizeof(exc));
  mr0 = l4_utcb_mr()->mr[0];
  mr1 = l4_utcb_mr()->mr[1];

  for (;;)
    {
    }

  return 0;
}
