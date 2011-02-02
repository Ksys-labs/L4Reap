// --------------------------------------------------------------------------
INTERFACE [arm && mptimer]:

#include "irq_chip.h"
#include "irq_pin.h"
#include "kmem.h"

EXTENSION class Timer
{
private:
  enum
  {
    Timer_load_reg     = Kmem::Mp_scu_map_base + 0x600 + 0x0,
    Timer_counter_reg  = Kmem::Mp_scu_map_base + 0x600 + 0x4,
    Timer_control_reg  = Kmem::Mp_scu_map_base + 0x600 + 0x8,
    Timer_int_stat_reg = Kmem::Mp_scu_map_base + 0x600 + 0xc,

    Prescaler = 0,

    Timer_control_enable    = 1 << 0,
    Timer_control_reload    = 1 << 1,
    Timer_control_itenable  = 1 << 2,
    Timer_control_prescaler = (Prescaler & 0xff) << 8,

    Timer_int_stat_event   = 1,
  };

  static Irq_base *irq;
};

// --------------------------------------------------------------
IMPLEMENTATION [arm && mptimer]:

#include <cstdio>
#include "config.h"
#include "io.h"
#include "irq_chip.h"
#include "kip.h"

#include "globals.h"

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  Io::write<Mword>(Timer_control_prescaler | Timer_control_reload
                   | Timer_control_enable | Timer_control_itenable,
                   Timer_control_reg);

  Io::write<Mword>(Interval, Timer_load_reg);
  Io::write<Mword>(Interval, Timer_counter_reg);

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

IMPLEMENT inline NEEDS["io.h"]
void Timer::acknowledge()
{
  Io::write<Mword>(Timer_int_stat_event, Timer_int_stat_reg);
  irq->pin()->ack();
}

IMPLEMENT inline
void Timer::enable()
{
  irq->pin()->unmask();
}

IMPLEMENT inline
void Timer::disable()
{
  irq->pin()->mask();
}

IMPLEMENT inline
void
Timer::update_one_shot(Unsigned64 wakeup)
{
  (void)wakeup;
}

IMPLEMENT inline NEEDS["config.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    return 0;
  return Kip::k()->clock;
}
