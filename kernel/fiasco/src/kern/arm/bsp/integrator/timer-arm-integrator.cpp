// --------------------------------------------------------------------------
INTERFACE [arm && integrator]:

#include "kmem.h"
#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  enum {
    Base = Kmem::Timer_map_base,

    TIMER0_VA_BASE = Base + 0x000,
    TIMER1_VA_BASE = Base + 0x100,
    TIMER2_VA_BASE = Base + 0x200,

    TIMER_LOAD   = 0x00,
    TIMER_VALUE  = 0x04,
    TIMER_CTRL   = 0x08,
    TIMER_INTCLR = 0x0c,

    TIMER_CTRL_IE       = 1 << 5,
    TIMER_CTRL_PERIODIC = 1 << 6,
    TIMER_CTRL_ENABLE   = 1 << 7,
  };

  static Irq_base *irq;
};

// ----------------------------------------------------------------------
IMPLEMENTATION [arm && integrator]:

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
  /* Switch all timers off */
  Io::write(0, TIMER0_VA_BASE + TIMER_CTRL);
  Io::write(0, TIMER1_VA_BASE + TIMER_CTRL);
  Io::write(0, TIMER2_VA_BASE + TIMER_CTRL);

  unsigned timer_ctrl = TIMER_CTRL_ENABLE | TIMER_CTRL_PERIODIC;
  unsigned timer_reload = 1000000 / Config::scheduler_granularity;

  Io::write(timer_reload, TIMER1_VA_BASE + TIMER_LOAD);
  Io::write(timer_reload, TIMER1_VA_BASE + TIMER_VALUE);
  Io::write(timer_ctrl | TIMER_CTRL_IE, TIMER1_VA_BASE + TIMER_CTRL);

  // reserve timer IRQ
  Irq_chip::hw_chip->reserve(Config::Scheduling_irq);

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, Config::Scheduling_irq);
  irq = &ib;
}

static inline
Unsigned64
Timer::timer_to_us(Unsigned32 /*cr*/)
{ return 0; }

static inline
Unsigned64
Timer::us_to_timer(Unsigned64 us)
{ (void)us; return 0; }

IMPLEMENT inline NEEDS["io.h", "irq_pin.h", Timer::timer_to_us, <cstdio>]
void Timer::acknowledge()
{
  Io::write(1, TIMER1_VA_BASE + TIMER_INTCLR);

  if (Config::scheduler_one_shot)
    {
      //Kip::k()->clock += timer_to_us(Io::read<Unsigned32>(OSCR));
    }
  else
    {
      Kip::k()->clock += Config::scheduler_granularity;
    }

  irq->pin()->ack();
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
Timer::update_one_shot(Unsigned64 wakeup)
{
  Unsigned32 apic;
  //Kip::k()->clock += timer_to_us(Io::read<Unsigned32>(OSCR));
  Unsigned64 now = Kip::k()->clock;

  if (EXPECT_FALSE (wakeup <= now) )
    // already expired
    apic = 1;
  else
    {
      apic = us_to_timer(wakeup - now);
      if (EXPECT_FALSE(apic > 0x0ffffffff))
	apic = 0x0ffffffff;
      if (EXPECT_FALSE (apic < 1) )
	// timeout too small
	apic = 1;
    }
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

