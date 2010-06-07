IMPLEMENTATION[{ia32,amd64}-rtc_timer]:

#include "irq_chip.h"
#include "irq_pin.h"
#include "rtc.h"
#include "pit.h"

#include <cstdio>

IMPLEMENT
void
Timer::init()
{
  Irq_chip *c = Irq_chip::hw_chip;
  unsigned in = c->legacy_override(8);
  printf("Using the RTC on IRQ %d (%sHz) for scheduling\n", in,
#ifdef CONFIG_SLOW_RTC
         "64"
#else
         "1k"
#endif
      );

  // set up timer interrupt (~ 1ms)
  Rtc::init(in);

  // make sure that PIT does pull its interrupt line
  Pit::done();

  // from now we can save energy in getchar()
  Config::getchar_does_hlt_works_ok = Config::hlt_works_ok;
}

IMPLEMENT inline NEEDS["rtc.h","irq_pin.h"]
void
Timer::acknowledge()
{
  // periodic scheduling is triggered by irq 8 connected with RTC
  Rtc::irq->pin()->mask();
  Rtc::ack_reset();
  Rtc::irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::enable()
{
  Rtc::irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void
Timer::disable()
{
  Rtc::irq->pin()->mask();
}

IMPLEMENT inline
void
Timer::update_timer(Unsigned64)
{
  // does nothing in periodic mode
}
