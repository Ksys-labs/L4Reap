// --------------------------------------------------------------------------
INTERFACE [arm && kirkwood]:

#include "kmem.h"
#include "irq_chip.h"

EXTENSION class Timer
{
private:
  enum {
    Control_Reg  = Mem_layout::Reset_map_base + 0x20300,
    Reload0_Reg  = Mem_layout::Reset_map_base + 0x20310,
    Timer0_Reg   = Mem_layout::Reset_map_base + 0x20314,
    Reload1_Reg  = Mem_layout::Reset_map_base + 0x20318,
    Timer1_Reg   = Mem_layout::Reset_map_base + 0x2031c,

    Bridge_cause = Mem_layout::Reset_map_base + 0x20110,
    Bridge_mask  = Mem_layout::Reset_map_base + 0x20114,

    Timer0_enable = 1 << 0,
    Timer0_auto   = 1 << 1,

    Timer0_bridge_num = 1 << 1,
    Timer1_bridge_num = 1 << 2,

    Reload_value = 200000,
  };
private:
  static Irq_base *irq;
};

// ----------------------------------------------------------------------
IMPLEMENTATION [arm && kirkwood]:

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
  // Disable timer
  Io::write(0, Control_Reg);

  // Set current timer value and reload value
  Io::write<Mword>(Reload_value, Timer0_Reg);
  Io::write<Mword>(Reload_value, Reload0_Reg);

  Irq_chip::hw_chip->reserve(Config::Scheduling_irq);

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, Config::Scheduling_irq);
  irq = &ib;

  Io::set<Mword>(Timer0_enable | Timer0_auto, Control_Reg);
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
  Io::clear<Unsigned32>(Timer0_bridge_num, Bridge_cause);
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::enable()
{
  Io::set<Unsigned32>(Timer0_bridge_num, Bridge_mask);
  irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::disable()
{
  Io::clear<Unsigned32>(Timer0_bridge_num, Bridge_mask);
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
    return 0;
  return Kip::k()->clock;
}
