INTERFACE[arm && tegra2]:

#define TARGET_NAME "Tegra2"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_irqs         = 258,
    Max_num_dirqs        = 256,

    Vkey_irq             = 256,
    Tbuf_irq             = 257,

    Scheduling_irq       = 29,
    scheduler_irq_vector = Scheduling_irq,
  };
};
