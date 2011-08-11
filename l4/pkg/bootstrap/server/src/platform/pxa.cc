/*!
 * \file   support_pxa.cc
 * \brief  Support for the PXA platform
 *
 * \date   2008-01-04
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
#include <l4/drivers/uart_pxa.h>

namespace {
class Platform_arm_pxa : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_pxa _uart(1,1);
    _uart.startup(0x40100000);
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_pxa);
