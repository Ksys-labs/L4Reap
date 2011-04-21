INTERFACE:

// On ARM the MMIO for the uart is accessible before the MMU is fully up
EXTENSION class Kernel_uart { enum { Bsp_init_mode = Init_before_mmu }; };

IMPLEMENTATION [arm && imx21 && serial]:

#include "mem_layout.h"

IMPLEMENT
bool Kernel_uart::startup(unsigned port, int /*irq*/)
{
  if(port!=3) return false;
  return Uart::startup(Mem_layout::Uart_base, 20);
}

IMPLEMENTATION [arm && imx35 && serial]:

#include "mem_layout.h"

IMPLEMENT
bool Kernel_uart::startup(unsigned port, int /*irq*/)
{
  if(port!=3) return false;
  // uart-1: 45
  // uart-2: 32
  // uart-3: 18
  return Uart::startup(Mem_layout::Uart_base, 45);
}

IMPLEMENTATION [arm && imx51 && serial]:

#include "mem_layout.h"

IMPLEMENT
bool Kernel_uart::startup(unsigned port, int /*irq*/)
{
  if(port!=3) return false;
  return Uart::startup(Mem_layout::Uart_base, 31);
}
