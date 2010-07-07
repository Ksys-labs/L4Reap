/*!
 * \file   support_imx.cc
 * \brief  Support for the i.MX platform
 *
 * \date   2008-02-04
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
#include <l4/drivers/uart_imx.h>

namespace {
class Platform_arm_imx : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
#ifdef PLATFORM_TYPE_imx21
    static L4::Uart_imx21 _uart(0, 0);
    _uart.startup(0x1000A000);
#elif defined(PLATFORM_TYPE_imx51)
    static L4::Uart_imx51 _uart(0, 0);
    _uart.startup(0x73fbc000);
#else
#error Which platform type?
#endif
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_imx);
