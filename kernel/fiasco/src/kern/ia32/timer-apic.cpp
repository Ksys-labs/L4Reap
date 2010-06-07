IMPLEMENTATION [apic_timer]:

#include <cstdio>

#include "apic.h"
#include "config.h"
#include "cpu.h"
#include "logdefs.h"
#include "pit.h"
#include "std_macros.h"

IMPLEMENT
void
Timer::init()
{
  Apic::timer_assign_irq_vector(Config::scheduler_irq_vector);

  if (Config::scheduler_one_shot)
    {
      Apic::timer_set_one_shot();
      Apic::timer_reg_write(0xffffffff);
    }
  else
    {
      Apic::timer_set_periodic();
      Apic::timer_reg_write(Apic::us_to_apic(Config::scheduler_granularity));
    }

  // make sure that PIT does pull its interrupt line
  Pit::done();

  if (!Config::scheduler_one_shot)
    // from now we can save energy in getchar()
    Config::getchar_does_hlt_works_ok = Config::hlt_works_ok;

  printf ("Using the Local APIC timer on vector %x (%s Mode) for scheduling\n",
          Config::scheduler_irq_vector,
          Config::scheduler_one_shot ? "One-Shot" : "Periodic");
}

IMPLEMENT inline NEEDS["apic.h"]
void
Timer::acknowledge()
{
  Apic::irq_ack();
}

IMPLEMENT inline NEEDS["apic.h"]
void
Timer::enable()
{
  Apic::timer_enable_irq();
  Apic::irq_ack();
}

IMPLEMENT inline NEEDS["apic.h"]
void
Timer::disable()
{
  Apic::timer_disable_irq();
}

static
void
Timer::update_one_shot(Unsigned64 wakeup)
{
  unsigned cpu = current_cpu();
  Unsigned32 apic;
  Unsigned64 now = Kip::k()->clock; //Cpu::cpus.cpu(cpu).time_us();
  if (EXPECT_FALSE (wakeup <= now))
    // already expired
    apic = 1;
  else
    {
      Unsigned64 delta = wakeup - now;
      if (delta < 1000)
	apic = Apic::us_to_apic(1000);
      else if (delta > 10000)
	apic = Apic::us_to_apic(10000);
      else
        apic = Apic::us_to_apic(delta);

      if (EXPECT_FALSE (apic < 1))
	// timeout too small
	apic = 1;
    }

  Apic::timer_reg_write(apic);
}

IMPLEMENT inline
void
Timer::update_timer(Unsigned64 wakeup)
{
  if (Config::scheduler_one_shot)
    update_one_shot(wakeup);
}
