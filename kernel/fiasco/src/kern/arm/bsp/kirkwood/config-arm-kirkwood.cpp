INTERFACE[arm && kirkwood]:

#define TARGET_NAME "Marvell Kirkwood"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_irqs         = 66,
    Max_num_dirqs        = 64,

    Vkey_irq             = 64,
    Tbuf_irq             = 65,

    Scheduling_irq       = 1,
    scheduler_irq_vector = Scheduling_irq,
  };
};
