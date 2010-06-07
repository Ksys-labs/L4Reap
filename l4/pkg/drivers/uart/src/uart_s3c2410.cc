/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "uart_s3c2410.h"

namespace L4
{
  enum {
    ULCON   = 0x0,  // line control register
    UCON    = 0x4,  // control register
    UFCON   = 0x8,  // FIFO control register
    UMCON   = 0xc,  // modem control register
    UTRSTAT = 0x10, // Tx/Rx status register
    UERSTAT = 0x14, // Rx error status register
    UFSTAT  = 0x18, // FIFO status register
    UMSTAT  = 0x1c, // modem status register
    UTXH    = 0x20, // transmit buffer register (little endian, 0x23 for BE)
    URXH    = 0x24, // receive buffer register (little endian, 0x27 for BE)
    UBRDIV  = 0x28, // baud rate divisor register

    ULCON_8N1_MODE = 0x3,

    UCON_MODE = 0x245,

    UFSTAT_Rx_COUNT_MASK = 0x00f,
    UFSTAT_Tx_COUNT_MASK = 0x0f0,
    UFSTAT_RxFULL        = 0x100,
    UFSTAT_TxFULL        = 0x200,

    UMCON_AFC = 1 << 4,

    UTRSTAT_Rx_RDY = 1 << 0,
    UTRSTAT_Tx_RDY = 1 << 1,
  };


  unsigned long Uart_s3c2410::rd(unsigned long reg) const
  {
    return *(volatile unsigned long*)(_base + reg);
  }

  void Uart_s3c2410::wr(unsigned long reg, unsigned long val) const
  {
    *(volatile unsigned long *)(_base + reg) = val;
  }

  void Uart_s3c2410::fifo_reset()
  {
    wr(UFCON, 7); // enable + fifo reset
  }

  bool Uart_s3c2410::startup(unsigned long base)
  {
    _base = base;

    fifo_reset();
    wr(UMCON, 0);

    wr(ULCON, ULCON_8N1_MODE);
    wr(UCON,  UCON_MODE);

    wr(UBRDIV, 0x23);
    for (int i=0; i < 1000; ++i)
      ;

    return true;
  }

  void Uart_s3c2410::shutdown()
  {
    // more
  }

  bool Uart_s3c2410::enable_rx_irq(bool /*enable*/)
  {
    return true;
  }

  bool Uart_s3c2410::enable_tx_irq(bool /*enable*/) { return false; }

  bool Uart_s3c2410::change_mode(Transfer_mode, Baud_rate r)
  {
    if (r != 115200)
      return false;

    wr(ULCON, ULCON_8N1_MODE);
    wr(UCON,  UCON_MODE);
    wr(UFCON, 1);

    wr(UBRDIV, 0x23);

    return true;
  }

  int Uart_s3c2410::get_char(bool blocking) const
  {
    while (!char_avail())
      if (!blocking)
	return -1;

    int uer = rd(UERSTAT);
    int d = rd(URXH);

    if (uer & 1)
      d = '@';
    else if (uer & 4)
      d = '$';

    return d;
  }

  int Uart_s3c2410::char_avail() const
  {
    //return rd(UTRSTAT) & UTRSTAT_Rx_RDY;
    //return rd(UFSTAT) & UFSTAT_Rx_COUNT_MASK;
    return rd(UFSTAT) & (UFSTAT_Rx_COUNT_MASK | UFSTAT_RxFULL);
  }

  void Uart_s3c2410::out_char(char c) const
  {
    //while (!(rd(UTRSTAT) & UTRSTAT_Tx_RDY))
    while (rd(UFSTAT) & UFSTAT_TxFULL)
      ;
    //while (!(rd(UMSTAT) & 0x1))
    //  ;
    wr(UTXH, c);
  }

  int Uart_s3c2410::write(char const *s, unsigned long count) const
  {
    unsigned long c = count;
    while (c)
      {
	if (*s == 10)
	  out_char(13);
	out_char(*s++);
	--c;
      }
    while (!(rd(UTRSTAT) & UTRSTAT_Tx_RDY))
    //while (rd(UFSTAT) & UFSTAT_Tx_COUNT_MASK)
      ;

    return count;
  }

  void Uart_s3c2410::auto_flow_control(bool on)
  {
    wr(UMCON, (rd(UMCON) & ~UMCON_AFC) | (on ? UMCON_AFC : 0));
  }

};

