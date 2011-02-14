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
}

IMPLEMENT FIASCO_INIT FIASCO_NOINLINE
void
Startup::stage2()
{
  Banner::init();
  Kip_init::init();
  Kmem_alloc::init();

  // initialize initial page tables (also used for other CPUs later)
  Kmem::init_mmu();

  // Initialize cpu-local data management and run constructors for CPU 0
  Per_cpu_data::init_ctors(Kmem_alloc::allocator());
  Per_cpu_data_alloc::alloc(0);
  Per_cpu_data::run_ctors(0);

  // set frequency in KIP to that of the boot CPU
  Kip_init::init_freq(Cpu::cpus.cpu(0));

  if (Io_apic::init())
    {
      Config::apic = true;
      Pic::disable_all_save();
      Dirq_io_apic::init();

      // If we use the IOAPIC, we route our timer IRQ to
      // Config::Apic_timer_vector, even with PIT or RTC
      enum { Pic_base = 0x20, Pic_irqs = 0x10 };

      if (   Config::scheduler_irq_vector >= Pic_base
	  && Config::scheduler_irq_vector < Pic_base + Pic_irqs)
	{
	  unsigned const pic_pin
	    = Io_apic::legacy_override(Config::scheduler_irq_vector - Pic_base);
	  // assume the legacy irqs are routet to IO-APIC 0
	  Io_apic_entry e = Io_apic::apic(0)->read_entry(pic_pin);
	  e.vector(Config::Apic_timer_vector);
	  Io_apic::apic(0)->write_entry(pic_pin, e);
	  Config::scheduler_irq_vector = Config::Apic_timer_vector;
	}
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
  Timer::master_cpu(0);
  Apic::check_still_getting_interrupts();
//  Cpu::init_global_features();
}
