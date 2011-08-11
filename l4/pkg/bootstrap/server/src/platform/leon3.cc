/**
 * \file   support_sparc_leon3.cc
 * \brief  Support for the Sparc LEON3 platform
 *
 * \date   2010
 * \author Björn Döbel <doebel@os.inf.tu-dresden.de>
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

#include <l4/drivers/uart_leon3.h>

#include "support.h"

namespace {
class Platform_leon3 : public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_leon3 _uart(1,1);
    _uart.startup(0x80000100);
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_leon3);
