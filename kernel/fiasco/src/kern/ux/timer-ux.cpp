// ------------------------------------------------------------------------
INTERFACE[ux]:

#include "irq_chip.h"

EXTENSION class Timer
{
private:
  static void bootstrap();

  static Irq_base *irq;
};

// ------------------------------------------------------------------------
IMPLEMENTATION[ux]:

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include "boot_info.h"
#include "initcalls.h"
#include "irq_chip.h"
#include "irq_pin.h"
#include "pic.h"

Irq_base *Timer::irq;

IMPLEMENT FIASCO_INIT_CPU
void
Timer::init()
{
  if (Boot_info::irq0_disabled())
    return;

  if (!Pic::setup_irq_prov (Pic::IRQ_TIMER, Boot_info::irq0_path(), bootstrap))
    {
      puts ("Problems setting up timer interrupt!");
      exit (1);
    }

  // reserve timer IRQ
  Irq_chip *c = Irq_chip::hw_chip;
  c->reserve(Pic::IRQ_TIMER);

  static Irq_base ib;
  c->setup(&ib, Pic::IRQ_TIMER);
  irq = &ib;
}

IMPLEMENT FIASCO_INIT_CPU
void
Timer::bootstrap()
{
  close (Boot_info::fd());
  execl (Boot_info::irq0_path(), "[I](irq0)", NULL);
}

IMPLEMENT inline
void
Timer::acknowledge()
{}

IMPLEMENT inline NEEDS["boot_info.h", "irq_pin.h"]
void
Timer::enable()
{
  if (Boot_info::irq0_disabled())
    return;

  irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::disable()
{
  irq->pin()->mask();
}

IMPLEMENT inline
void
Timer::update_timer(Unsigned64)
{
  // does nothing in periodic mode
}
