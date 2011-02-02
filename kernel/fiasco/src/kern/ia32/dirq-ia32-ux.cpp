INTERFACE[ia32,amd64,ux]:
#include "std_macros.h"
#include "types.h"

IMPLEMENTATION[ia32,amd64,ux]:

#include <cassert>

#include "cpu_lock.h"
#include "globalconfig.h"
#include "globals.h"
#include "irq.h"
#include "logdefs.h"
#include "std_macros.h"
#include "thread.h"
#include "timer.h"

// screen spinner for debugging purposes
static inline void irq_spinners(int irqnum)
{
#ifdef CONFIG_IRQ_SPINNER
  Unsigned16 *p = (Unsigned16 *)Mem_layout::Adap_vram_cga_beg;
  p += (20 + current_cpu()) * 80 + irqnum;
  if (p < (Unsigned16 *)Mem_layout::Adap_vram_cga_end)
    (*p)++;
#else
  (void)irqnum;
#endif
}

/** Hardware interrupt entry point.  Calls corresponding Dirq instance's
    Dirq::hit() method.
    @param irqobj hardware-interrupt object
 */
extern "C" FIASCO_FASTCALL
void
irq_interrupt(Mword _irqobj, Mword ip)
{
  Mword irqobj = (Smword)((Signed32)_irqobj);
  Thread::assert_irq_entry();

  CNT_IRQ;
  (void)ip;

  // we're entered with disabled irqs
  Irq *i = nonull_static_cast<Irq*>((Irq_base*)irqobj);
  Irq::log_irq(i, irqobj);
  irq_spinners(i->irq());

#if defined(CONFIG_IA32) && defined(CONFIG_PROFILE)
  cpu_lock.lock();
#endif

  i->hit();

#if defined(CONFIG_IA32) && defined(CONFIG_PROFILE)
  cpu_lock.clear_irqdisable();
#endif
}


// We are entering with disabled interrupts!
extern "C" FIASCO_FASTCALL
void
thread_timer_interrupt (Address ip)
{
#if defined(CONFIG_IA32) && defined(CONFIG_PROFILE)
  cpu_lock.lock();
#endif

  Thread::assert_irq_entry();

  Timer::acknowledge();
  Timer::update_system_clock();

  Irq::log_timer_irq(Config::scheduler_irq_vector);
  (void)ip;
  irq_spinners(Config::scheduler_irq_vector);

  current_thread()->handle_timer_interrupt();

#if defined(CONFIG_IA32) && defined(CONFIG_PROFILE)
  cpu_lock.clear();
#endif
}

