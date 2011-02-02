INTERFACE:

#include "thread_object.h"

class Kernel_thread : public Thread_object
{
private:
  /**
   * Frees the memory of the initcall sections.
   *
   * Virtually initcall sections are freed by not marking them
   * reserved in the KIP. This method just invalidates the contents of
   * the memory, by filling it with some invalid data and may be
   * unmapping it.
   */
  void	free_initcall_section();
  void	bootstrap()		asm ("call_bootstrap") FIASCO_FASTCALL;
  void	bootstrap_arch();
  void	run();
  void  do_idle() __attribute__((noreturn));

protected:
  void	init_workload();
};

INTERFACE [kernel_can_exit]:

EXTENSION class Kernel_thread
{
private:
  void  arch_exit() __attribute__((noreturn));
};

IMPLEMENTATION:

#include <cstdlib>
#include <cstdio>

#include "config.h"
#include "cpu.h"
#include "delayloop.h"
#include "globals.h"
#include "helping_lock.h"
#include "kernel_task.h"
#include "processor.h"
#include "task.h"
#include "thread.h"
#include "thread_state.h"
#include "timer.h"


PUBLIC
Kernel_thread::Kernel_thread() : Thread_object(Thread::Kernel)
{}

PUBLIC inline
Mword *
Kernel_thread::init_stack()
{ return _kernel_sp; }

// the kernel bootstrap routine
IMPLEMENT FIASCO_INIT
void
Kernel_thread::bootstrap()
{
  // Initializations done -- Helping_lock can now use helping lock
  Helping_lock::threading_system_active = true;

  state_change_dirty (0, Thread_ready);		// Set myself ready

  set_cpu_of(this, Cpu::boot_cpu()->id());
  Timer::init_system_clock();
  Sched_context::rq(cpu()).set_idle(this->sched());

  Kernel_task::kernel_task()->mem_space()->make_current();

  // Setup initial timeslice
  set_current_sched(sched());

  Timer::enable();

  bootstrap_arch();

  Per_cpu_data::run_late_ctors(0);

  Proc::sti();
  printf("Calibrating timer loop... ");
  // Init delay loop, needs working timer interrupt
  if (running)
    Delay::init();
  printf("done.\n");

  run();
}

/**
 * The idle loop
 * NEVER inline this function, because our caller is an initcall
 */
IMPLEMENT FIASCO_NOINLINE FIASCO_NORETURN
void
Kernel_thread::run()
{
  free_initcall_section();

  // No initcalls after this point!

  kernel_context(cpu(), this);

  // init_workload cannot be an initcall, because it fires up the userland
  // applications which then have access to initcall frames as per kinfo page.
  init_workload();

  do_idle();
}

// ------------------------------------------------------------------------
IMPLEMENTATION [!arch_idle]:

PUBLIC inline NEEDS["processor.h"]
void
Kernel_thread::idle_op()
{
  if (Config::hlt_works_ok)
    Proc::halt();			// stop the CPU, waiting for an int
  else
    Proc::pause();
}

// ------------------------------------------------------------------------
IMPLEMENTATION [!kernel_can_exit]:

IMPLEMENT
void
Kernel_thread::do_idle()
{
  while (1)
    idle_op();
}

// ------------------------------------------------------------------------
IMPLEMENTATION [kernel_can_exit]:

IMPLEMENT
void
Kernel_thread::do_idle()
{
  while (running)
    {
      Mem::rmb();
      idle_op();
    }

  puts ("\nExiting, wait...");

  Reap_list rl;
  Task *sigma0 = static_cast<Task *>(sigma0_task);
  delete sigma0;

  rl = Reap_list();
  boot_task->initiate_deletion(rl.list());
  boot_task->destroy(rl.list());
  rl.del();
  delete boot_task; // Nuke everything else

  Helping_lock::threading_system_active = false;

  arch_exit();
}
