/**
 * PowerPC timer using internal decrementer
 */

IMPLEMENTATION [ppc32]:

#include "cpu.h"
#include "config.h"
#include "globals.h"
#include "kip.h"
#include "decrementer.h"
#include "warn.h"

#include <cstdio>

IMPLEMENT inline NEEDS ["decrementer.h", "kip.h", "config.h", <cstdio>]
void
Timer::init()
{
  printf("Using PowerPC decrementer for scheduling\n");

  //1000 Hz
  Decrementer::d()->init(Kip::k()->frequency_bus /
                         (4*Config::scheduler_granularity));

}

IMPLEMENT inline NEEDS ["decrementer.h"]
void
Timer::enable()
{
  Decrementer::d()->enable();
}

IMPLEMENT inline NEEDS ["decrementer.h"]
void
Timer::disable()
{
  Decrementer::d()->disable();
}

IMPLEMENT inline NEEDS ["kip.h"]
void
Timer::init_system_clock()
{
  Kip::k()->clock = 0;
}

IMPLEMENT inline NEEDS ["globals.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  return Kip::k()->clock;
}

IMPLEMENT inline NEEDS ["decrementer.h", "config.h", "globals.h", "kip.h"]
void
Timer::update_system_clock()
{
  //not boot cpu
  if(current_cpu())
    return;

  Decrementer::d()->set();
  Kip::k()->clock += Config::scheduler_granularity;
}

