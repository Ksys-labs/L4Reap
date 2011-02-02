INTERFACE:

class Irq;

class Vkey
{
public:
  enum Echo_type { Echo_off = 0, Echo_on = 1, Echo_crnl = 2 };
};


// ---------------------------------------------------------------------------
IMPLEMENTATION:

#include "irq.h"

static Irq *vkey_irq;

PUBLIC static
void
Vkey::irq(Irq *i)
{ vkey_irq = i; }

// ---------------------------------------------------------------------------
IMPLEMENTATION [debug && serial && !ux]:

#include "config.h"
#include "cpu.h"
#include "globals.h"
#include "kernel_console.h"
#include "kernel_uart.h"
#include "keycodes.h"

static Vkey::Echo_type vkey_echo;
static char     vkey_buffer[256];
static unsigned vkey_tail, vkey_head;
static Console *uart = Kconsole::console()->find_console(Console::UART);

PUBLIC static
void
Vkey::set_echo(Echo_type echo)
{
  vkey_echo = echo;
}

PUBLIC static
int
Vkey::check_(int irq = -1)
{
  if (!uart)
    return 1;

  int  ret = 0;
  bool hit = false;

  // disable last branch recording, branch trace recording ...
  Cpu::cpus.cpu(current_cpu()).debugctl_disable();

  while(1)
    {
      int c = uart->getchar(false);

      if (irq == Kernel_uart::uart()->irq() && c == -1)
        {
          ret = 1;
          break;
        }

      if (c == -1 || c == KEY_ESC)
        break;

      unsigned nh = (vkey_head + 1) % sizeof(vkey_buffer);
      unsigned oh = vkey_head;
      if (nh != vkey_tail)
        {
          vkey_buffer[vkey_head] = c;
          vkey_head = nh;
        }

      if (oh == vkey_tail)
        hit = true;

      if (vkey_echo == Vkey::Echo_crnl && c == '\r')
        c = '\n';

      if (vkey_echo)
        putchar(c);

      ret = 1;
    }

  if (hit && vkey_irq)
    vkey_irq->hit();

  if(Config::serial_esc == Config::SERIAL_ESC_IRQ)
    Kernel_uart::uart()->enable_rcv_irq();

  // reenable debug stuff (undo debugctl_disable)
  Cpu::cpus.cpu(current_cpu()).debugctl_enable();

  return ret;
}

PUBLIC static
int
Vkey::get()
{
  if (vkey_tail != vkey_head)
    return vkey_buffer[vkey_tail];

  return -1;
}

PUBLIC static
void
Vkey::clear()
{
  if (vkey_tail != vkey_head)
    vkey_tail = (vkey_tail + 1) % sizeof(vkey_buffer);
}

//----------------------------------------------------------------------------
IMPLEMENTATION [!debug || !serial || ux]:

PUBLIC static
void
Vkey::set_echo(Echo_type)
{}

PUBLIC static
void
Vkey::clear()
{}


//----------------------------------------------------------------------------
IMPLEMENTATION [debug && (!serial || ux)]:

#include "kernel_console.h"

PUBLIC static
int
Vkey::get()
{
  return Kconsole::console()->getchar(0);
}

//----------------------------------------------------------------------------
IMPLEMENTATION [!debug && serial]:

#include "kernel_console.h"
#include "kernel_uart.h"

static Console *uart = Kconsole::console()->find_console(Console::UART);

PUBLIC static
int
Vkey::get()
{
  return uart->getchar(false);
}

//----------------------------------------------------------------------------
IMPLEMENTATION[!debug && !serial]:

PUBLIC static
int
Vkey::get()
{ return -1; }

//----------------------------------------------------------------------------
IMPLEMENTATION[!debug || !serial]:

PUBLIC static inline
int
Vkey::check_(int = -1)
{ return 0; }
