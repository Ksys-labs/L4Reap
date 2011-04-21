INTERFACE[{ia32,amd64}-pit_timer]:

class Irq_base;

EXTENSION class Timer
{
  static Irq_base *irq;
};

IMPLEMENTATION[{ia32,amd64}-pit_timer]:

#include "irq_chip.h"
#include "irq_pin.h"
#include "pit.h"
#include "pic.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT inline int Timer::irq_line() { return 0; }

IMPLEMENT
void
Timer::init()
{
  Irq_chip *c = Irq_chip::hw_chip;
  unsigned in = c->legacy_override(0);
  printf("Using the PIT (i8254) on IRQ %d for scheduling\n", in);

  irq = c->irq(in);
  if (irq)
    panic("Could not find IRQ for PIT timer\n");

  c->reserve(in);

  static Irq_base ib;
  c->setup(&ib, in);
  irq = &ib;

  // set up timer interrupt (~ 1ms)
  Pit::init();

  // from now we can save energy in getchar()
  Config::getchar_does_hlt_works_ok = Config::hlt_works_ok;
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::acknowledge()
{
  irq->pin()->ack();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::enable()
{
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
