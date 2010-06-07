IMPLEMENTATION [arm && pxa && serial]:
#include "mem_layout.h"

IMPLEMENT
bool Kernel_uart::startup(unsigned port, int /*irq*/)
{
  if(port!=3) return false;
  return Uart::startup(Mem_layout::Uart_base, 22);
}
