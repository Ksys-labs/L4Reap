// ------------------------------------------------------------------------
IMPLEMENTATION [arm && tegra2]:

#include "kmem.h"
#include "mmio_register_block.h"

void __attribute__ ((noreturn))
platform_reset(void)
{
  enum { RESET = Mem_layout::Clock_reset_phys_base + 0x4 };
  Mmio_register_block b(Kmem::mmio_remap(RESET));
  b.modify<Mword>(4, 0, 0);
  for (;;)
    ;
}
