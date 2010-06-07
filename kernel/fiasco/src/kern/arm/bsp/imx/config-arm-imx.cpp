INTERFACE [arm && imx]:

#define TARGET_NAME "i.MX"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 26,
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 66,
    Max_num_dirqs        = 64,
    Vkey_irq             = 64,
    Tbuf_irq             = 65,
  };
};

