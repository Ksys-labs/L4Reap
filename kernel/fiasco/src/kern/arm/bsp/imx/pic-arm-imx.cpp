// ---------------------------------------------------------------------
INTERFACE [arm && imx]:

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

  enum
  {
    INTCTL      = Kmem::Pic_map_base + 0x00,
    NIMASK      = Kmem::Pic_map_base + 0x04,
    INTENNUM    = Kmem::Pic_map_base + 0x08,
    INTDISNUM   = Kmem::Pic_map_base + 0x0c,
    INTENABLEH  = Kmem::Pic_map_base + 0x10,
    INTENABLEL  = Kmem::Pic_map_base + 0x14,
    INTTYPEH    = Kmem::Pic_map_base + 0x18,
    INTTYPEL    = Kmem::Pic_map_base + 0x1c,
    NIPRIORITY7 = Kmem::Pic_map_base + 0x20,
    NIPRIORITY0 = Kmem::Pic_map_base + 0x3c,
    NIVECSR     = Kmem::Pic_map_base + 0x40,
    FIVECSR     = Kmem::Pic_map_base + 0x44,
    INTSRCH     = Kmem::Pic_map_base + 0x48,
    INTSRCL     = Kmem::Pic_map_base + 0x4c,
    INTFRCH     = Kmem::Pic_map_base + 0x50,
    INTFRCL     = Kmem::Pic_map_base + 0x54,
    NIPNDH      = Kmem::Pic_map_base + 0x58,
    NIPNDL      = Kmem::Pic_map_base + 0x5c,
    FIPNDH      = Kmem::Pic_map_base + 0x60,
    FIPNDL      = Kmem::Pic_map_base + 0x64,


    INTCTL_FIAD  = 1 << 19, // Fast Interrupt Arbiter Rise ARM Level
    INTCTL_NIAD  = 1 << 20, // Normal Interrupt Arbiter Rise ARM Level
    INTCTL_FIDIS = 1 << 21, // Fast Interrupt Disable
    INTCTL_NIDIS = 1 << 22, // Normal Interrupt Disable
  };
};

// ---------------------------------------------------------------------
IMPLEMENTATION [arm && imx]:

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_chip_generic.h"
#include "irq_pin.h"
#include "vkey.h"

#include <cstdio>

class Imx_pin : public Irq_pin
{
public:
  explicit Imx_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }
};

PUBLIC
void
Imx_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Imx_pin::do_mask()
{
  assert (cpu_lock.test());
  Io::write<Mword>(irq(), Pic::INTDISNUM); // disable pin
}

PUBLIC
void
Imx_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::write<Mword>(irq(), Pic::INTDISNUM); // disable pin
  // ack is empty
}

PUBLIC
void
Imx_pin::ack()
{
  // ack is empty
}

PUBLIC
void
Imx_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Imx_pin::do_unmask()
{
  assert (cpu_lock.test());
  Io::write<Mword>(irq(), Pic::INTENNUM);
}


PUBLIC
bool
Imx_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
Imx_pin::set_cpu(unsigned)
{
}

class Irq_chip_arm_x : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_x::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < Config::Max_num_dirqs)
    irq->pin()->replace<Imx_pin>(irqnum);
}

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_arm_x _ia;
  Irq_chip::hw_chip = &_ia;

  Io::write<Mword>(0,    INTCTL);
  Io::write<Mword>(0x10, NIMASK); // Do not disable any normal interrupts

  Io::write<Mword>(0, INTTYPEH); // All interrupts generate normal interrupts
  Io::write<Mword>(0, INTTYPEL);

  // Init interrupt priorities
  for (int i = 0; i < 8; ++i)
    Io::write<Mword>(0x1111, NIPRIORITY7 + (i * 4)); // low addresses start with 7
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{
  Status s = 0;
  return s;
}

IMPLEMENT inline
void Pic::restore_all( Status /*s*/ )
{
}

PUBLIC static inline NEEDS["io.h"]
Unsigned32 Pic::pending()
{
  return Io::read<Mword>(NIVECSR) >> 16;
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{
  return irqs == irq;
}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && imx]:

PUBLIC
char const *
Imx_pin::pin_type() const
{ return "HW i.MX IRQ"; }

