/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "debug.h"

#include <cstdio>
#include <cstdarg>

static unsigned _debug_level = 1;

void set_debug_level(unsigned level)
{
  _debug_level = level;
}

bool dlevel(unsigned level)
{
  return _debug_level >= level;
}

void d_printf(unsigned level, char const *fmt, ...)
{
  if (_debug_level < level)
    return;

  va_list a;
  va_start(a, fmt);
  vprintf(fmt, a);
  va_end(a);
}

