INTERFACE:

#include "dirq_pic_pin.h"
#include "initcalls.h"

class Dirq_io_apic : public Dirq_pic_pin
{
protected:
  class Chip : public Dirq_pic_pin::Chip
  {
  public:
    bool alloc(Irq_base *irq, unsigned irqnum);
    void setup(Irq_base *irq, unsigned irqnum);
    unsigned legacy_override(unsigned irq);
    unsigned nr_irqs() const;
    void disable_irq(unsigned irqnum);
  };
};



IMPLEMENTATION:

#include "apic.h"
#include "io_apic.h"
#include "receiver.h"
#include "idt.h"
#include "irq.h"

enum
{
  Default_mode = Irq::Trigger_edge | Irq::Polarity_high,
  //Default_mode = Irq::Trigger_level | Irq::Polarity_high,
};


IMPLEMENT
unsigned
Dirq_io_apic::Chip::legacy_override(unsigned irq)
{
  return Io_apic::legacy_override(irq);
}



IMPLEMENT
unsigned
Dirq_io_apic::Chip::nr_irqs() const
{ return Io_apic::nr_irqs(); }


IMPLEMENT
void
Dirq_io_apic::Chip::setup(Irq_base *irq, unsigned irqnum)
{
  //irq->pin()->set_mode(Default_mode);
  if (irq->pin()->get_mode() & Irq::Trigger_level)
    irq->pin()->replace<Pin_io_apic_level>(irqnum);
  else
    irq->pin()->replace<Pin_io_apic_edge>(irqnum);
}

IMPLEMENT
bool
Dirq_io_apic::Chip::alloc(Irq_base *irq, unsigned irqnum)
{
  if (!Dirq_pic_pin::Chip::alloc(irq, irqnum))
    return false;

  Io_apic_entry e = Io_apic::apic()->read_entry(irqnum);
  e.vector(vector(irqnum));
  Io_apic::apic()->write_entry(irqnum, e);
  return true;
}

IMPLEMENT
void
Dirq_io_apic::Chip::disable_irq(unsigned vector)
{
  extern char entry_int_apic_ignore[];
  Idt::set_entry(vector, Address(&entry_int_apic_ignore), false);
}

static inline
Mword to_io_apic_trigger(unsigned mode)
{
  return (mode & Irq::Trigger_level)
            ? Io_apic_entry::Level
            : Io_apic_entry::Edge;
}

static inline
Mword to_io_apic_polarity(unsigned mode)
{
  return (mode & Irq::Polarity_low)
             ? Io_apic_entry::Low_active
             : Io_apic_entry::High_active;
}

class Pin_io_apic_level : public Dirq_pic_pin
{
public:
  explicit Pin_io_apic_level(unsigned irq) : Dirq_pic_pin(irq) {}
};

class Pin_io_apic_edge : public Pin_io_apic_level
{
public:
  explicit Pin_io_apic_edge(unsigned irq) : Pin_io_apic_level(irq) {}
};



PUBLIC
void
Pin_io_apic_level::disable()
{
  extern char entry_int_apic_ignore[];
  unsigned vector = this->vector();
  Idt::set_entry(vector, Address(&entry_int_apic_ignore), false);
  disable_vector();
}

PUBLIC void
Pin_io_apic_edge::do_mask_and_ack()
{
  assert (cpu_lock.test());
  Apic::irq_ack();
}


PUBLIC void
Pin_io_apic_edge::do_set_mode(unsigned mode)
{
  Io_apic_entry e = Io_apic::apic()->read_entry(irq());
  e.polarity(to_io_apic_polarity(mode));
  e.trigger(to_io_apic_trigger(mode));
  Io_apic::apic()->write_entry(irq(), e);
  if (mode & Irq::Trigger_level)
    new (this) Pin_io_apic_level(irq());
}


PUBLIC void
Pin_io_apic_level::do_mask()
{
  assert (cpu_lock.test());
  Io_apic::mask(irq());
}


PUBLIC
void
Pin_io_apic_level::ack()
{
  assert (cpu_lock.test());
  Apic::irq_ack();
}



PUBLIC void
Pin_io_apic_level::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io_apic::mask(irq());
  Apic::irq_ack();
}


PUBLIC void
Pin_io_apic_level::do_unmask()
{
  assert (cpu_lock.test());
  Io_apic::unmask(irq());
}

PUBLIC void
Pin_io_apic_level::set_cpu(unsigned cpu)
{
  Io_apic::set_dest(irq(), Cpu::cpus.cpu(cpu).phys_id());
}


PUBLIC void
Pin_io_apic_level::do_set_mode(unsigned mode)
{
  Io_apic_entry e = Io_apic::apic()->read_entry(irq());
  e.polarity(to_io_apic_polarity(mode));
  e.trigger(to_io_apic_trigger(mode));
  Io_apic::apic()->write_entry(irq(), e);
  if (!(mode & Irq::Trigger_level))
    new (this) Pin_io_apic_edge(irq());
}


PUBLIC static FIASCO_INIT
void
Dirq_io_apic::init()
{
  static Chip _ia;
  Irq_chip::hw_chip = &_ia;
}

// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

PUBLIC
char const *
Pin_io_apic_level::pin_type() const
{ return "HW IRQ (IOAPIC level)"; }

PUBLIC
char const *
Pin_io_apic_edge::pin_type() const
{ return "HW IRQ (IOAPIC edge)"; }
