INTERFACE:

EXTENSION class Kernel_uart { enum { Bsp_init_mode = Init_before_mmu }; };

IMPLEMENTATION [ppc32 && serial]:

#include <boot_info.h>

IMPLEMENT
bool Kernel_uart::startup(unsigned, int /*irq*/)
{
  return Uart::startup(Boot_info::uart_base(), 1);
}
