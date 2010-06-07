IMPLEMENTATION [ppc32 && serial]:

#include <boot_info.h>

IMPLEMENT
bool Kernel_uart::startup(unsigned, int /*irq*/)
{
  return Uart::startup(Boot_info::uart_base(), 1);
}
