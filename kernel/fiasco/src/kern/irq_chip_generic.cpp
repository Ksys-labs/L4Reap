INTERFACE:

#include "irq_chip.h"

class Irq_chip_gen : public Irq_chip
{
  friend void irq_handler();

private:
  static Irq_base *irqs[];

};


// -------------------------------------------------------------------------
IMPLEMENTATION:

#include "config.h"

Irq_base *Irq_chip_gen::irqs[Config::Max_num_dirqs];

PUBLIC
unsigned
Irq_chip_gen::nr_irqs() const
{ return Config::Max_num_dirqs; }

PUBLIC static inline
Irq_base *
Irq_chip_gen::lookup(unsigned irqn)
{ return irqs[irqn]; }

PUBLIC
bool
Irq_chip_gen::is_free(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  return !irqs[irqn];
}

PUBLIC
Irq_base *
Irq_chip_gen::irq(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return 0;

  return irqs[irqn];
}


PUBLIC
bool
Irq_chip_gen::alloc(Irq_base *irq, unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  if (irqs[irqn])
    return false;

  irqs[irqn] = irq;
  setup(irq, irqn);
  return true;
}


PUBLIC
bool
Irq_chip_gen::free(Irq_base *irq, unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  if (!irq || irq != irqs[irqn])
    return false;

  irqs[irqn] = 0;
  return true;
}

#if 0
PUBLIC
void
Irq_chip_gen::setup(Irq_base *irq, unsigned irqnum)
{
  irq->pin()->replace<Dirq_pic_pin>(irqnum);
}
#endif


PUBLIC
bool
Irq_chip_gen::reserve(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  if (irqs[irqn])
    return false;

  irqs[irqn] = (Irq_base*)1;

  return true;
}


