/*
 * Dummy inputlib with no functionality.
 *
 * by Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/input/libinput.h>
#include <l4/sys/err.h>

int l4input_ispending(void)
{
  return 0;
}

int l4input_flush(void *buffer, int count)
{
  (void)buffer; (void)count;
  return 0;
}

int l4input_pcspkr(int tone)
{
  (void)tone;
  return -L4_ENODEV;
}

int l4input_init(int prio, void (*handler)(struct l4input *))
{
  (void)prio; (void)handler;
  return 0;
}
