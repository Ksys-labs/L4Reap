INTERFACE [arm && tegra2]:

#include "gic.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 0,
    No_irq_pending = 1023,
  };
};

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && tegra2]:

#include <cstring>
#include <cstdio>

#include "config.h"
#include "initcalls.h"
#include "irq_chip.h"
#include "irq_chip_generic.h"
#include "kmem.h"

Gic Gic_pin::_gic[1];

class Irq_chip_tegra2 : public Irq_chip_gen
{
};

PUBLIC
void Irq_chip_tegra2::setup(Irq_base *irq, unsigned irqnum)
{
  irq->pin()->replace<Gic_pin>(0, irqnum);
}

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_tegra2 _ia;
  Irq_chip::hw_chip = &_ia;

  Gic_pin::_gic[0].init(Kmem::Gic_cpu_map_base, Kmem::Gic_dist_map_base);
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all(Status)
{}

PUBLIC static inline
Unsigned32 Pic::pending()
{
  return Gic_pin::_gic[0].pending();
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{ return irqs == irq; }

//-------------------------------------------------------------------
IMPLEMENTATION [arm && mp && pic_gic && tegra2]:

PUBLIC static
void Pic::init_ap()
{
  Gic_pin::_gic[0].init_ap();
}
