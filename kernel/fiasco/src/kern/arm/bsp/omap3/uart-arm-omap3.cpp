IMPLEMENTATION [arm && omap3_evm]: // -------------------------------------

#include "arm/uart_omap35x.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_omap35x uart(72, 72);
  return &uart;
}

IMPLEMENTATION [arm && omap3_beagleboard]: // -----------------------------

#include "arm/uart_omap35x.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_omap35x uart(74, 74);
  return &uart;
}
