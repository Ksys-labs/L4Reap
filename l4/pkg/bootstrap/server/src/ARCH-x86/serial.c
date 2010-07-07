/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>,
 *          Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <termios.h>
#include <unistd.h>
#include <l4/util/port_io.h>
#include "base_critical.h"
#include "serial.h"

static int ser_io_base;


struct bootstrap_termios {
  unsigned long c_iflag;
  unsigned long c_oflag;
  unsigned long c_cflag;
  unsigned long c_lflag;
  unsigned char c_cc[20];
  long          c_ispeed;
  long          c_ospeed;

} serial_termios =
{
  0,			/* input flags */
  OPOST,		/* output flags */
  CS8,			/* control flags */
  0,			/* local flags */
  {	'D'-64,		/* VEOF */
    _POSIX_VDISABLE,	/* VEOL */
    0,
    'H'-64,		/* VERASE */
    0,
    'U'-64,		/* VKILL */
    0,
    0,
    'C'-64,		/* VINTR */
    '\\'-64,		/* VQUIT */
    'Z'-64,		/* VSUSP */
    0,
    'Q'-64,		/* VSTART */
    'S'-64,		/* VSTOP */
  },
  115200,		/* input speed */
  115200,		/* output speed */
};

void
com_cons_putchar(int ch)
{
  base_critical_enter();

  if (ser_io_base == 0) 
    {
      base_critical_leave();
      return;
    }

  if (serial_termios.c_oflag & OPOST)
    if (ch == '\n')
      com_cons_putchar('\r');

  /* Wait for the transmit buffer to become available.  */
  while (!(l4util_in8(ser_io_base + 5) & 0x20));

  l4util_out8(ch, ser_io_base + 0);

  base_critical_leave();
}

int
com_cons_char_avail(void)
{
  return !!(l4util_in8(ser_io_base + 5) & 0x01);
}

int
com_cons_try_getchar(void)
{
  int ch = -1;

  base_critical_enter();

  if (ser_io_base == 0) 
    {
      base_critical_leave();
      return -1;
    }

  /* character available?  */
  if (com_cons_char_avail()) {
      /* Grab it.  */
      ch = l4util_in8(ser_io_base + 0);
  }

  base_critical_leave();
  return ch;
}

static int
have_serial(unsigned port_base)
{
  unsigned char scratch, scratch2, scratch3;

  scratch = l4util_in8(port_base + 1);
  l4util_out8(0, port_base + 1);

  l4util_iodelay();

  scratch2 = l4util_in8(port_base + 1);
  l4util_out8(0x0f, port_base + 1);

  l4util_iodelay();

  scratch3 = l4util_in8(port_base + 1);
  l4util_out8(scratch, port_base + 1);

  if (scratch2 || scratch3 != 0x0f)
    return 0;

  return 1;
}

int
com_cons_init(int com_port_or_base)
{
  unsigned char dfr;
  unsigned divisor;

  base_critical_enter();

  switch (com_port_or_base)
    {
    case 1: com_port_or_base = 0x3f8; break;
    case 2: com_port_or_base = 0x2f8; break;
    case 3: com_port_or_base = 0x3e8; break;
    case 4: com_port_or_base = 0x2e8; break;
    }

  /* Silently fail if serial port is not available */
  if (!have_serial(com_port_or_base))
    {
      base_critical_leave();
      return 1;
    }

  ser_io_base = com_port_or_base;

  /* Determine what to plug in the data format register.  */
  if (serial_termios.c_cflag & PARENB)
    if (serial_termios.c_cflag & PARODD)
      dfr = 0x08;
    else
      dfr = 0x18;
  else
    dfr = 0x00;
  if (serial_termios.c_cflag & CSTOPB)
    dfr |= 0x04;
  switch (serial_termios.c_cflag & 0x00000300)
    {
    case CS5: dfr |= 0x00; break;
    case CS6: dfr |= 0x01; break;
    case CS7: dfr |= 0x02; break;
    case CS8: dfr |= 0x03; break;
    }

  /* Convert the baud rate into a divisor latch value.  */
  divisor = 115200 / serial_termios.c_ospeed;

  /* Initialize the serial port.  */
  l4util_out8(0x80 | dfr,     ser_io_base + 3);	/* DLAB = 1 */
  l4util_out8(divisor & 0xff, ser_io_base + 0);
  l4util_out8(divisor >> 8,   ser_io_base + 1);
  l4util_out8(0x03 | dfr,     ser_io_base + 3);	/* DLAB = 0, frame = 8N1 */
  l4util_out8(0x00,           ser_io_base + 1);	/* no interrupts enabled */
  l4util_out8(0x0b,           ser_io_base + 4);	/* OUT2, RTS, and DTR enabled */

  l4util_out8(0x41, ser_io_base + 2);  /* 4 byte trigger + on */

  /* Clear all serial interrupts.  */
  l4util_in8(ser_io_base + 6);	/* ID 0: read RS-232 status register */
  l4util_in8(ser_io_base + 2);	/* ID 1: read interrupt identification reg */
  l4util_in8(ser_io_base + 0);	/* ID 2: read receive buffer register */
  l4util_in8(ser_io_base + 5);	/* ID 3: read serialization status reg */

  base_critical_leave();

  return 0;
}
