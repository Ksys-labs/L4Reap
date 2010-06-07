INTERFACE:

#include "irq.h"

class Virq : public Irq
{
private:
  Virq();
  Virq(Virq&);
};


IMPLEMENTATION:

#include "config.h"
#include "kdb_ke.h"
#include "atomic.h"
#include "thread_state.h"
#include "static_init.h"

PUBLIC inline
explicit
Virq::Virq(unsigned /*irqnum*/) : Irq(0)
{
}

PUBLIC inline
void
Virq::notify()
{
  hit();
}

PUBLIC
bool
Virq::put()
{ return false; }



PUBLIC
bool
Virq::check_debug_irq()
{ return true; }

PUBLIC
void
Virq::mask()
{}

PUBLIC
void
Virq::ack()
{}

PUBLIC
void
Virq::mask_and_ack()
{}

PUBLIC
void
Virq::unmask()
{}

