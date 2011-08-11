/*!
 * \file   support_beagleboard.cc
 * \brief  Support for the Beagleboard
 *
 * \date   2009-08
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "support.h"
#include <l4/drivers/uart_omap35x.h>

namespace {
class Platform_arm_omap : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_omap35x _uart(1, 1);
#ifdef PLATFORM_TYPE_beagleboard
    _uart.startup(0x49020000);
#elif defined(PLATFORM_TYPE_omap3evm)
    _uart.startup(0x4806a000);
#elif defined(PLATFORM_TYPE_pandaboard)
    _uart.startup(0x48020000);
#else
#error Unknown platform
#endif
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_omap);
