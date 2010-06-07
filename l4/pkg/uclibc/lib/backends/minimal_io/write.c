/**
 * \file   dietlibc/lib/backends/minimal_io/write.c
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <l4/sys/kdebug.h>

ssize_t write(int fd, const void *buf, size_t count)
{
    // just accept write to stdout and stderr
    if ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO))
    {
        outnstring((const char*)buf, count);
        return count;
    }

    // writes to other fds shall fail fast
    errno = EBADF;
    return -1;
}
