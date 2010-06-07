#include "uart_s3c2410.h"

namespace L4
{
  enum
  {
    OFFSET = 0x0000,
    ULCON   = OFFSET + 0x0,  // line control register
    UCON    = OFFSET + 0x4,  // control register
    UFCON   = OFFSET + 0x8,  // FIFO control register
    UMCON   = OFFSET + 0xc,  // modem control register
    UTRSTAT = OFFSET + 0x10, // Tx/Rx status register
    UERSTAT = OFFSET + 0x14, // Rx error status register
    UFSTAT  = OFFSET + 0x18, // FIFO status register
    UMSTAT  = OFFSET + 0x1c, // modem status register
    UTXH    = OFFSET + 0x20, // transmit buffer register (little endian, 0x23 for BE)
    URXH    = OFFSET + 0x24, // receive buffer register (little endian, 0x27 for BE)
    UBRDIV  = OFFSET + 0x28, // baud rate divisor register

    ULCON_8N1_MODE = 0x3,

    UCON_MODE = 0x045 | (1 << 7),

    UFSTAT_Rx_COUNT_MASK = 0x00f,
    UFSTAT_Tx_COUNT_MASK = 0x0f0,
    UFSTAT_RxFULL        = 0x100,
    UFSTAT_TxFULL        = 0x200,
  };


  unsigned long Uart_s3c2410::rd(unsigned long reg) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    return *r;
  }

  void Uart_s3c2410::wr(unsigned long reg, unsigned long val) const
  {
    volatile unsigned long *r = (unsigned long*)(_base + reg);
    *r = val;
  }

  bool Uart_s3c2410::startup(unsigned long base)
  {
    _base = base;

    wr(ULCON, ULCON_8N1_MODE);
    wr(UCON,  UCON_MODE);
    wr(UFCON, 1);

    wr(UBRDIV, 0x23);
    return true;
  }

  void Uart_s3c2410::shutdown()
  {
    // more
  }

  bool Uart_s3c2410::enable_rx_irq(bool enable)
  { return true; }

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
      if (!blocking) return -1;

    int c = rd(URXH) & 0xff;
    return c;
  }

  int Uart_s3c2410::char_avail() const
  {
    return rd(UFSTAT) & UFSTAT_Rx_COUNT_MASK;
  }

  void Uart_s3c2410::out_char(char c) const
  {
    while (rd(UFSTAT) & UFSTAT_TxFULL)
      ;
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
    while (rd(UFSTAT) & UFSTAT_Tx_COUNT_MASK)
      ;

    return count;
  }

};

