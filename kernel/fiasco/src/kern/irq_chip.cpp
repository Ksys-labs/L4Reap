INTERFACE:

#include "types.h"

class Irq_base;

class Irq_chip
{
public:
  virtual bool reserve(unsigned irq) = 0;
  virtual bool is_free(unsigned irq) = 0;
  virtual bool alloc(Irq_base *irq, unsigned irqnum) = 0;
  virtual void setup(Irq_base *irq, unsigned irqnum) = 0;
  virtual bool free(Irq_base *irq, unsigned irqnum) = 0;
  virtual Irq_base *irq(unsigned irqnum) = 0;
  virtual void reset(unsigned /*irqnum*/) {}
  virtual unsigned legacy_override(unsigned i) { return i; }

  virtual unsigned nr_irqs() const = 0;
  virtual Mword msg(unsigned /*irqn*/) { return 0; }

public:
  static Irq_chip *hw_chip;
  static Irq_chip *hw_chip_msi;
};

//--------------------------------------------------------------------------
IMPLEMENTATION:

Irq_chip *Irq_chip::hw_chip;
Irq_chip *Irq_chip::hw_chip_msi;


