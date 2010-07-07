/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

void com_cons_putchar(int ch);
int  com_cons_try_getchar(void);
int  com_cons_char_avail(void);
int  com_cons_init(int com_port);

EXTERN_C_END

#endif

