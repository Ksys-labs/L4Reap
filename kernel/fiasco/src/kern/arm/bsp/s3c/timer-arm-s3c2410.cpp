INTERFACE [arm && s3c2410]:

#include "kmem.h"
#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  enum {
    TCFG0  = Kmem::Timer_map_base + 0x00,
    TCFG1  = Kmem::Timer_map_base + 0x04,
    TCON   = Kmem::Timer_map_base + 0x08,
    TCNTB0 = Kmem::Timer_map_base + 0x0c,
    TCMPB0 = Kmem::Timer_map_base + 0x10,
    TCNTO0 = Kmem::Timer_map_base + 0x14,
    TCNTB1 = Kmem::Timer_map_base + 0x18,
    TCMPB1 = Kmem::Timer_map_base + 0x1c,
    TCNTO1 = Kmem::Timer_map_base + 0x20,
    TCNTB2 = Kmem::Timer_map_base + 0x24,
    TCMPB2 = Kmem::Timer_map_base + 0x28,
    TCNTO2 = Kmem::Timer_map_base + 0x2c,
    TCNTB3 = Kmem::Timer_map_base + 0x30,
    TCMPB3 = Kmem::Timer_map_base + 0x34,
    TCNTO3 = Kmem::Timer_map_base + 0x38,
    TCNTB4 = Kmem::Timer_map_base + 0x3c,
    TCNTO4 = Kmem::Timer_map_base + 0x40,

  };

  static Irq_base *irq;
};

// -----------------------------------------------------------------------
IMPLEMENTATION [arm && s3c2410]:

#include "config.h"
#include "kip.h"
#include "irq_chip.h"
#include "io.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  Io::write(0, TCFG0); // prescaler config
  Io::write(0, TCFG1); // mux select
  Io::write(33333, TCNTB4); // reload value

  Io::write(5 << 20, TCON); // start + autoreload

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

IMPLEMENT inline
void Timer::acknowledge()
{
  irq->pin()->ack();
  //irq->pin()->unmask();
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


