/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <sched.h>

int sched_get_priority_max(int policy)
{
  (void)policy;
  return 255;
}

int sched_get_priority_min(int policy)
{
  (void)policy;
  return 1;
}
