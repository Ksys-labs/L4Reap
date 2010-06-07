IMPLEMENTATION [imx]:

#include "arm/uart_imx.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_imx uart(20, 20);
  return &uart;
}
