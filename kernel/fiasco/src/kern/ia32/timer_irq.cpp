//---------------------------------------------------------------------------
IMPLEMENTATION[ia32 || amd64]:

#include "config.h"
#include "io.h"
#include "globals.h"
#include "kernel_console.h"
#include "kdb_ke.h"
#include "timer.h"
#include "vkey.h"
#include "watchdog.h"


/** Extra version of timer interrupt handler which is used when the jdb is
    active to prevent busy waiting. */
extern "C"
void
thread_timer_interrupt_stop(void)
{
  Timer::acknowledge();
}

/** Slow version of timer interrupt.  Activated on every clock tick.
    Checks if something related to debugging is to do. After returning
    from this function, the real timer interrupt handler is called.
 */
extern "C"
void
thread_timer_interrupt_slow(void)
{
  if (!current_cpu())
    {
      if (Config::esc_hack)
	{
	  // <ESC> timer hack: check if ESC key hit at keyboard
	  if (Io::in8(0x60) == 1)
	    kdb_ke("ESC");
	}

      if (Config::serial_esc != Config::SERIAL_NO_ESC)
	{
	  // Here we have to check for serial characters because the
	  // serial interrupt could have an lower priority than a not
	  // acknowledged interrupt. The regular case is to stop when
	  // receiving the serial interrupt.
	  if(Kconsole::console()->char_avail()==1 && !Vkey::check_())
	    kdb_ke("SERIAL_ESC");
	}

      if (Config::watchdog)
	{
	  // tell doggy that we are alive
	  Watchdog::touch();
	}
    }
}


