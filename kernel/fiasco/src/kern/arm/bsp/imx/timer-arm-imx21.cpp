// --------------------------------------------------------------------------
INTERFACE [arm && imx21]:

#include "kmem.h"
#include "irq_chip.h"

EXTENSION class Timer
{
private:
  enum {
    TCTL  = Kmem::Timer_map_base + 0x00,
    TPRER = Kmem::Timer_map_base + 0x04,
    TCMP  = Kmem::Timer_map_base + 0x08,
    TCR   = Kmem::Timer_map_base + 0x0c,
    TCN   = Kmem::Timer_map_base + 0x10,
    TSTAT = Kmem::Timer_map_base + 0x14,

    TCTL_TEN                            = 1 << 0,
    TCTL_CLKSOURCE_PERCLK1_TO_PRESCALER = 1 << 1,
    TCTL_CLKSOURCE_32kHz                = 1 << 3,
    TCTL_COMP_EN                        = 1 << 4,
    TCTL_SW_RESET                       = 1 << 15,
  };
private:
  static Irq_base *irq;
};

// ----------------------------------------------------------------------
IMPLEMENTATION [arm && imx21]:

#include "config.h"
#include "kip.h"
#include "irq_chip.h"
#include "irq_pin.h"
#include "io.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  Io::write<Mword>(0, TCTL); // Disable
  Io::write<Mword>(TCTL_SW_RESET, TCTL); // reset timer
  for (int i = 0; i < 10; ++i)
    Io::read<Mword>(TCN); // docu says reset takes 5 cycles

  Io::write<Mword>(TCTL_CLKSOURCE_32kHz | TCTL_COMP_EN, TCTL);
  Io::write<Mword>(0, TPRER);
  Io::write<Mword>(32, TCMP);

  Irq_chip::hw_chip->reserve(Config::Scheduling_irq);

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, Config::Scheduling_irq);
  irq = &ib;

  Io::set<Mword>(TCTL_TEN, TCTL);
}

static inline
Unsigned64
Timer::timer_to_us(Unsigned32 /*cr*/)
{ return 0; }

static inline
Unsigned64
Timer::us_to_timer(Unsigned64 us)
{ (void)us; return 0; }

IMPLEMENT inline NEEDS["io.h"]
void Timer::acknowledge()
{
  Io::write<Mword>(1, TSTAT);
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::enable()
{
  irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::disable()
{
  irq->pin()->mask();
}

IMPLEMENT inline NEEDS["kip.h", "io.h", Timer::timer_to_us, Timer::us_to_timer]
void
Timer::update_one_shot(Unsigned64 /*wakeup*/)
{
}

IMPLEMENT inline NEEDS["config.h", "kip.h", "io.h", Timer::timer_to_us]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    //return Kip::k()->clock + timer_to_us(Io::read<Unsigned32>(OSCR));
    return 0;
  else
    return Kip::k()->clock;
}

