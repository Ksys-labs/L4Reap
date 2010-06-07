INTERFACE [ppc32 && qemu]:

#define TARGET_NAME "QEMU"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_dirqs = 3,
    Vkey_irq      = Max_num_dirqs,
    Tbuf_irq      = Max_num_dirqs + 1,
    Max_num_irqs  = Max_num_dirqs + 2,
  };
};
