// ---------------------------------------------------------------------
INTERFACE [arm-integrator]:

#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 1,
    No_irq_pending = 0,
  };

  enum
  {
    IRQ_STATUS       = Kmem::Pic_map_base + 0x00,
    IRQ_ENABLE_SET   = Kmem::Pic_map_base + 0x08,
    IRQ_ENABLE_CLEAR = Kmem::Pic_map_base + 0x0c,

    FIQ_ENABLE_CLEAR = Kmem::Pic_map_base + 0x2c,

    PIC_START = 0,
    PIC_END   = 31,
  };
};

// ---------------------------------------------------------------------
IMPLEMENTATION [arm && integrator]:

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_pin.h" 
#include "irq_chip_generic.h"
#include "vkey.h"

class Integr_pin : public Irq_pin
{
public:
  explicit Integr_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }
};

class Irq_chip_arm_integr : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_integr::setup(Irq_base *irq, unsigned irqnum)
{
  irq->pin()->replace<Integr_pin>(irqnum);
}

PUBLIC
void
Integr_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Integr_pin::do_mask()
{
  assert (cpu_lock.test());
  Io::write(1 << (irq() - Pic::PIC_START), Pic::IRQ_ENABLE_CLEAR);
}

PUBLIC
void
Integr_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::write(1 << (irq() - Pic::PIC_START), Pic::IRQ_ENABLE_CLEAR);
  // ack is empty
}

PUBLIC
void
Integr_pin::ack()
{
  // ack is empty
}
PUBLIC
void
Integr_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Integr_pin::do_unmask()
{
  assert (cpu_lock.test());
  Io::write(1 << (irq() - Pic::PIC_START), Pic::IRQ_ENABLE_SET);
}

PUBLIC
void
Integr_pin::do_set_mode(unsigned)
{
}


PUBLIC
bool
Integr_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
Integr_pin::set_cpu(unsigned)
{
}


IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_arm_integr _ia;
  Irq_chip::hw_chip = &_ia;
  Io::write(0xffffffff, IRQ_ENABLE_CLEAR);
  Io::write(0xffffffff, FIQ_ENABLE_CLEAR);
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
  return Io::read<Mword>(IRQ_STATUS);
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{
  Mword ret = irqs & (1 << irq);
  irqs &= ~(1 << irq);
  return ret;
}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && integrator]:

PUBLIC
char const *
Integr_pin::pin_type() const
{ return "HW Integrator IRQ"; }

