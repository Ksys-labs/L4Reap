INTERFACE:

#include "mux_console.h"
#include "std_macros.h"

class Kconsole : public Mux_console
{
public:
  int  getchar( bool blocking = true );
  void getchar_chance();

  static Mux_console *console() FIASCO_CONST;

private:
  static bool initialized;
};

IMPLEMENTATION:

#include "config.h"
#include "console.h"
#include "mux_console.h"
#include "processor.h"


IMPLEMENT
int Kconsole::getchar( bool blocking )
{
  if (!blocking)
    return Mux_console::getchar(false);

  while(1)
    {
      int c;
      if ((c = Mux_console::getchar(false)) != -1)
	return c;

      if (Config::getchar_does_hlt &&		// does work in principle
	  Config::getchar_does_hlt_works_ok &&	// wakeup timer is enabled
	  Proc::interrupts())			// does'nt work without ints
	Proc::halt();
      else
	Proc::pause();
    }
}



bool Kconsole::initialized;

PUBLIC static
void
Kconsole::activate()
{
  if (!initialized)
    {
      initialized = true;
      Console::stdout = console();
      Console::stderr = Console::stdout;
      Console::stdin  = Console::stdout;
    }
}

PUBLIC
virtual bool
Kconsole::register_console( Console *c, int pos = 0)
{
  bool b = Mux_console::register_console(c, pos);
  if (b) 
    activate();
  
  return b;
}

IMPLEMENT 
Mux_console *Kconsole::console()
{
  static Kconsole cons;
  return &cons;
}

