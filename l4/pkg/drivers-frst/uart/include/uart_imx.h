/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef L4_CXX_UART_imx_H__
#define L4_CXX_UART_imx_H__

#include "uart_base.h"

namespace L4
{
  class Uart_imx : public Uart
  {
  public:
    enum platform_type { Type_imx21, Type_imx35, Type_imx51 };
    Uart_imx(int rx_irq, int tx_irq, enum platform_type type)
       : Uart(rx_irq, tx_irq), _base(~0UL), _type(type) {}
    bool startup(unsigned long base);
    void shutdown();
    bool enable_rx_irq(bool enable = true);
    bool enable_tx_irq(bool enable = true);
    bool change_mode(Transfer_mode m, Baud_rate r);
    int get_char(bool blocking = true) const;
    int char_avail() const;
    inline void out_char(char c) const;
    int write(char const *s, unsigned long count) const;

  private:
    unsigned long _base;
    enum platform_type _type;

    inline unsigned long rd(unsigned long reg) const;
    inline void wr(unsigned long reg, unsigned long val) const;
  };

  class Uart_imx21 : public Uart_imx
  {
  public:
    Uart_imx21(int rx_irq, int tx_irq)
       : Uart_imx(rx_irq, tx_irq, Type_imx21) {}
  };

  class Uart_imx35 : public Uart_imx
  {
  public:
    Uart_imx35(int rx_irq, int tx_irq)
       : Uart_imx(rx_irq, tx_irq, Type_imx35) {}
  };

  class Uart_imx51 : public Uart_imx
  {
  public:
    Uart_imx51(int rx_irq, int tx_irq)
       : Uart_imx(rx_irq, tx_irq, Type_imx51) {}
  };
};

#endif
