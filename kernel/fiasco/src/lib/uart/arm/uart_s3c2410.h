#ifndef L4_CXX_UART_S3C2410_H__
#define L4_CXX_UART_S3C2410_H__

#include "uart_base.h"

namespace L4
{
  class Uart_s3c2410 : public Uart
  {
  private:
    unsigned long _base;

    inline unsigned long rd(unsigned long reg) const;
    inline void wr(unsigned long reg, unsigned long val) const;

  public:
    Uart_s3c2410(int rx_irq, int tx_irq)
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
