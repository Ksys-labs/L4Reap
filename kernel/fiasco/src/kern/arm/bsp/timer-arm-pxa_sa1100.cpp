// ------------------------------------------------------------------------
INTERFACE [arm && (sa1100 || pxa)]:

#include "kmem.h"
#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  enum {
    Base = Kmem::Timer_map_base,
  };
  // Base = 0x0000000 must be set in ...
  enum {
    OSMR0 = Base + 0x00,
    OSMR1 = Base + 0x04,
    OSMR2 = Base + 0x08,
    OSMR3 = Base + 0x0c,
    OSCR  = Base + 0x10,
    OSSR  = Base + 0x14,
    OWER  = Base + 0x18,
    OIER  = Base + 0x1c,


    Timer_diff = (36864 * Config::scheduler_granularity) / 10000, // 36864MHz*1ms
  };

  static Irq_base *irq;
};


// -------------------------------------------------------------
IMPLEMENTATION [arm && (sa1100 || pxa)]:

#include "config.h"
#include "kip.h"
#include "irq_chip.h"
#include "pic.h"
#include "io.h"

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  Io::write(1,          OIER); // enable OSMR0
  Io::write(0,          OWER); // disable Watchdog
  Io::write<Mword>(Timer_diff, OSMR0);
  Io::write(0,          OSCR); // set timer counter to zero
  Io::write(~0U,        OSSR); // clear all status bits

  Irq_chip::hw_chip->reserve(26);

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, 26);
  irq = &ib;
}

static inline
Unsigned64
Timer::timer_to_us(Unsigned32 cr)
{ return (((Unsigned64)cr) << 14) / 60398; }

static inline
Unsigned64
Timer::us_to_timer(Unsigned64 us)
{ return (us * 60398) >> 14; }

IMPLEMENT inline NEEDS["io.h", "pic.h", Timer::timer_to_us]
void Timer::acknowledge()
{
  if (Config::scheduler_one_shot)
    {
      Kip::k()->clock += timer_to_us(Io::read<Unsigned32>(OSCR));
      //puts("Reset timer");
      Io::write(0, OSCR);
      Io::write(0xffffffff, OSMR0);
    }
  else
    Io::write(0, OSCR);
  Io::write(1, OSSR); // clear all status bits
  enable();
}

IMPLEMENT inline NEEDS["pic.h"]
void Timer::enable()
{
  irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["pic.h"]
void Timer::disable()
{
  irq->pin()->mask();
}

IMPLEMENT inline NEEDS["kip.h", "io.h", Timer::timer_to_us, Timer::us_to_timer]
void
Timer::update_one_shot(Unsigned64 wakeup)
{
  Unsigned32 apic;
  Kip::k()->clock += timer_to_us(Io::read<Unsigned32>(OSCR));
  Io::write(0, OSCR);
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

  //printf("%15lld: Set Timer to %lld [%08x]\n", now, wakeup, apic);

  Io::write(apic, OSMR0);
  Io::write(1, OSSR); // clear all status bits
}

IMPLEMENT inline NEEDS["config.h", "kip.h", "io.h", Timer::timer_to_us]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    return Kip::k()->clock + timer_to_us(Io::read<Unsigned32>(OSCR));
  else
    return Kip::k()->clock;
}
