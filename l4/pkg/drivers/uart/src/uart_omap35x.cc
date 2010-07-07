/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "uart_omap35x.h"

namespace L4
{
  enum {
    DLL_REG  = 0x00,
    RHR_REG  = 0x00,
    THR_REG  = 0x00,
    IER_REG  = 0x04,
    DLH_REG  = 0x04,
    LCD_REG  = 0x08,
    LSR_REG  = 0x14,
    SSR_REG  = 0x44,
    SYSC_REG = 0x54,
    SYSS_REG = 0x58,

    LCD_REG_CHAR_LENGTH_5BIT       = 0 << 0,
    LCD_REG_CHAR_LENGTH_6BIT       = 1 << 0,
    LCD_REG_CHAR_LENGTH_7BIT       = 2 << 0,
    LCD_REG_CHAR_LENGTH_8BIT       = 3 << 0,
    LCD_REG_CHAR_NB_STOP_2         = 1 << 2,
    LCD_REG_CHAR_PARITY_EN         = 1 << 3,
    LCD_REG_CHAR_PARITY_TYPE1_EVEN = 1 << 4,

    LSR_REG_RX_FIFO_E_AVAIL        = 1 << 0,
    LSR_REG_TX_FIFO_E_EMPTY        = 1 << 5,

    SSR_REG_TX_FIFO_FULL           = 1 << 0,

    SYSC_REG_SOFTRESET             = 1 << 1,

    SYSC_REG_RESETDONE             = 1 << 0,
  };


  unsigned long Uart_omap35x::rd(unsigned long reg) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    return *r;
  }

  void Uart_omap35x::wr(unsigned long reg, unsigned long val) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    *r = val;
  }

  bool Uart_omap35x::startup(unsigned long base)
  {
    _base = base;

    // Reset UART
    //wr(SYSC_REG, rd(SYSC_REG) | SYSC_REG_SOFTRESET);
    //while (!(rd(SYSS_REG) & SYSC_REG_RESETDONE))
    //  ;

    return true;
  }

  void Uart_omap35x::shutdown()
  {
  }

  bool Uart_omap35x::enable_rx_irq(bool enable)
  {
    wr(IER_REG, enable ? 1 : 0);
    return true;
  }
  bool Uart_omap35x::enable_tx_irq(bool /*enable*/) { return false; }
  bool Uart_omap35x::change_mode(Transfer_mode, Baud_rate r)
  {
    if (r != 115200)
      return false;

    return true;
  }

  int Uart_omap35x::get_char(bool blocking) const
  {
    while (!char_avail())
      if (!blocking)
	return -1;

    return rd(RHR_REG);
  }

  int Uart_omap35x::char_avail() const
  {
    return rd(LSR_REG) & LSR_REG_RX_FIFO_E_AVAIL;
  }

  void Uart_omap35x::out_char(char c) const
  {
    wr(THR_REG, c);
    while (!(rd(LSR_REG) & LSR_REG_TX_FIFO_E_EMPTY))
      ;
  }

  int Uart_omap35x::write(char const *s, unsigned long count) const
  {
    unsigned long c = count;
    while (c)
      {
	if (*s == 10)
	  out_char(13);
	out_char(*s++);
	--c;
      }
  //  while (rd(UART01x_FR) & UART01x_FR_BUSY)
   //   ;

    return count;
  }

};

