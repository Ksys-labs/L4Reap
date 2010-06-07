INTERFACE [arm && s3c2410]:

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
    SRCPND    = Kmem::Pic_map_base + 0x00,
    INTMODE   = Kmem::Pic_map_base + 0x04,
    INTMSK    = Kmem::Pic_map_base + 0x08,
    PRIORITY  = Kmem::Pic_map_base + 0x0c,
    INTPND    = Kmem::Pic_map_base + 0x10,
    INTOFFSET = Kmem::Pic_map_base + 0x14,
    SUBSRCPND = Kmem::Pic_map_base + 0x18,
    INTSUBMSK = Kmem::Pic_map_base + 0x1c,
  };

  enum
  {
    MAIN_0        = 0,
    MAIN_EINT4_7  = 4,
    MAIN_EINT8_23 = 5,
    MAIN_UART2    = 15,
    MAIN_LCD      = 16,
    MAIN_UART1    = 23,
    MAIN_UART0    = 28,
    MAIN_ADC      = 31,
    MAIN_31       = 31,

    SUB_RXD0 = 0,
    SUB_TXD0 = 1,
    SUB_ERR0 = 2,
    SUB_RXD1 = 3,
    SUB_TXD1 = 4,
    SUB_ERR1 = 5,
    SUB_RXD2 = 6,
    SUB_TXD2 = 7,
    SUB_ERR2 = 8,
    SUB_TC   = 9,
    SUB_ADC  = 10,
  };

  enum // Interrupts
  {
    // EINT4_7
    EINT4  = 32,
    EINT7  = 35,
    // EINT8_23
    EINT8  = 36,
    EINT23 = 51,
    // UART2
    INT_UART2_ERR = 52,
    INT_UART2_RXD = 53,
    INT_UART2_TXD = 54,
    // LCD
    INT_LCD_FRSYN = 55,
    INT_LCD_FICNT = 56,
    // UART1
    INT_UART1_ERR = 57,
    INT_UART1_RXD = 58,
    INT_UART1_TXD = 59,
    // UART0
    INT_UART0_ERR = 60,
    INT_UART0_RXD = 61,
    INT_UART0_TXD = 62,
    // ADC
    INT_ADC = 63,
    INT_TC  = 64,
  };
};

// ---------------------------------------------------------------------
IMPLEMENTATION [arm && s3c2410]:

#include "boot_info.h"
#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_pin.h"
#include "irq_chip_generic.h"
#include "vkey.h"

#include <cstdio>

class S3c_pin : public Irq_pin
{
public:
  explicit S3c_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }
};

