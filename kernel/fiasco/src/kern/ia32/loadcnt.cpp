IMPLEMENTATION[ia32,amd64]:

#include <cstring>
#include "cmdline.h"
#include "perf_cnt.h"
#include "static_init.h"

class Loadcnt
{
};

STATIC_INITIALIZE_P(Loadcnt, WATCHDOG_INIT);

PUBLIC static
void FIASCO_INIT_CPU
Loadcnt::init()
{
  if (!strstr(Cmdline::cmdline(), " -loadcnt"))
    return;

  Perf_cnt::setup_loadcnt();
}
