IMPLEMENTATION [s3c2410]:

#include "arm/uart_s3c2410.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_s3c2410 uart(28,28);
  return &uart;
}
