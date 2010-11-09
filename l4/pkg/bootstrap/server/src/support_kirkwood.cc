/*!
 * \file   support_kirkwood.cc
 * \brief  Support for the kirkwood platform
 *
 * \date   2010-11
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2010 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "support.h"
#include <l4/drivers/uart_pxa.h>

namespace {
class Platform_arm_kirkwood : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_pxa _uart(1, 1);
    _uart.startup(0xf1012000); // uart0
    //_uart.startup(0xf1012100); // uart1
    _uart.change_mode(0x3, 8500); // TCLK=200000000, Divisor: 108=TCLK/115200/16
    set_stdio_uart(&_uart);


    // SPI init, as there's an interrupt pending when coming from u-boot on
    // the dockstar, so make it go away
    *(unsigned *)0xf1010600 = 0; // disable
    *(unsigned *)0xf1010614 = 0; // mask interrupt
    *(unsigned *)0xf1010610 = 1; // clear interrupt
  }
};
}

REGISTER_PLATFORM(Platform_arm_kirkwood);
