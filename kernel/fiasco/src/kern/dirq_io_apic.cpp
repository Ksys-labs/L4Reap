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
#include "vkey.h"

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
{ return Io_apic::total_irqs(); }


IMPLEMENT
void
Dirq_io_apic::Chip::setup(Irq_base *irq, unsigned irqnum)
{
  unsigned apic_idx = Io_apic::find_apic(irqnum);
  irqnum -= Io_apic::apic(apic_idx)->gsi_offset();

  //irq->pin()->set_mode(Default_mode);
  if (irq->pin()->get_mode() & Irq::Trigger_level)
    irq->pin()->replace<Pin_io_apic_level>(apic_idx, irqnum);
  else
    irq->pin()->replace<Pin_io_apic_edge>(apic_idx, irqnum);
}

IMPLEMENT
bool
Dirq_io_apic::Chip::alloc(Irq_base *irq, unsigned irqnum)
{
  if (!Dirq_pic_pin::Chip::alloc(irq, irqnum))
    return false;


  unsigned apic_idx = Io_apic::find_apic(irqnum);
  Io_apic *a = Io_apic::apic(apic_idx);
  unsigned lirqn = irqnum - a->gsi_offset();


  Io_apic_entry e = a->read_entry(lirqn);
  e.vector(vector(irqnum));
  a->write_entry(lirqn, e);
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

class Pin_io_apic_level : public Irq_pin
{
public:
  explicit Pin_io_apic_level(unsigned apic, unsigned irq)
  { payload()[0] = apic + (irq << 16); }

  unsigned irq() const { return payload()[0] >> 16; }
  unsigned apic_idx() const { return payload()[0] & 0xffff; }
  Io_apic *apic() const { return Io_apic::apic(apic_idx()); }
  unsigned gsi() const { return apic()->gsi_offset() + irq(); }
};

class Pin_io_apic_edge : public Pin_io_apic_level
{
public:
  explicit Pin_io_apic_edge(unsigned apic, unsigned irq)
  : Pin_io_apic_level(apic, irq) {}
};

PUBLIC
bool
Pin_io_apic_level::check_debug_irq()
{
  return !Vkey::check_(gsi());
}

PUBLIC
void
Pin_io_apic_level::unbind_irq()
{
  do_mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), gsi());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Pin_io_apic_level::disable()
{
  extern char entry_int_apic_ignore[];
  unsigned vector = Dirq_pic_pin::Chip::vector(gsi());
  Dirq_pic_pin::Chip::vfree(Irq_base::self(this), vector);
  Idt::set_entry(vector, Address(&entry_int_apic_ignore), false);
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
  Io_apic_entry e = apic()->read_entry(irq());
  e.polarity(to_io_apic_polarity(mode));
  e.trigger(to_io_apic_trigger(mode));
  apic()->write_entry(irq(), e);
  if (mode & Irq::Trigger_level)
    new (this) Pin_io_apic_level(apic_idx(), irq());
}


PUBLIC void
Pin_io_apic_level::do_mask()
{
  assert (cpu_lock.test());
  apic()->mask(irq());
  apic()->sync();
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
  apic()->mask(irq());
  apic()->sync();
  Apic::irq_ack();
}


PUBLIC void
Pin_io_apic_level::do_unmask()
{
  assert (cpu_lock.test());
  apic()->unmask(irq());
}

PUBLIC void
Pin_io_apic_level::set_cpu(unsigned cpu)
{
  apic()->set_dest(irq(), Cpu::cpus.cpu(cpu).phys_id());
}


PUBLIC void
Pin_io_apic_level::do_set_mode(unsigned mode)
{
  Io_apic_entry e = apic()->read_entry(irq());
  e.polarity(to_io_apic_polarity(mode));
  e.trigger(to_io_apic_trigger(mode));
  apic()->write_entry(irq(), e);
  if (!(mode & Irq::Trigger_level))
    new (this) Pin_io_apic_edge(apic_idx(), irq());
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
