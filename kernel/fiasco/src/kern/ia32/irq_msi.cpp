IMPLEMENTATION:

#include "idt.h"
#include "irq_pin.h"
#include "irq_chip.h"
#include "irq.h"

#include "dirq_pic_pin.h"
#include "apic.h"
#include "static_init.h"

class Irq_pin_msi : public Irq_pin
{
public:
  enum
  {
    Vector_offs = 0x50,
  };

  explicit Irq_pin_msi(unsigned vect) { payload()[0] = vect; }
  unsigned vector() const { return payload()[0]; }
};


class Irq_chip_msi : public Dirq_pic_pin::Chip
{
public:
  void disable_irq(unsigned irqnum);
};

PUBLIC
unsigned
Irq_chip_msi::nr_irqs() const
{ return APIC_IRQ_BASE - 0x10 - Irq_pin_msi::Vector_offs; }

PUBLIC
Mword
Irq_chip_msi::msg(unsigned irqn)
{ return irqn + Irq_pin_msi::Vector_offs; }

PUBLIC
void
Irq_chip_msi::setup(Irq_base *irq, unsigned irqnum)
{
  unsigned v = irqnum + Irq_pin_msi::Vector_offs;
  if (v >= APIC_IRQ_BASE - 0x10)
    return;

  irq->pin()->replace<Irq_pin_msi>(v);
}

PUBLIC
Irq_base *
Irq_chip_msi::irq(unsigned irqnum)
{
  unsigned v = irqnum + Irq_pin_msi::Vector_offs;
  if (v >= APIC_IRQ_BASE - 0x10)
    return 0;

  return virq(v);
}

IMPLEMENT
void
Irq_chip_msi::disable_irq(unsigned vector)
{
  extern char entry_int_apic_ignore[];
  Idt::set_entry(vector, Address(&entry_int_apic_ignore), false);
}

PUBLIC
bool
Irq_chip_msi::alloc(Irq_base *irq, unsigned irqnum)
{
  unsigned v = irqnum + Irq_pin_msi::Vector_offs;
  if (!valloc(irq, v))
    return false;

  setup(irq, irqnum);
  return true;
}

PUBLIC
bool
Irq_chip_msi::free(Irq_base *irq, unsigned irqnum)
{
  return vfree(irq, irqnum + Irq_pin_msi::Vector_offs);
}


PUBLIC static FIASCO_INIT
void
Irq_chip_msi::init()
{
  static Irq_chip_msi _ia;
  Irq_chip::hw_chip_msi = &_ia;
  for (unsigned i = 0; i < _ia.nr_irqs(); ++i)
    _ia.disable_irq(i + Irq_pin_msi::Vector_offs);
}

STATIC_INITIALIZE(Irq_chip_msi);


PUBLIC
void
Irq_pin_msi::do_mask()
{}

PUBLIC
void
Irq_pin_msi::do_unmask()
{}

PUBLIC
void
Irq_pin_msi::do_mask_and_ack()
{
  Apic::irq_ack();
}

PUBLIC
void
Irq_pin_msi::do_set_mode(unsigned)
{}

PUBLIC
void
Irq_pin_msi::ack()
{
  Apic::irq_ack();
}

PUBLIC
void
Irq_pin_msi::set_cpu(unsigned)
{}

PUBLIC
void
Irq_pin_msi::unbind_irq()
{
  Irq_chip::hw_chip_msi->free(Irq::self(this), vector() - Vector_offs);
  replace<Sw_irq_pin>();
}

//--------------------------------------------------------------------------
IMPLEMENTATION [debug]:

PUBLIC
char const *
Irq_pin_msi::pin_type() const
{ return "HW IRQ (MSI)"; }


