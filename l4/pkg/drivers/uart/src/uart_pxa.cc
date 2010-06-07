/*!
 * \file   uart_pxa.cc
 * \brief  PXA UART implementation
 *
 * \date   2008-01-04
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *         Alexander Warg <alexander.warg@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "uart_pxa.h"

namespace L4
{
  enum Registers {
    TRB      = 0x00, // Transmit/Receive Buffer  (read/write)
    BRD_LOW  = 0x00, // Baud Rate Divisor LSB if bit 7 of LCR is set  (read/write)
    IER      = 0x04, // Interrupt Enable Register  (read/write)
    BRD_HIGH = 0x04, // Baud Rate Divisor MSB if bit 7 of LCR is set  (read/write)
    IIR      = 0x08, // Interrupt Identification Register  (read only)
    FCR      = 0x08, // 16550 FIFO Control Register  (write only)
    LCR      = 0x0c, // Line Control Register  (read/write)
    MCR      = 0x10, // Modem Control Register  (read/write)
    LSR      = 0x14, // Line Status Register  (read only)
    MSR      = 0x18, // Modem Status Register  (read only)
    SPR      = 0x1c, // Scratch Pad Register  (read/write)
  };

  enum {
    Base_rate     = 921600,
    Base_ier_bits = 1 << 6,
  };

  enum {
    PAR_NONE = 0x00,
    PAR_EVEN = 0x18,
    PAR_ODD  = 0x08,
    DAT_5    = 0x00,
    DAT_6    = 0x01,
    DAT_7    = 0x02,
    DAT_8    = 0x03,
    STOP_1   = 0x00,
    STOP_2   = 0x04,

    MODE_8N1 = PAR_NONE | DAT_8 | STOP_1,
    MODE_7E1 = PAR_EVEN | DAT_7 | STOP_1,

    // these two values are to leave either mode
    // or baud rate unchanged on a call to change_mode
    MODE_NC  = 0x1000000,
    BAUD_NC  = 0x1000000,
  };

  unsigned long Uart_pxa::rd(unsigned long reg) const
  { return *(volatile unsigned long*)(_base + reg); }

  void Uart_pxa::wr(unsigned long reg, unsigned long val) const
  { *(volatile unsigned long*)(_base + reg) = val; }

  bool Uart_pxa::startup(unsigned long base)
  {
    _base = base;

    char scratch, scratch2, scratch3;

    scratch = rd(IER);
    wr(IER, 0);

    scratch2 = rd(IER);
    wr(IER, 0xf);

    scratch3 = rd(IER);
    wr(IER, scratch);

    if (!(scratch2 == 0x00 && scratch3 == 0x0f))
      return false;  // this is not the uart

    //proc_status o = proc_cli_save();
    wr(IER, Base_ier_bits);/* disable all rs-232 interrupts */
    wr(MCR, 0x0b);         /* out2, rts, and dtr enabled */
    wr(FCR, 1);            /* enable fifo */
    wr(FCR, 0x07);         /* clear rcv xmit fifo */
    wr(FCR, 1);            /* enable fifo */
    wr(LCR, 0);            /* clear line control register */

    /* clearall interrupts */
    /*read*/ rd(MSR); /* IRQID 0*/
    /*read*/ rd(IIR); /* IRQID 1*/
    /*read*/ rd(TRB); /* IRQID 2*/
    /*read*/ rd(LSR); /* IRQID 3*/

    while (rd(LSR) & 1/*DATA READY*/)
      /*read*/ rd(TRB);
    //proc_sti_restore(o);

    return true;
  }

  void Uart_pxa::shutdown()
  {
    //proc_status o = proc_cli_save();
    wr(MCR, 6);
    wr(FCR, 0);
    wr(LCR, 0);
    wr(IER, 0);
    //proc_sti_restore(o);
  }

  bool Uart_pxa::enable_rx_irq(bool /*enable*/) { return true; }
  bool Uart_pxa::enable_tx_irq(bool /*enable*/) { return false; }
  bool Uart_pxa::change_mode(Transfer_mode m, Baud_rate r)
  {
    //proc_status o = proc_cli_save();
    unsigned long old_lcr = rd(LCR);
    if(r != BAUD_NC) {
      unsigned short divisor = Base_rate / r;
      wr(LCR, old_lcr | 0x80/*DLAB*/);
      wr(TRB, divisor & 0x0ff );        /* BRD_LOW  */
      wr(IER, (divisor >> 8) & 0x0ff ); /* BRD_HIGH */
      wr(LCR, old_lcr);
    }
    if (m != MODE_NC)
      wr(LCR, m & 0x07f);

    //proc_sti_restore(o);
    return true;
  }

  int Uart_pxa::get_char(bool blocking) const
  {
    char old_ier, ch;

    if (!blocking && !char_avail())
      return -1;

    old_ier = rd(IER);
    wr(IER, old_ier & ~0xf);
    while (!char_avail())
      ;
    ch = rd(TRB);
    wr(IER, old_ier);
    return ch;
  }

  int Uart_pxa::char_avail() const
  {
    return rd(LSR) & 1; // DATA READY
  }

  void Uart_pxa::out_char(char c) const
  {
    // hmm
    write(&c, 1);
  }

  int Uart_pxa::write(char const *s, unsigned long count) const
  {
    /* disable uart irqs */
    char old_ier;
    unsigned i;
    old_ier = rd(IER);
    wr(IER, old_ier & ~0x0f);

    /* transmission */
    for (i = 0; i < count; i++) {
      while (!(rd(LSR) & 0x20 /* THRE */))
	;
      if (s[i] == '\346')
	wr(TRB, '\265');
      else
	wr(TRB, s[i]);
      if (s[i]=='\n') {
	while (!(rd(LSR) & 0x20 /* THRE */))
	  ;
	wr(TRB, '\r');
      }
    }

    /* wait till everything is transmitted */
    while (!(rd(LSR) & 0x40 /* TSRE */))
      ;

    wr(IER, old_ier);
    return 1;
  }

};

