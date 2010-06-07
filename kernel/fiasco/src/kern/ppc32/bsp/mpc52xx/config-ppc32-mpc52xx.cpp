INTERFACE [ppc32 && mpc52xx]:

#include "pic.h"

#define TARGET_NAME "MPC52xx"

EXTENSION class Config
{
public:
  enum
  {

    Max_num_dirqs = Pic::IRQ_MAX,
    Vkey_irq      = Max_num_dirqs,
    Tbuf_irq      = Max_num_dirqs + 1,
    Max_num_irqs  = Max_num_dirqs + 2,
  };
};
