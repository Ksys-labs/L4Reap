IMPLEMENTATION[ia32,amd64]:

#include <cstdlib>
#include <cstdio>

#include "apic.h"
#include "banner.h"
#include "boot_console.h"
#include "boot_info.h"
#include "config.h"
#include "cpu.h"
#include "dirq.h"
#include "dirq_io_apic.h"
#include "fpu.h"
#include "idt.h"
#include "initcalls.h"
#include "ipi.h"
#include "kernel_console.h"
#include "kernel_task.h"
#include "kip_init.h"
#include "kernel_uart.h"
#include "kmem.h"
#include "kmem_alloc.h"
#include "per_cpu_data.h"
#include "per_cpu_data_alloc.h"
#include "pic.h"
#include "static_init.h"
#include "std_macros.h"
#include "thread.h"
#include "timer.h"
#include "utcb_init.h"
#include "vmem_alloc.h"

#include "io_apic.h"

IMPLEMENT FIASCO_INIT FIASCO_NOINLINE
void
Startup::stage1()
{
  Boot_info::init();
  Config::init();
  if (Kernel_uart::init(Kernel_uart::Init_before_mmu))
    Banner::init();
}

IMPLEMENT FIASCO_INIT FIASCO_NOINLINE
void
Startup::stage2()
{
  Kip_init::init();
  Kmem_alloc::init();

  // initialize initial page tables (also used for other CPUs later)
  Kmem::init_mmu();

  if (Kernel_uart::init(Kernel_uart::Init_after_mmu))
    Banner::init();

  // Initialize cpu-local data management and run constructors for CPU 0
  Per_cpu_data::init_ctors(Kmem_alloc::allocator());
  Per_cpu_data_alloc::alloc(0);
  Per_cpu_data::run_ctors(0);

  // set frequency in KIP to that of the boot CPU
  Kip_init::init_freq(Cpu::cpus.cpu(0));

  bool use_io_apic = Io_apic::init();
  if (use_io_apic)
    {
      Config::apic = true;
      Pic::disable_all_save();
      Dirq_io_apic::init();
    }
  else
    {
      Pic::init();
      Dirq_pic_pin::init();
    }

  Kernel_task::init(); // enables current_mem_space()
  Vmem_alloc::init();

  // initialize initial TSS, GDT, IDT
  Kmem::init_cpu(Cpu::cpus.cpu(0));
  Utcb_init::init();
  Idt::init();
  Fpu::init(0);
  Apic::init();
  Ipi::cpu(0).init();
  Timer::init();
  int timer_irq = Timer::irq_line();
  if (use_io_apic)
    {
      // If we use the IOAPIC, we route our timer IRQ to
      // Config::Apic_timer_vector, even with PIT or RTC
      Config::scheduler_irq_vector = Config::Apic_timer_vector;

      if (timer_irq >= 0)
	{
	  unsigned const pic_pin = Io_apic::legacy_override(timer_irq);
	  // assume the legacy irqs are routet to IO-APIC 0
	  Io_apic_entry e = Io_apic::apic(0)->read_entry(pic_pin);
	  e.vector(Config::Apic_timer_vector);
	  Io_apic::apic(0)->write_entry(pic_pin, e);
	}
    }
  else
    {
      if (timer_irq >= 0)
	Config::scheduler_irq_vector = 0x20 + timer_irq;
      else
	Config::scheduler_irq_vector = Config::Apic_timer_vector;
    }

  Idt::set_vectors_run();
  Timer::master_cpu(0);
  Apic::check_still_getting_interrupts();
//  Cpu::init_global_features();
}
