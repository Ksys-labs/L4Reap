IMPLEMENTATION [arm]:

#include "dirq.h"
#include "pic.h"
#include "std_macros.h"
#include "timer.h"
#include "thread.h"
#include "logdefs.h"


IMPLEMENTATION:

#include "irq_chip_generic.h"
#include "irq.h"

extern "C"
void irq_handler()
{
    Thread::assert_irq_entry();

    if (Pic::Multi_irq_pending)
      {
next_irq:
	Mword irqs = Pic::pending();

	if (Pic::is_pending(irqs, Config::Scheduling_irq))
	  {
	    Irq::log_timer_irq(Config::Scheduling_irq);
	    Timer::acknowledge();
	    Timer::update_system_clock();
	    current_thread()->handle_timer_interrupt();
	    goto next_irq;
	  }

	for (unsigned irq=0; irq < sizeof(Mword)*8; irq++)
	  {
	    if (irqs & (1<<irq))
	      {
	        if (Thread::check_for_ipi(irq))
	          goto next_irq;

		Irq *i = nonull_static_cast<Irq*>(Irq_chip_gen::irqs[irq]);

		Irq::log_irq(i, irq);

                i->pin()->mask();
		i->pin()->hit();
		goto next_irq;
	      }
	  }
      }
    else
      {
	Mword irq = Pic::pending();
	if (EXPECT_FALSE(irq == Pic::No_irq_pending))
	  return;

	if (Thread::check_for_ipi(irq))
	  return;

	if (Pic::is_pending(irq, Config::Scheduling_irq))
	  {
	    Irq::log_timer_irq(irq);
	    Timer::acknowledge();
	    Timer::update_system_clock();
	    current_thread()->handle_timer_interrupt();
	  }
	else
	  {
	    Irq *i = nonull_static_cast<Irq*>(Irq_chip_gen::irqs[irq]);
	    Irq::log_irq(i, irq);
	    i->pin()->hit();
	  }
      }
}


