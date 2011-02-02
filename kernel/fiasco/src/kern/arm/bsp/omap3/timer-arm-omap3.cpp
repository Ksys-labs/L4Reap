// --------------------------------------------------------------------------
INTERFACE [arm && omap3]:

#include "kmem.h"
#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  enum {
    TIOCP_CFG = Kmem::Timer_base + 0x010, // config
    TISTAT    = Kmem::Timer_base + 0x014, // non-interrupt status
    TISR      = Kmem::Timer_base + 0x018, // pending interrupts
    TIER      = Kmem::Timer_base + 0x01c, // enable/disable of interrupt events
    TWER      = Kmem::Timer_base + 0x020, // wake-up features
    TCLR      = Kmem::Timer_base + 0x024, // optional features
    TCRR      = Kmem::Timer_base + 0x028, // internal counter
    TLDR      = Kmem::Timer_base + 0x02c, // timer load value
    TTGR      = Kmem::Timer_base + 0x030, // trigger reload by writing
    TWPS      = Kmem::Timer_base + 0x034, // write-posted pending
    TMAR      = Kmem::Timer_base + 0x038, // compare value
    TCAR1     = Kmem::Timer_base + 0x03c, // first capture value of the counter
    TCAR2     = Kmem::Timer_base + 0x044, // second capture value of the counter
    TPIR      = Kmem::Timer_base + 0x048, // positive inc, gpt1, 2 and 10 only
    TNIR      = Kmem::Timer_base + 0x04C, // negative inc, gpt1, 2 and 10 only


    CM_CLKSEL_WKUP = Kmem::Wkup_cm_map_base + 0x40,
  };

  static Irq_base *irq;
};

// -----------------------------------------------------------------------
IMPLEMENTATION [arm && omap3]:

#include "config.h"
#include "kip.h"
#include "irq_chip.h"
#include "io.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  // reset
  Io::write<Mword>(1, TIOCP_CFG);
  while (!Io::read<Mword>(TISTAT))
    ;
  // reset done

  // overflow mode
  Io::write<Mword>(0x2, TIER);
  // no wakeup
  Io::write<Mword>(0x0, TWER);

  // select 32768 Hz input to GPTimer1 (timer1 only!)
  Io::write<Mword>(~1 & Io::read<Mword>(CM_CLKSEL_WKUP), CM_CLKSEL_WKUP);

  // program 1000 Hz timer frequency
  Io::write<Mword>(232000, TPIR); // gpt1, gpt2 and gpt10 only
  Io::write<Mword>(-768000, TNIR); // gpt1, gpt2 and gpt10 only
  Io::write<Mword>(0xffffffe0, TCRR);
  Io::write<Mword>(0xffffffe0, TLDR);

  // enable
  Io::write<Mword>(1 | 2, TCLR);

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

IMPLEMENT inline NEEDS["config.h", "io.h"]
void Timer::acknowledge()
{
  Io::write<Mword>(2, TISR);
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

IMPLEMENT inline NEEDS["config.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    return 0;
  else
    return Kip::k()->clock;
}
