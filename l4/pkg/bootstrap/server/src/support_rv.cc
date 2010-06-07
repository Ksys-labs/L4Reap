/*!
 * \file   support_rv.cc
 * \brief  Support for the rv platform
 *
 * \date   2008-01-02
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "support.h"

#include <l4/drivers/uart_pl011.h>

namespace {
class Platform_arm_rv : public Platform_single_region_ram
{
  bool probe() { return true; }
  void init()
  {
    static L4::Uart_pl011 _uart(36,36);
    _uart.startup(0x10009000);
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_rv);