PRIVATE static
void
S3c_pin::disable_pin(unsigned irq)
{
  int mainirq;

  switch (irq)
    {
      case Pic::INT_TC:        Io::set<Mword>(1 << Pic::SUB_TC,   Pic::INTSUBMSK); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_ADC:       Io::set<Mword>(1 << Pic::SUB_ADC,  Pic::INTSUBMSK); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_UART0_RXD: Io::set<Mword>(1 << Pic::SUB_RXD0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_TXD: Io::set<Mword>(1 << Pic::SUB_TXD0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_ERR: Io::set<Mword>(1 << Pic::SUB_ERR0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART1_RXD: Io::set<Mword>(1 << Pic::SUB_RXD1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_TXD: Io::set<Mword>(1 << Pic::SUB_TXD1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_ERR: Io::set<Mword>(1 << Pic::SUB_ERR1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART2_RXD: Io::set<Mword>(1 << Pic::SUB_RXD2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_TXD: Io::set<Mword>(1 << Pic::SUB_TXD2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_ERR: Io::set<Mword>(1 << Pic::SUB_ERR2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      default:
         if (irq > 31)
           return; // XXX: need to add other cases
         mainirq = irq;
    };

  Io::set<Mword>(1 << mainirq, Pic::INTMSK);
}

PRIVATE static
void
S3c_pin::enable_pin(unsigned irq)
{
  int mainirq;

  switch (irq)
    {
      case Pic::INT_TC:        Io::clear<Mword>(1 << Pic::SUB_TC,   Pic::INTSUBMSK); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_ADC:       Io::clear<Mword>(1 << Pic::SUB_ADC,  Pic::INTSUBMSK); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_UART0_RXD: Io::clear<Mword>(1 << Pic::SUB_RXD0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_TXD: Io::clear<Mword>(1 << Pic::SUB_TXD0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_ERR: Io::clear<Mword>(1 << Pic::SUB_ERR0, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART1_RXD: Io::clear<Mword>(1 << Pic::SUB_RXD1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_TXD: Io::clear<Mword>(1 << Pic::SUB_TXD1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_ERR: Io::clear<Mword>(1 << Pic::SUB_ERR1, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART2_RXD: Io::clear<Mword>(1 << Pic::SUB_RXD2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_TXD: Io::clear<Mword>(1 << Pic::SUB_TXD2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_ERR: Io::clear<Mword>(1 << Pic::SUB_ERR2, Pic::INTSUBMSK); mainirq = Pic::MAIN_UART2; break;
      default:
         if (irq > 31)
           return; // XXX: need to add other cases
         mainirq = irq;
    };

  Io::clear<Mword>(1 << mainirq, Pic::INTMSK);
}

PRIVATE static
void
S3c_pin::ack_pin(unsigned irq)
{
  int mainirq;

  switch (irq)
    {
      case Pic::INT_TC:        Io::write<Mword>(1 << Pic::SUB_TC,   Pic::SUBSRCPND); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_ADC:       Io::write<Mword>(1 << Pic::SUB_ADC,  Pic::SUBSRCPND); mainirq = Pic::MAIN_ADC;   break;
      case Pic::INT_UART0_RXD: Io::write<Mword>(1 << Pic::SUB_RXD0, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_TXD: Io::write<Mword>(1 << Pic::SUB_TXD0, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART0_ERR: Io::write<Mword>(1 << Pic::SUB_ERR0, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART0; break;
      case Pic::INT_UART1_RXD: Io::write<Mword>(1 << Pic::SUB_RXD1, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_TXD: Io::write<Mword>(1 << Pic::SUB_TXD1, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART1_ERR: Io::write<Mword>(1 << Pic::SUB_ERR1, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART1; break;
      case Pic::INT_UART2_RXD: Io::write<Mword>(1 << Pic::SUB_RXD2, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_TXD: Io::write<Mword>(1 << Pic::SUB_TXD2, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART2; break;
      case Pic::INT_UART2_ERR: Io::write<Mword>(1 << Pic::SUB_ERR2, Pic::SUBSRCPND); mainirq = Pic::MAIN_UART2; break;
      default:
         if (irq > 31)
           return; // XXX: need to add other cases
        mainirq = irq;
    };

  Io::write<Mword>(1 << mainirq, Pic::SRCPND); // only 1s are set to 0
  Io::write<Mword>(1 << mainirq, Pic::INTPND); // clear pending interrupt
}

PUBLIC
void
S3c_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
S3c_pin::do_mask()
{
  assert (cpu_lock.test());
  disable_pin(irq());
}

PUBLIC
void
S3c_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  disable_pin(irq());
  ack_pin(irq());
}

PUBLIC
void
S3c_pin::ack()
{
  ack_pin(irq());
}
PUBLIC
void
S3c_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
S3c_pin::do_unmask()
{
  assert (cpu_lock.test());
  enable_pin(irq());
}


PUBLIC
bool
S3c_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
S3c_pin::set_cpu(unsigned)
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
    irq->pin()->replace<S3c_pin>(irqnum);
}

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_arm_x _ia;
  Irq_chip::hw_chip = &_ia;

  Io::write<Mword>(0xffffffff, INTMSK); // all masked
  Io::write<Mword>(0x7fe, INTSUBMSK);   // all masked
  Io::write<Mword>(0, INTMODE);         // all IRQs, no FIQs
  Io::write<Mword>(Io::read<Mword>(SRCPND), SRCPND); // clear source pending
  Io::write<Mword>(Io::read<Mword>(SUBSRCPND), SUBSRCPND); // clear sub src pnd
  Io::write<Mword>(Io::read<Mword>(INTPND), INTPND); // clear pending interrupt

  printf("IRQ init done\n");
}


IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{
  Status s = 0;
  return s;
}

IMPLEMENT inline
void Pic::restore_all( Status /*s*/ )
{ }

PUBLIC static inline NEEDS["io.h"]
Unsigned32 Pic::pending()
{
  int mainirq = Io::read<Mword>(INTOFFSET);

  switch (mainirq)
    {
    case MAIN_ADC:
	{
	  int subirq = Io::read<Mword>(SUBSRCPND);
	  if ((1 << SUB_ADC) & subirq)
	    return INT_ADC;
	  else if ((1 << SUB_TC) & subirq)
	    return INT_TC;
	}
      break;
    // more: tbd
    default:
      return mainirq;
    }
  return No_irq_pending;
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{ return irqs == irq; }

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && s3c2410]:

PUBLIC
char const *
S3c_pin::pin_type() const
{ return "HW S3C2410 IRQ"; }

