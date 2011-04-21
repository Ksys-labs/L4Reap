#include "irqs.h"
#include "main.h"

int
Kernel_irq_pin::unbind()
{
  int err = l4_error(system_icu()->icu->unbind(_idx, irq()));
  set_shareable(false);
  return err;
}

int
Kernel_irq_pin::bind(L4::Cap<L4::Irq> irq, unsigned mode)
{
  int err = l4_error(system_icu()->icu->bind(_idx, irq));

  // allow sharing if IRQ must be acknowledged via the IRQ object 
  if (err == 0)
    set_shareable(true);

  if (err < 0)
    return err;

  // printf(" IRQ[%x]: mode=%x ... ", n, mode);
  err = l4_error(system_icu()->icu->set_mode(_idx, mode));
  // printf("result=%d\n", err);

  return err;
}

int
Kernel_irq_pin::unmask()
{
  system_icu()->icu->unmask(_idx);
  return -L4_EINVAL;
}

