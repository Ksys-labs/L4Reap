/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/log/log.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void LOG_flush(void)
{
  fflush(NULL);
}

void LOG_printf(const char *format, ...)
{
  va_list list;

  va_start(list, format);
  vprintf(format, list);
  va_end(list);
}

void LOG_vprintf(const char *format, va_list list)
{
  vprintf(format, list);
}

void LOG_log(const char *function, const char *format, ...)
{
  va_list list;

  printf("%s(): ", function);
  va_start(list, format);
  vprintf(format, list);
  va_end(list);
  puts("");
}


void LOG_logl(const char *file, int line, const char *function,
              const char *format, ...)
{
  va_list list;
  const char *fileshort;

  if ((fileshort = strstr(file, "pkg/")))
    file = fileshort + 4;


  printf("%s:%d:%s():\n", file, line, function);
  va_start(list, format);
  vprintf(format, list);
  va_end(list);
  puts("");
}
