INTERFACE:

#include "irq_pin.h"
#include "initcalls.h"

class Irq_base;

class Dirq_pic_pin : public Irq_pin
{
public:
  explicit Dirq_pic_pin(unsigned irq) { payload()[0] = irq; }
  unsigned irq() const { return payload()[0]; }

  static void init() FIASCO_INIT;
};

//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "irq.h"
#include "pic.h"
#include "vkey.h"
#if 0
PRIVATE static
bool
Dirq_pic_pin::setup_hw_pin(Irq_base *irq, unsigned irqnum)
{
  if (irqnum >= Pic::nr_irqs())
    return false;

  new (irq->pin()) Dirq_pic_pin(irqnum);
  return true;
}
#endif

PUBLIC
void
Dirq_pic_pin::unbind_irq()
{
  do_mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Dirq_pic_pin::do_mask()
{
  assert (cpu_lock.test());
  Pic::disable_locked(irq());
}


PUBLIC
void
Dirq_pic_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Pic::disable_locked(irq());
  Pic::acknowledge_locked(irq());
}

PUBLIC
void
Dirq_pic_pin::ack()
{
  Pic::acknowledge_locked(irq());
}

PUBLIC
void
Dirq_pic_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Dirq_pic_pin::do_set_mode(unsigned)
{}

PUBLIC
void
Dirq_pic_pin::do_unmask()
{
  assert (cpu_lock.test());
  Pic::enable_locked(irq(), 0xa); //prio);
#if 0
  unsigned long prio;

  if (EXPECT_FALSE(!Irq::self(this)->owner()))
    return;
  if (Irq::self(this)->owner() == (Receiver*)-1)
    prio = ~0UL; // highes prio for JDB IRQs
  else
    prio = Irq::self(this)->owner()->sched()->prio();
#endif

}

PUBLIC
void
Dirq_pic_pin::set_cpu(unsigned)
{}

PUBLIC
bool
Dirq_pic_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:

PUBLIC
char const *
Dirq_pic_pin::pin_type() const
{ return "HW IRQ (DIRQ)"; }

