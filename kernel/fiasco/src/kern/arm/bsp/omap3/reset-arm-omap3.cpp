IMPLEMENTATION [arm && omap3]:

#include "io.h"
#include "kmem.h"

void __attribute__ ((noreturn))
pc_reset(void)
{
  enum
    {
      PRM_RSTCTRL = Kmem::Prm_global_reg_map_base + 0x50,
    };

  Io::write<Mword>(2, PRM_RSTCTRL);

  for (;;)
    ;
}
