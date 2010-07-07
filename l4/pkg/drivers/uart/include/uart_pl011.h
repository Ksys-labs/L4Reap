/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef L4_CXX_UART_PL011_H__
#define L4_CXX_UART_PL011_H__

#include "uart_base.h"

namespace L4
{
  class Uart_pl011 : public Uart
  {
  private:
    unsigned long _base;

    inline unsigned long rd(unsigned long reg) const;
    inline void wr(unsigned long reg, unsigned long val) const;

  public:
    Uart_pl011(int rx_irq, int tx_irq)
       : Uart(rx_irq, tx_irq), _base(~0UL) {}
    bool startup(unsigned long base);
    void shutdown();
    bool enable_rx_irq(bool enable = true);
    bool enable_tx_irq(bool enable = true);
    bool change_mode(Transfer_mode m, Baud_rate r);
    int get_char(bool blocking = true) const;
    int char_avail() const;
    inline void out_char(char c) const;
    int write(char const *s, unsigned long count) const;
  };
};

#endif
