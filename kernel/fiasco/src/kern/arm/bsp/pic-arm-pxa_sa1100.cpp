INTERFACE [arm && pxa]: // -------------------------------------

#include "kmem.h"

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 1,
    No_irq_pending = 0,
  };

  enum {
    ICIP = Kmem::Pic_map_base + 0x000000,
    ICMR = Kmem::Pic_map_base + 0x000004,
    ICLR = Kmem::Pic_map_base + 0x000008,
    ICCR = Kmem::Pic_map_base + 0x000014,
    ICFP = Kmem::Pic_map_base + 0x00000c,
    ICPR = Kmem::Pic_map_base + 0x000010,
  };
};

INTERFACE [arm && sa1100]: // ----------------------------------

#include "kmem.h"

EXTENSION class Pic
{
public:
  enum
  {
    Multi_irq_pending = 1,
    No_irq_pending = 0,
  };

  enum {
    ICIP = Kmem::Pic_map_base + 0x00000,
    ICMR = Kmem::Pic_map_base + 0x00004,
    ICLR = Kmem::Pic_map_base + 0x00008,
    ICCR = Kmem::Pic_map_base + 0x0000c,
    ICFP = Kmem::Pic_map_base + 0x00010,
    ICPR = Kmem::Pic_map_base + 0x00020,
  };
};

// -------------------------------------------------------------
IMPLEMENTATION [arm && (sa1100 || pxa)]:

#include <cstring>
#include <cstdio>

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "irq.h"
#include "irq_pin.h"
#include "irq_chip_generic.h"
#include "io.h"
#include "vkey.h"

class Pxa_sa_pin : public Irq_pin
{
public:
  explicit Pxa_sa_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }
};

class Irq_chip_arm_pxa_sa : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_pxa_sa::setup(Irq_base *irq, unsigned irqnum)
{
  irq->pin()->replace<Pxa_sa_pin>(irqnum);
}

PUBLIC
void
Pxa_sa_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Pxa_sa_pin::do_mask()
{
  assert (cpu_lock.test());
  Io::write(Io::read<Mword>(Pic::ICMR) & ~(1 << irq()), Pic::ICMR);
}

PUBLIC
void
Pxa_sa_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::write(Io::read<Mword>(Pic::ICMR) & ~(1 << irq()), Pic::ICMR);
  // ack is empty
}

PUBLIC
void
Pxa_sa_pin::ack()
{
  // ack is empty
}

PUBLIC
void
Pxa_sa_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Pxa_sa_pin::do_unmask()
{
  Io::write(Io::read<Mword>(Pic::ICMR) | (1 << irq()), Pic::ICMR);
}

PUBLIC
void
Pxa_sa_pin::do_set_mode(unsigned)
{
}

PUBLIC
bool
Pxa_sa_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
Pxa_sa_pin::set_cpu(unsigned)
{
}



IMPLEMENT FIASCO_INIT
void Pic::init()
{
  // only unmasked interrupts wakeup from idle
  Io::write(0x01, ICCR);
  // mask all interrupts
  Io::write(0x00, ICMR);
  // all interrupts are IRQ's (no FIQ)
  Io::write(0x00, ICLR);
}

IMPLEMENT inline NEEDS["io.h"]
Pic::Status Pic::disable_all_save()
{
  Status s;
  s  = Io::read<Mword>(ICMR);
  Io::write(0, ICMR);
  return s;
}

IMPLEMENT inline NEEDS["io.h"]
void Pic::restore_all( Status s )
{
  Io::write(s, ICMR);
}

PUBLIC static inline NEEDS["io.h"]
Unsigned32 Pic::pending()
{
  return Io::read<Unsigned32>(ICIP);
}

PUBLIC static inline NEEDS[Pic::pending]
Mword Pic::is_pending(Mword &irqs, Mword irq)
{
  Mword ret = irqs & (1 << irq);
  irqs &= ~(1 << irq);
  return ret;
}

// -------------------------------------------------------------
IMPLEMENTATION [arm && debug && (sa1100 || pxa)]:

PUBLIC
char const *
Pxa_sa_pin::pin_type() const
{ return "HW PXA/SA IRQ"; }
