/*!
 * \file   uart_sa1000.h
 * \brief  SA1000 uart header
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
#ifndef __L4_CXX_UART_SA1000_H__
#define __L4_CXX_UART_SA1000_H__

#include "uart_base.h"

namespace L4
{
  class Uart_sa1000 : public Uart
  {
  private:
    unsigned long _base;

    inline unsigned long rd(unsigned long reg) const;
    inline void wr(unsigned long reg, unsigned long val) const;

  public:
    Uart_sa1000(int rx_irq, int tx_irq)
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
