/* ARM specific */

INTERFACE [arm]:
#include "types.h"

//---------------------------------------------------------------------------
IMPLEMENTATION [arm]:
#include <cstdlib>
#include <cstdio>
#include <initfini.h>
#include "boot_info.h"
#include "terminate.h"

void kernel_main(void);

void console_init();

extern "C"
void __main( )
{
  atexit(&static_destruction);
  static_construction();
  kernel_main();
  terminate(0);
}
