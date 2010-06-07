// --------------------------------------------------------------------------
INTERFACE [arm]:

#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  static Irq_base *irq;
};

// --------------------------------------------------------------------------
INTERFACE [arm && sp804]:

#include "kmem.h"

EXTENSION class Timer
{
private:
  enum {
    System_control = Kmem::System_ctrl_map_base,

    Refclk = 0,
    Timclk = 1,

    Timer1_enable = 15,
    Timer2_enable = 17,
    Timer3_enable = 19,
    Timer4_enable = 21,

    Timer_load   = 0x00,
    Timer_value  = 0x04,
    Timer_ctrl   = 0x08,
    Timer_intclr = 0x0c,

    Load_0 = Kmem::Timer0_map_base + Timer_load,
    Load_1 = Kmem::Timer1_map_base + Timer_load,
    Load_2 = Kmem::Timer2_map_base + Timer_load,
    Load_3 = Kmem::Timer3_map_base + Timer_load,

    Value_0 = Kmem::Timer0_map_base + Timer_value,
    Value_1 = Kmem::Timer1_map_base + Timer_value,
    Value_2 = Kmem::Timer2_map_base + Timer_value,
    Value_3 = Kmem::Timer3_map_base + Timer_value,

    Ctrl_0 = Kmem::Timer0_map_base + Timer_ctrl,
    Ctrl_1 = Kmem::Timer1_map_base + Timer_ctrl,
    Ctrl_2 = Kmem::Timer2_map_base + Timer_ctrl,
    Ctrl_3 = Kmem::Timer3_map_base + Timer_ctrl,

    Intclr_0 = Kmem::Timer0_map_base + Timer_intclr,
    Intclr_1 = Kmem::Timer1_map_base + Timer_intclr,
    Intclr_2 = Kmem::Timer2_map_base + Timer_intclr,
    Intclr_3 = Kmem::Timer3_map_base + Timer_intclr,

    Interval = 1000,

    Ctrl_ie        = 1 << 5,
    Ctrl_periodic  = 1 << 6,
    Ctrl_enable    = 1 << 7,
  };
};

// --------------------------------------------------------------------------
INTERFACE [arm && mptimer && !realview_pbx]:

EXTENSION class Timer
{
private:
  enum { Interval = 104999, /* assumed 210MHz */};
};

// --------------------------------------------------------------------------
INTERFACE [arm && mptimer && realview_pbx]:

EXTENSION class Timer
{
private:
  enum { Interval = 49999, };
};

// -----------------------------------------------------------------------
IMPLEMENTATION [arm]:

Irq_base *Timer::irq;

// -----------------------------------------------------------------------
IMPLEMENTATION [arm && sp804]:

#include "config.h"
#include "kip.h"
#include "io.h"

#include <cstdio>

IMPLEMENT
void Timer::init()
{
  Mword v;

  v = Io::read<Mword>(System_control);
  v |= Timclk << Timer1_enable;
  Io::write<Mword>(v, System_control);

  // all timers off
  Io::write<Mword>(0, Ctrl_0);
  Io::write<Mword>(0, Ctrl_1);
  Io::write<Mword>(0, Ctrl_2);
  Io::write<Mword>(0, Ctrl_3);

  Io::write<Mword>(Interval, Load_0);
  Io::write<Mword>(Interval, Value_0);
  Io::write<Mword>(Ctrl_enable | Ctrl_periodic | Ctrl_ie, Ctrl_0);

  if (!current_cpu())
    {
      Irq_chip::hw_chip->reserve(Config::Scheduling_irq);

      static Irq_base ib;
      Irq_chip::hw_chip->setup(&ib, Config::Scheduling_irq);
      irq = &ib;
    }
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
  // XXX: there's a update_system_clock function !?!?!?!
  //if (!Config::scheduler_one_shot)
  //  Kip::k()->clock += Config::scheduler_granularity;

  Io::write<Mword>(0, Intclr_0);
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

IMPLEMENT inline NEEDS["kip.h", "io.h", Timer::timer_to_us, Timer::us_to_timer]
void
Timer::update_one_shot(Unsigned64 wakeup)
{
  Unsigned32 apic;
  //Kip::k()->clock += timer_to_us(Io::read<Unsigned32>(Oscr));
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

IMPLEMENT inline NEEDS["config.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    return 0;
  else
    return Kip::k()->clock;
}
