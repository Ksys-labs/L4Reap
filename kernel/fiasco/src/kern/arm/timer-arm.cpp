INTERFACE [arm]:

EXTENSION class Timer
{
private:
  static inline void update_one_shot(Unsigned64 wakeup);
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include "config.h"
#include "globals.h"
#include "kip.h"

IMPLEMENT inline NEEDS["kip.h"]
void
Timer::init_system_clock()
{
  Kip::k()->clock = 0;
}

IMPLEMENT inline NEEDS["config.h", "globals.h", "kip.h"]
void
Timer::update_system_clock()
{
  if (!current_cpu())
    Kip::k()->clock += Config::scheduler_granularity;
}

IMPLEMENT inline NEEDS[Timer::update_one_shot, "config.h"]
void
Timer::update_timer(Unsigned64 wakeup)
{
  if (Config::scheduler_one_shot)
    update_one_shot(wakeup);
}
