/*!
 * \file   support_omap3evm.cc
 * \brief  Support for the OMAP3EVM platform
 *
 * \date   2008-06-13
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
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

#include "support.h"
#include <l4/drivers/uart_omap35x.h>

namespace {
class Platform_arm_omap3 : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_omap35x _uart(1, 1);
    _uart.startup(0x4806a000);
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_omap3);
