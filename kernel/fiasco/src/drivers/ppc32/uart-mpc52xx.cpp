IMPLEMENTATION[uart_mpc52xx && libuart && debug]:

#include "ppc32/uart_mpc52xx.h"
#include "pic.h"


IMPLEMENT L4::Uart *Uart::uart()
{
  static int irq = Pic::get_irq_num((char*)"serial", (char*)"serial");
  static L4::Uart_mpc52xx uart(irq, irq);
  return &uart;
}

IMPLEMENTATION[uart_mpc52xx && libuart && !debug]:

#include "ppc32/uart_mpc52xx.h"

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_mpc52xx uart(-1, -1);
  return &uart;
}
