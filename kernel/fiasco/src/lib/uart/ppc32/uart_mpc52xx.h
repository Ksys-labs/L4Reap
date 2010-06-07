#ifndef L4_CXX_UART_MPC52XX_H__
#define L4_CXX_UART_MPC52XX_H__

#include "uart_base.h"
#include "of1275.h"

namespace L4
{
  class Uart_mpc52xx : public Uart
  {
  private:
    typedef unsigned char  u8;
    typedef unsigned short u16;
    typedef unsigned int   u32;

    /**
     * PSC registers
     */
    //psc + 0x00, size u8
    unsigned long mode() const
    { return _psc_base; }

    //psc + 0x04, size u16
    unsigned long status() const
    { return _psc_base + 0x04; }

    //psc + 0x08, size u8
    unsigned long command() const
    { return _psc_base + 0x08; }

    //psc + 0x0c, size u32
    unsigned long buffer() const
    { return _psc_base + 0x0c; }

    //psc + 0x14, size u16
    unsigned long imr() const
    { return _psc_base + 0x14; }

    unsigned long sicr() const
    { return _psc_base + 0x44; }

    template < typename T >
    void wr(unsigned long addr, T val) const;

    template < typename T > 
    T rd(unsigned long addr) const;

    template < typename T >
    void wr_dirty(unsigned long addr, T val) const;

    template < typename T > 
    T rd_dirty(unsigned long addr) const;

    inline void mpc52xx_out_char(char c) const;
 
    unsigned long _psc_base;
  public:
    Uart_mpc52xx(int rx_irq, int tx_irq) : Uart(rx_irq, tx_irq) {}
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
