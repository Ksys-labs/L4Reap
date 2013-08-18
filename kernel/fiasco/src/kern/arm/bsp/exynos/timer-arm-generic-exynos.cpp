IMPLEMENTATION [arm_generic_timer && exynos]:

#include "timer_mct.h"
#include "mem_layout.h"

PUBLIC static
unsigned Timer::irq()
{
  switch (Gtimer::Type)
    {
    case Generic_timer::Physical: return 29;
    case Generic_timer::Virtual:  return 27;
    };
}

IMPLEMENT
void Timer::bsp_init()
{
  Mct_timer mct(Kmem::mmio_remap(Mem_layout::Mct_phys_base));
  mct.start_free_running();
}
