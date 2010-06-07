/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <stdio.h>
#include <errno.h>

int fcntl(int fd, int cmd);

int fcntl(int fd, int cmd)
{
  (void)fd;
  (void)cmd;
  printf("fcntl() called: unimplemented!\n");
  errno = EINVAL;
  return -EINVAL;
}

