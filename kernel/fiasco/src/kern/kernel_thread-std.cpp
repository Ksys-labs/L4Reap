IMPLEMENTATION:

#include "config.h"
#include "cmdline.h"
#include "scheduler.h"
#include "factory.h"
#include "initcalls.h"
#include "ipc_gate.h"
#include "irq.h"
#include "map_util.h"
#include "mem_layout.h"
#include "mem_space_sigma0.h"
#include "task.h"
#include "thread.h"
#include "types.h"
#include "ram_quota.h"
#include "vlog.h"
#include "irq_controller.h"

static Vlog vlog;
static Scheduler scheduler;
static Icu icu;

enum Default_base_caps
{
  C_task      = 1,
  C_factory   = 2,
  C_thread    = 3,
  C_pager     = 4,
  C_log       = 5,
  C_icu       = 6,
  C_scheduler = 7,
  C_log_irq,

};

struct Sigma0_space_factory
{
  static void create(Mem_space *v, Ram_quota *q)
  { new (v) Mem_space_sigma0(q); }


  template< typename S >
  static void create(S *v)
  { new (v) S(); }

};


IMPLEMENT
void
Kernel_thread::init_workload()
{
  Lock_guard<Cpu_lock> g(&cpu_lock);
  //
  // create sigma0
  //

  char const *s;
  if (Config::Jdb &&
      (!strstr (Cmdline::cmdline(), " -nojdb")) &&
      ((s = strstr (Cmdline::cmdline(), " -jdb_cmd="))))
    {
      // extract the control sequence from the command line
      char ctrl[128];
      char *d;

      for (s=s+10, d=ctrl;
	   d < ctrl+sizeof(ctrl)-1 && *s && *s != ' '; *d++ = *s++)
	;
      *d = '\0';
      printf("JDB: exec cmd '%s'\n", ctrl);
      kdb_ke_sequence(ctrl);
    }

  // kernel debugger rendezvous
  if (strstr (Cmdline::cmdline(), " -wait"))
    kdb_ke("Wait");

#if 0
    {
      Lock_guard<Cpu_lock> g(&cpu_lock);
      kdb_ke("Wait");
    }
#endif


  Task *sigma0 = Task::create(Sigma0_space_factory(), Ram_quota::root,
      L4_fpage::mem(Mem_layout::Utcb_addr, Config::PAGE_SHIFT));
  sigma0_task = sigma0;
  assert(sigma0_task);

  check (sigma0->initialize());
  check (map(sigma0,          sigma0_task->obj_space(), sigma0_task, C_task, 0));
  check (map(Factory::root(), sigma0_task->obj_space(), sigma0_task, C_factory, 0));
  check (map(&scheduler,      sigma0_task->obj_space(), sigma0_task, C_scheduler, 0));
  check (map(&vlog,           sigma0_task->obj_space(), sigma0_task, C_log, 0));
  check (map(&icu,            sigma0_task->obj_space(), sigma0_task, C_icu, 0));

  sigma0_space = sigma0_task->mem_space();

  Thread *sigma0_thread = new (Ram_quota::root) Thread();
  // Config::sigma0_prio

  assert_kdb(sigma0_thread);
  check (map(sigma0_thread, sigma0_task->obj_space(), sigma0_task, C_thread, 0));

  Address sp = init_workload_s0_stack();
  check (sigma0_thread->control(Thread_ptr(false), Thread_ptr(false),
                                sigma0_task,
                                (void*)Mem_layout::Utcb_addr) == 0);

  check (sigma0_thread->ex_regs(Kip::k()->sigma0_ip, sp));

  //sigma0_thread->thread_lock()->clear();

  //
  // create the boot task
  //

  boot_task = Task::create(Space::Default_factory(), Ram_quota::root,
      L4_fpage::mem(Mem_layout::Utcb_addr, Config::PAGE_SHIFT+2));
  assert(boot_task);

  check (boot_task->initialize());

  Thread *boot_thread = new (Ram_quota::root) Thread();
  // Config::boot_prio

  assert_kdb (boot_thread);

  check (map(boot_task,   boot_task->obj_space(), boot_task, C_task, 0));
  check (map(boot_thread, boot_task->obj_space(), boot_task, C_thread, 0));

  check (boot_thread->control(Thread_ptr(C_pager), Thread_ptr(~0UL),
                              boot_task,
                              (void*)Mem_layout::Utcb_addr) == 0);
  check (boot_thread->ex_regs(Kip::k()->root_ip, Kip::k()->root_sp));

  Ipc_gate *s0_b_gate = Ipc_gate::create(Ram_quota::root, sigma0_thread, 4 << 4);

  check (s0_b_gate);
  check (map(s0_b_gate, boot_task->obj_space(), boot_task, C_pager, 0));

  //Cpu::cpus.cpu(0).tz_switch_to_ns();
  set_cpu_of(sigma0_thread, 0);
  set_cpu_of(boot_thread, 0);
  sigma0_thread->state_del_dirty(Thread_suspended);
  boot_thread->state_del_dirty(Thread_suspended);

  sigma0_thread->activate();
  check (obj_map(sigma0_task, C_factory,   1, boot_task, C_factory, 0).error() == 0);
  check (obj_map(sigma0_task, C_scheduler, 1, boot_task, C_scheduler, 0).error() == 0);
  check (obj_map(sigma0_task, C_log,       1, boot_task, C_log, 0).error() == 0);
  check (obj_map(sigma0_task, C_icu,       1, boot_task, C_icu, 0).error() == 0);

  boot_thread->activate();
}

IMPLEMENTATION [ia32,amd64]:

PRIVATE inline
Address
Kernel_thread::init_workload_s0_stack()
{
  // push address of kernel info page to sigma0's stack
  Address sp = Kip::k()->sigma0_sp - sizeof(Mword);
  // assume we run in kdir 1:1 mapping
  *reinterpret_cast<Address*>(sp) = Kmem::virt_to_phys(Kip::k());
  return sp;
}

IMPLEMENTATION [ux,arm,ppc32]:

PRIVATE inline
Address
Kernel_thread::init_workload_s0_stack()
{ return Kip::k()->sigma0_sp; }


// ---------------------------------------------------------------------------
IMPLEMENTATION [io]:

#include "io_space_sigma0.h"

PUBLIC
static
void
Sigma0_space_factory::create(Io_space *v)
{ new (v) Io_space_sigma0<Space>(); }
