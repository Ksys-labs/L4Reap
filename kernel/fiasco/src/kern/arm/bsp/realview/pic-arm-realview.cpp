INTERFACE [arm && realview]:

#include "gic.h"
#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 0,

    No_irq_pending = 1023,
  };

private:
  static Gic gic[2];
};

//-------------------------------------------------------------------
INTERFACE [arm && realview && (mpcore || armca9)]:

EXTENSION class Pic
{
private:
  enum
  {
    INTMODE_NEW_NO_DDC = 1 << 23,
  };
};

//-------------------------------------------------------------------
IMPLEMENTATION [arm && !(mpcore || armca9)]:

PRIVATE static inline
void Pic::configure_core()
{}

PUBLIC static inline
bool Pic::is_ipi(unsigned)
{ return 0; }

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic]:

#include <cstring>
#include <cstdio>

#include "config.h"
#include "initcalls.h"
#include "irq_chip.h"

Gic Gic_pin::_gic[2];

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && realview && (realview_pb11mp || (realview_eb && (mpcore || (armca9 && mp))))]:

#include "irq_chip_generic.h"

class Irq_chip_arm_rv : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_rv::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < 64)
    irq->pin()->replace<Gic_pin>(0, irqnum);
  else if (irqnum < 96)
    irq->pin()->replace<Gic_pin>(1, irqnum - 32);
}

PRIVATE static inline
void
Pic::init_other_gics()
{
  Gic_pin::_gic[1].init(Kmem::Gic1_cpu_map_base,
                        Kmem::Gic1_dist_map_base);

  static Gic_cascade_irq casc_irq(&Gic_pin::_gic[1], 32);

  Irq_chip::hw_chip->alloc(&casc_irq, 42);

  casc_irq.pin()->replace<Gic_cascade_pin>(0, 42);
  casc_irq.pin()->unmask();
}

PRIVATE static inline
void
Pic::init_ap_other_gics()
{
  Gic_pin::_gic[1].init_ap();
}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && !(realview && (realview_pb11mp || (realview_eb && (mpcore || (armca9 && mp)))))]:

#include "irq_chip_generic.h"

class Irq_chip_arm_rv : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_rv::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < 64)
    irq->pin()->replace<Gic_pin>(0, irqnum);
}

PRIVATE static inline
void
Pic::init_other_gics()
{}

PRIVATE static inline
void
Pic::init_ap_other_gics()
{}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic]:

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  configure_core();

  static Irq_chip_arm_rv _ia;
  Irq_chip::hw_chip = &_ia;

  Gic_pin::_gic[0].init(Kmem::Gic_cpu_map_base, Kmem::Gic_dist_map_base);

  init_other_gics();
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all( Status /*s*/ )
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
IMPLEMENTATION [arm && mp && pic_gic]:

PUBLIC static
void Pic::init_ap()
{
  Gic_pin::_gic[0].init_ap();
  init_ap_other_gics();
}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && (mpcore || armca9)]:

#include "cpu.h"
#include "io.h"
#include "platform.h"

PRIVATE static
void Pic::unlock_config()
{ Io::write<Mword>(0xa05f, Platform::Sys::Lock); }

PRIVATE static
void Pic::lock_config()
{ Io::write<Mword>(0x0, Platform::Sys::Lock); }

PRIVATE static
void Pic::configure_core()
{
  // Enable 'new' interrupt-mode, no DCC
  unlock_config();
  Io::write<Mword>(Io::read<Mword>(Platform::Sys::Pld_ctrl1) | INTMODE_NEW_NO_DDC,
                   Platform::Sys::Pld_ctrl1);
  lock_config();
}
