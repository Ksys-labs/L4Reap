INTERFACE[ia32,amd64]:

class Console;

class Boot_console
{
public:
  static void init();
  static inline Console * cons();

private:
  static Console *_c;
};

IMPLEMENTATION[ia32,amd64]:

#include <cstring>
#include <cstdio>

#include "cmdline.h"
#include "kernel_console.h"
#include "keyb.h"
#include "mux_console.h"
#include "initcalls.h"
#include "static_init.h"
#include "vga_console.h"
#include "mem_layout.h"


Console *Boot_console::_c;


static Console *vga_console()
{
#if defined(CONFIG_IRQ_SPINNER)
  static Vga_console v(Mem_layout::Adap_vram_cga_beg,80,20,true,true);
#else
  static Vga_console v(Mem_layout::Adap_vram_cga_beg,80,25,true,true);
#endif
  return &v;
}

//STATIC_INITIALIZE_P(Boot_console, BOOT_CONSOLE_INIT_PRIO);

IMPLEMENT FIASCO_INIT
void Boot_console::init()
{
  static Keyb k;
  Kconsole::console()->register_console(&k);

  if (strstr(Cmdline::cmdline(), " -noscreen"))
    return;

  Vga_console *c = (Vga_console*)vga_console();
  if(c->is_working())
    Kconsole::console()->register_console(c);

#if defined(CONFIG_IRQ_SPINNER)
  for (int y = 20; y < 25; ++y)
    for (int x = 0; x < 80; ++x)
      c->printchar(x, y, ' ', 8);
#endif
};

IMPLEMENT inline
Console * Boot_console::cons()
{
  return _c;
}
