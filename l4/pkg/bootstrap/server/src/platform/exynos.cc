/*!
 * \file
 * \brief  Support for Exynos platforms
 *
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2011-2013 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "support.h"
#include <l4/drivers/uart_s3c2410.h>

#include <cstdio>

namespace {
class Platform_arm_exynos : public Platform_single_region_ram
{
public:
  bool probe() { return true; }

  void init()
  {
    static L4::Uart_s5pv210 _uart;
    unsigned uart_nr = 2;
    unsigned long uart_base = 0x12c00000;
    unsigned long uart_offset = 0x10000;

    static L4::Io_register_block_mmio r(uart_base + uart_nr * uart_offset);
    _uart.startup(&r);
    set_stdio_uart(&_uart);
  }
};
}

REGISTER_PLATFORM(Platform_arm_exynos);
