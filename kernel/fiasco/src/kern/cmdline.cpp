
INTERFACE:

#include "initcalls.h"

class Cmdline
{
private:
  static char _cmdline[256];
};

IMPLEMENTATION:

#include <cstdio>

char Cmdline::_cmdline[256];

PUBLIC static FIASCO_INIT
void
Cmdline::init (const char *line)
{
  snprintf (_cmdline, sizeof (_cmdline), "%s", line);
}

PUBLIC static inline
char const *
Cmdline::cmdline()
{
  return _cmdline;
}
