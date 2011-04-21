// --------------------------------------------------------------------------
INTERFACE [arm && imx_epit]:

#include "kmem.h"
#include "irq_chip.h"

EXTENSION class Timer
{
private:
  enum {
    EPITCR   = Kmem::Timer_map_base + 0x00,
    EPITSR   = Kmem::Timer_map_base + 0x04,
    EPITLR   = Kmem::Timer_map_base + 0x08,
    EPITCMPR = Kmem::Timer_map_base + 0x0c,
    EPITCNR  = Kmem::Timer_map_base + 0x10,

    EPITCR_ENABLE                  = 1 << 0, // enable EPIT
    EPITCR_ENMOD                   = 1 << 1, // enable mode
    EPITCR_OCIEN                   = 1 << 2, // output compare irq enable
    EPITCR_RLD                     = 1 << 3, // reload
    EPITCR_SWR                     = 1 << 16, // software reset
    EPITCR_WAITEN                  = 1 << 19, // wait enabled
    EPITCR_CLKSRC_IPG_CLK          = 1 << 24,
    EPITCR_CLKSRC_IPG_CLK_HIGHFREQ = 2 << 24,
    EPITCR_CLKSRC_IPG_CLK_32K      = 3 << 24,
    EPITCR_PRESCALER_SHIFT         = 4,
    EPITCR_PRESCALER_MASK          = ((1 << 12) - 1) << EPITCR_PRESCALER_SHIFT,

    EPITSR_OCIF = 1,
  };
private:
  static Irq_base *irq;
};

// ----------------------------------------------------------------------
IMPLEMENTATION [arm && imx_epit]:

#include "config.h"
#include "kip.h"
#include "irq_chip.h"
#include "irq_pin.h"
#include "io.h"

#include <cstdio>

Irq_base *Timer::irq;

IMPLEMENT
void Timer::init()
{
  Io::write<Mword>(0, EPITCR); // Disable
  Io::write<Mword>(EPITCR_SWR, EPITCR);
  while (Io::read<Mword>(EPITCR) & EPITCR_SWR)
    ;

  Io::write<Mword>(EPITSR_OCIF, EPITSR);

  Io::write<Mword>(EPITCR_CLKSRC_IPG_CLK_32K
                   | (0 << EPITCR_PRESCALER_SHIFT)
		   | EPITCR_WAITEN
		   | EPITCR_RLD
		   | EPITCR_OCIEN
		   | EPITCR_ENMOD,
                   EPITCR);

  Io::write<Mword>(0, EPITCMPR);

  Io::write<Mword>(32, EPITLR);


  Irq_chip::hw_chip->reserve(Config::Scheduling_irq);

  static Irq_base ib;
  Irq_chip::hw_chip->setup(&ib, Config::Scheduling_irq);
  irq = &ib;

  Io::set<Mword>(EPITCR_ENABLE, EPITCR);
}

static inline
Unsigned64
Timer::timer_to_us(Unsigned32 /*cr*/)
{ return 0; }

static inline
Unsigned64
Timer::us_to_timer(Unsigned64 us)
{ (void)us; return 0; }

IMPLEMENT inline NEEDS["io.h"]
void Timer::acknowledge()
{
  Io::write<Mword>(EPITSR_OCIF, EPITSR);
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::enable()
{
  irq->pin()->unmask();
}

IMPLEMENT inline NEEDS["irq_pin.h"]
void Timer::disable()
{
  irq->pin()->mask();
}

IMPLEMENT inline NEEDS["kip.h", "io.h", Timer::timer_to_us, Timer::us_to_timer]
void
Timer::update_one_shot(Unsigned64 /*wakeup*/)
{
}

IMPLEMENT inline NEEDS["config.h", "kip.h", "io.h", Timer::timer_to_us]
Unsigned64
Timer::system_clock()
{
  if (Config::scheduler_one_shot)
    return 0;
  else
    return Kip::k()->clock;
}

