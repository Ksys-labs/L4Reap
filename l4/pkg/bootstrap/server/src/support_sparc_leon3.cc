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

#include "support.h"

#include <l4/drivers/uart_dummy.h>

#define UART_TYPE Uart_dummy

namespace {
class Platform_leon3 //: public Platform_single_region_ram
{
  bool probe() { return true; }

  void init()
  {
	  asm volatile("ta 0\n");
#if 0
    static L4::UART_TYPE _uart(1,1);
    _uart.startup(0x50000000);
    set_stdio_uart(&_uart);
#endif
  }
};
}

REGISTER_PLATFORM(Platform_leon3);
