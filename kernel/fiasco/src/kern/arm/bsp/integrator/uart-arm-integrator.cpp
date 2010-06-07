IMPLEMENTATION [integrator]:

#include "arm/uart_pl011.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_pl011 uart(1,1);
  return &uart;
}
