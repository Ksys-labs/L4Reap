INTERFACE:

#include "irq.h"
#include "ram_quota.h"
#include "icu_helper.h"

class Irq_chip;

class Icu : public Icu_h<Icu>
{
  FIASCO_DECLARE_KOBJ();

  friend class Icu_h<Icu>;
};


//----------------------------------------------------------------------------
IMPLEMENTATION:

#include "entry_frame.h"
#include "irq.h"
#include "irq_chip.h"
#include "l4_types.h"
#include "l4_buf_iter.h"

FIASCO_DEFINE_KOBJ(Icu);

PRIVATE static inline NEEDS["irq_chip.h"]
Irq_chip *
Icu::chip(bool msi)
{ return msi ?  Irq_chip::hw_chip_msi : Irq_chip::hw_chip; }

PUBLIC inline NEEDS[Icu::chip]
Irq_base *
Icu::icu_get_irq(unsigned irqnum)
{
  Irq_chip *c = chip(irqnum & Msi_bit);

  if (EXPECT_FALSE(!c))
    return 0;

  return c->irq(irqnum & ~Msi_bit);
}


PUBLIC inline NEEDS[Icu::chip]
L4_msg_tag
Icu::icu_bind_irq(Irq *irq, unsigned irqnum)
{
  Irq_chip *c = chip(irqnum & Msi_bit);

  if (!c)
    return commit_result(-L4_err::EInval);

  irq->pin()->unbind_irq();
 
  if (!c->alloc(irq, irqnum & ~Msi_bit))
    return commit_result(-L4_err::EPerm);

  return commit_result(0);
}


PUBLIC inline NEEDS[Icu::chip]
void
Icu::icu_get_info(Mword *features, Mword *num_irqs, Mword *num_msis)
{
  *features = 0 | (Irq_chip::hw_chip_msi ? (unsigned)Msi_bit : 0);
  *num_irqs = Irq_chip::hw_chip->nr_irqs();
  *num_msis = Irq_chip::hw_chip_msi
              ? Irq_chip::hw_chip_msi->nr_irqs()
              : 0;
}

PUBLIC inline NEEDS[Icu::chip]
L4_msg_tag
Icu::icu_get_msi_info(Mword msi, Utcb *out)
{
  if (!Irq_chip::hw_chip_msi)
    return commit_result(-L4_err::EInval);

  out->values[0] = Irq_chip::hw_chip_msi->msg(msi);
  return commit_result(0, 1);
}


PUBLIC inline
Icu::Icu()
{
  initial_kobjects.register_obj(this, 6);
}

static Icu icu;

