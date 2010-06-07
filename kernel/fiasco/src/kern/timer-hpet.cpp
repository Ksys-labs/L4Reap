INTERFACE [hpet_timer]:

class Irq_base;

EXTENSION class Timer
{
  static Irq_base *irq;
};

IMPLEMENTATION [hpet_timer]:

#include "config.h"
#include "cpu.h"
#include "hpet.h"
#include "irq_chip.h"
#include "irq_pin.h"
#include "logdefs.h"
#include "pit.h"
#include "std_macros.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT
void
Timer::init()
{
  if (!Hpet::init())
    return;

  int hpet_irq = Hpet::int_num();
  if (hpet_irq == 0 && Hpet::int_avail(2))
    hpet_irq = 2;
  Config::scheduler_irq_vector = hpet_irq + 0x20;

  if (Config::scheduler_one_shot)
    {
      // tbd
    }
  else
    {
      // setup hpet for periodic here
    }

  if (!Config::scheduler_one_shot)
    // from now we can save energy in getchar()
    Config::getchar_does_hlt_works_ok = Config::hlt_works_ok;

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, hpet_irq);
  irq = &ib;

  Hpet::enable();
  Hpet::dump();

  printf("Using HPET timer on IRQ %d / vector %x (%s Mode) for scheduling\n",
         hpet_irq, Config::scheduler_irq_vector,
         Config::scheduler_one_shot ? "One-Shot" : "Periodic");
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::acknowledge()
{
  irq->pin()->ack();
}

IMPLEMENT inline NEEDS["hpet.h", "irq_pin.h"]
void
Timer::enable()
{
  irq->pin()->unmask();
  Hpet::enable_timer();
}

IMPLEMENT inline NEEDS["hpet.h", "irq_pin.h"]
void
Timer::disable()
{
  Hpet::disable_timer();
  irq->pin()->mask();
}

static
void
Timer::update_one_shot(Unsigned64 /*wakeup*/)
{
}

IMPLEMENT inline
void
Timer::update_timer(Unsigned64 /*wakeup*/)
{
}
