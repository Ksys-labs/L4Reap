IMPLEMENTATION[ia32,ux,amd64]:

#include "cpu.h"
#include "config.h"
#include "globals.h"
#include "kip.h"

IMPLEMENT inline NEEDS ["config.h", "cpu.h", "kip.h"]
void
Timer::init_system_clock()
{
  if (Config::kinfo_timer_uses_rdtsc)
    Kip::k()->clock = Cpu::cpus.cpu(_cpu).time_us();
  else
    Kip::k()->clock = 0;
}

IMPLEMENT inline NEEDS ["config.h", "cpu.h", "globals.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  if (!current_cpu() && Config::kinfo_timer_uses_rdtsc)
    Kip::k()->clock = Cpu::cpus.cpu(_cpu).time_us();

  return Kip::k()->clock;
}

IMPLEMENT inline NEEDS ["config.h", "cpu.h", "globals.h", "kip.h"]
void
Timer::update_system_clock()
{
  if (current_cpu())
    return;

  if (Config::kinfo_timer_uses_rdtsc)
    Kip::k()->clock = Cpu::cpus.cpu(_cpu).time_us();
  else
    Kip::k()->clock += Config::scheduler_granularity;
}
