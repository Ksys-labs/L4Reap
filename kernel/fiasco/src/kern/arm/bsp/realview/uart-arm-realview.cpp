IMPLEMENTATION [realview && (!(mpcore || armca9) || realview_pbx)]:

#include "arm/uart_pl011.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_pl011 uart(44, 44);
  return &uart;
}

IMPLEMENTATION [realview && (mpcore || armca9) && !realview_pbx && !realview_vexpress]:

#include "arm/uart_pl011.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_pl011 uart(36, 36);
  return &uart;
}

IMPLEMENTATION [realview && realview_vexpress]:

#include "arm/uart_pl011.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_pl011 uart(37, 37);
  return &uart;
}

