IMPLEMENTATION [imx21]:

#include "arm/uart_imx.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_imx21 uart(20, 20);
  return &uart;
}

IMPLEMENTATION [imx35]:

#include "arm/uart_imx.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_imx35 uart(45, 45);
  return &uart;
}

IMPLEMENTATION [imx51]:

#include "arm/uart_imx.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_imx51 uart(31, 31);
  return &uart;
}
