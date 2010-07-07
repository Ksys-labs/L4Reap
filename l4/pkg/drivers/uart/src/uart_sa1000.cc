/*!
 * \file   uart_sa1000.cc
 * \brief  SA1000 Uart
 *
 * \date   2008-01-02
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *         Alexander Warg <alexander.warg@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "uart_sa1000.h"

namespace L4
{
  enum {
    PAR_NONE = 0x00,
    PAR_EVEN = 0x03,
    PAR_ODD  = 0x01,
    DAT_5    = (unsigned)-1,
    DAT_6    = (unsigned)-1,
    DAT_7    = 0x00,
    DAT_8    = 0x08,
    STOP_1   = 0x00,
    STOP_2   = 0x04,

    MODE_8N1 = PAR_NONE | DAT_8 | STOP_1,
    MODE_7E1 = PAR_EVEN | DAT_7 | STOP_1,

    // these two values are to leave either mode
    // or baud rate unchanged on a call to change_mode
    MODE_NC  = 0x1000000,
    BAUD_NC  = 0x1000000,
  };

  enum {
    UTCR0 = 0x00,
    UTCR1 = 0x04,
    UTCR2 = 0x08,
    UTCR3 = 0x0c,
    UTCR4 = 0x10,
    UTDR  = 0x14,
    UTSR0 = 0x1c,
    UTSR1 = 0x20,


    UTCR0_PE  = 0x01,
    UTCR0_OES = 0x02,
    UTCR0_SBS = 0x04,
    UTCR0_DSS = 0x08,
    UTCR0_SCE = 0x10,
    UTCR0_RCE = 0x20,
    UTCR0_TCE = 0x40,

    UTCR3_RXE = 0x01,
    UTCR3_TXE = 0x02,
    UTCR3_BRK = 0x04,
    UTCR3_RIE = 0x08,
    UTCR3_TIE = 0x10,
    UTCR3_LBM = 0x20,


    UTSR0_TFS = 0x01,
    UTSR0_RFS = 0x02,
    UTSR0_RID = 0x04,
    UTSR0_RBB = 0x08,
    UTSR0_REB = 0x10,
    UTSR0_EIF = 0x20,

    UTSR1_TBY = 0x01,
    UTSR1_RNE = 0x02,
    UTSR1_TNF = 0x04,
   UTSR1_PRE = 0x08,
    UTSR1_FRE = 0x10,
    UTSR1_ROR = 0x20,

    UARTCLK = 3686400,
  };

  unsigned long Uart_sa1000::rd(unsigned long reg) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    return *r;
  }

  void Uart_sa1000::wr(unsigned long reg, unsigned long val) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    *r = val;
  }

  bool Uart_sa1000::startup(unsigned long base)
  {
    _base = base;
    wr(UTSR0, ~0UL); // clear pending status bits
    wr(UTCR3, UTCR3_RXE | UTCR3_TXE); //enable transmitter and receiver
    return true;
  }

  void Uart_sa1000::shutdown()
  {
    wr(UTCR3, 0);
  }

  bool Uart_sa1000::enable_rx_irq(bool /*enable*/)
  {
    return true;
  }
  bool Uart_sa1000::enable_tx_irq(bool /*enable*/) { return false; }
  bool Uart_sa1000::change_mode(Transfer_mode m, Baud_rate baud)
  {
    unsigned old_utcr3, quot;
    //proc_status st;

    if (baud == (Baud_rate)-1)
      return false;
    if (baud != BAUD_NC && (baud>115200 || baud<96))
      return false;
    if (m == (Transfer_mode)-1)
      return false;

    //st = proc_cli_save();
    old_utcr3 = rd(UTCR3);
    wr(UTCR3, (old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE)));
    //proc_sti_restore(st);

    while (rd(UTSR1) & UTSR1_TBY)
      ;

    /* disable all */
    wr(UTCR3, 0);

    /* set parity, data size, and stop bits */
    if(m != MODE_NC)
      wr(UTCR0, m & 0x0ff);

    /* set baud rate */
    if(baud!=BAUD_NC)
      {
	quot = (UARTCLK / (16*baud)) -1;
	wr(UTCR1, (quot & 0xf00) >> 8);
	wr(UTCR2, quot & 0x0ff);
      }

    wr(UTSR0, (unsigned)-1);
    wr(UTCR3, old_utcr3);
    return true;

  }

  int Uart_sa1000::get_char(bool blocking) const
  {
    int ch;
    unsigned long old_utcr3 = rd(UTCR3);
    wr(UTCR3, old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE));

    while (!(rd(UTSR1) & UTSR1_RNE))
      if(!blocking)
	return -1;

    ch = rd(UTDR);
    wr(UTCR3, old_utcr3);
    return ch;

  }

  int Uart_sa1000::char_avail() const
  {
    return !!(rd(UTSR1) & UTSR1_RNE);
  }

  void Uart_sa1000::out_char(char c) const
  {
    // do UTCR3 thing here as well?
    while(!(rd(UTSR1) & UTSR1_TNF))
      ;
    wr(UTDR, c);
  }

  int Uart_sa1000::write(char const *s, unsigned long count) const
  {
    unsigned old_utcr3;
    //proc_status st;
    unsigned i;

    //st = proc_cli_save();
    old_utcr3 = rd(UTCR3);
    wr(UTCR3, (old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE)) | UTCR3_TXE );

    /* transmission */
    for(i = 0; i < count; i++)
      {
	out_char(s[i]);
	if (s[i] == '\n')
	  out_char('\r');
      }

    /* wait till everything is transmitted */
    while (rd(UTSR1) & UTSR1_TBY)
      ;

    wr(UTCR3, old_utcr3);
    //proc_sti_restore(st);
    return count;
  }
};
