// ---------------------------------------------------------------------
INTERFACE [arm && imx51]:

#include "gic.h"
#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 0,
    No_irq_pending = 0,
  };
private:
  static Gic gic[1];
};

// ---------------------------------------------------------------------
IMPLEMENTATION [arm && imx51]:

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_chip_generic.h"
#include "irq_pin.h"
#include "vkey.h"

#include <cstdio>

Gic Gic_pin::_gic[1];

class Irq_chip_arm_imx51 : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_imx51::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < Config::Max_num_dirqs)
    irq->pin()->replace<Gic_pin>(0, irqnum);
}

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_arm_imx51 _i;
  Irq_chip::hw_chip = &_i;

  Gic_pin::_gic[0].init(0, Kmem::Pic_map_base);
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{
  return 0;
}

IMPLEMENT inline
void Pic::restore_all( Status /*s*/ )
{
}

PUBLIC static inline NEEDS["io.h"]
Unsigned32 Pic::pending()
{
  return Gic_pin::_gic[0].pending();
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{
  return irqs == irq;
}
