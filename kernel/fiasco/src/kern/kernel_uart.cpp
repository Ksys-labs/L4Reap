INTERFACE:

class Kernel_uart
{
public:
  Kernel_uart();
  static void enable_rcv_irq();
};

INTERFACE [serial]:

#include "uart.h"
#include "std_macros.h"

/**
 * Glue between kernel and UART driver.
 */
EXTENSION class Kernel_uart : public Uart
{
private:
  /**
   * Prototype for the UART specific startup implementation.
   * @param uart, the instantiation to start.
   * @param port, the com port number.
   */
  bool startup(unsigned port, int irq=-1);
};

//---------------------------------------------------------------------------
IMPLEMENTATION [serial]:

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "irq.h"
#include "irq_chip.h"
#include "irq_pin.h"
#include "uart.h"
#include "cmdline.h"
#include "config.h"
#include "panic.h"

PUBLIC static FIASCO_CONST
Uart *
Kernel_uart::uart()
{
  static Kernel_uart c;
  return &c;
}

IMPLEMENT
Kernel_uart::Kernel_uart()
{
  char const * const cmdline = Cmdline::cmdline();
  char *s;
  bool ok;

  unsigned n = Config::default_console_uart_baudrate;
  Uart::TransferMode m = Uart::MODE_8N1;
  unsigned p = Config::default_console_uart;
  int      i = -1;

  if (  (s = strstr(cmdline, " -comspeed "))
      ||(s = strstr(cmdline, " -comspeed=")))
    {
      if ((n = strtoul(s + 11, 0, 0)) > 115200 || n < 1)
	{
	  puts ("-comspeed > 115200 not supported or invalid (using 115200)!");
	  n = 115200;
	}
    }

  if (  (s = strstr(cmdline, " -comport "))
      ||(s = strstr(cmdline, " -comport=")))
    p = strtoul(s + 10, 0, 0);

  if ((s = strstr(cmdline, " -comirq=")))
    i = strtoul(s + 9, 0, 0);

  if (!(ok = startup(p, i)))
    printf("Comport 0x%04x is not accepted by the uart driver!\n", p);

  if (ok && !change_mode(m, n))
    panic("Somthing is wrong with the baud rate (%d)!\n", n);
}


IMPLEMENT
void
Kernel_uart::enable_rcv_irq()
{
  // we must not allocate the IRQ in the constructor but here 
  // since the constructor is called before Dirq::Dirq() constructor
  static Irq uart_irq;
  if (Irq_chip::hw_chip->alloc(&uart_irq, uart()->irq()))
    {
      uart_irq.alloc((Receiver*)-1);
      uart_irq.pin()->unmask();
      uart()->enable_rcv_irq();
    }
}

//---------------------------------------------------------------------------
IMPLEMENTATION [!serial]: 

IMPLEMENT inline
Kernel_uart::Kernel_uart()
{}

IMPLEMENT inline
void
Kernel_uart::enable_rcv_irq()
{}
