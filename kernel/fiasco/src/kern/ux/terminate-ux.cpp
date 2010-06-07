IMPLEMENTATION[ux]:

#include "context.h"
#include "globals.h"

void
terminate(int)
{
  running = 0;
  Context::boost_idle_prio(0);
}

