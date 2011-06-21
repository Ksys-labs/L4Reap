INTERFACE [arm && omap3]:

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

  enum
  {
    INTCPS_SYSCONFIG         = Kmem::Intc_map_base + 0x010,
    INTCPS_SYSSTATUS         = Kmem::Intc_map_base + 0x014,
    INTCPS_CONTROL           = Kmem::Intc_map_base + 0x048,
    INTCPS_TRESHOLD          = Kmem::Intc_map_base + 0x068,
    INTCPS_ITRn_base         = Kmem::Intc_map_base + 0x080,
    INTCPS_MIRn_base         = Kmem::Intc_map_base + 0x084,
    INTCPS_MIR_CLEARn_base   = Kmem::Intc_map_base + 0x088,
    INTCPS_MIR_SETn_base     = Kmem::Intc_map_base + 0x08c,
    INTCPS_ISR_SETn_base     = Kmem::Intc_map_base + 0x090,
    INTCPS_ISR_CLEARn_base   = Kmem::Intc_map_base + 0x094,
    INTCPS_PENDING_IRQn_base = Kmem::Intc_map_base + 0x098,
    INTCPS_ILRm_base         = Kmem::Intc_map_base + 0x100,
  };
};

//-------------------------------------------------------------------
IMPLEMENTATION [arm && omap3]:

#include <cstring>
#include <cstdio>

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_chip_generic.h"
#include "irq_pin.h"
#include "panic.h"
#include "vkey.h"

class Omap3_pin : public Irq_pin
{
public:
  explicit Omap3_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }
};

PUBLIC
void
Omap3_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Omap3_pin::do_mask()
{
  assert (cpu_lock.test());
  Io::write<Mword>(1 << (irq() & 31), Pic::INTCPS_MIR_SETn_base + (irq() & 0xe0));
}

PUBLIC
void
Omap3_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::write<Mword>(1 << (irq() & 31), Pic::INTCPS_MIR_SETn_base + (irq() & 0xe0));
  Io::write<Mword>(1, Pic::INTCPS_CONTROL);
}

PUBLIC
void
Omap3_pin::ack()
{
  Io::write<Mword>(1, Pic::INTCPS_CONTROL);
}


PUBLIC
void
Omap3_pin::do_unmask()
{
  assert (cpu_lock.test());
  Io::write<Mword>(1 << (irq() & 31), Pic::INTCPS_MIR_CLEARn_base + (irq() & 0xe0));
}


PUBLIC
bool
Omap3_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
Omap3_pin::set_cpu(unsigned)
{
}

// ---


class Irq_chip_arm_x : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_arm_x::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < Config::Max_num_dirqs)
    irq->pin()->replace<Omap3_pin>(irqnum);
}


IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_arm_x _ia;
  Irq_chip::hw_chip = &_ia;

  // Reset
  Io::write<Mword>(2, INTCPS_SYSCONFIG);
  while (!Io::read<Mword>(INTCPS_SYSSTATUS))
    ;

  // auto-idle
  Io::write<Mword>(1, INTCPS_SYSCONFIG);

  // disable treshold
  Io::write<Mword>(0xff, INTCPS_TRESHOLD);

  // set priority for each interrupt line, lets take 0x20
  // setting bit0 to 0 means IRQ (1 would mean FIQ)
  for (int m = 0; m < Config::Max_num_dirqs; ++m)
    Io::write<Mword>(0x20 << 2, INTCPS_ILRm_base + (4 * m));

  // mask all interrupts
  for (int n = 0; n < 3; ++n)
    Io::write<Mword>(0xffffffff, INTCPS_MIR_SETn_base + 0x20 * n);
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all( Status /*s*/ )
{}

PUBLIC static inline NEEDS["io.h",<cstdio>]
Unsigned32 Pic::pending()
{
  for (int n = 0; n < (Config::Max_num_dirqs >> 5); ++n)
    {
      unsigned long x = Io::read<Mword>(INTCPS_PENDING_IRQn_base + 0x20 * n);
      for (int i = 0; i < 32; ++i)
	if (x & (1 << i))
	{
	  return i + n * 32;
	}
    }
  return 0;
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{ return irqs == irq; }

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && omap3]:

PUBLIC
char const *
Omap3_pin::pin_type() const
{ return "HW OMAP3 IRQ"; }
