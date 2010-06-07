/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/util/util.h>
#include <unistd.h>

int usleep(useconds_t usec)
{
  l4_usleep(usec);
  return 0;
}

