INTERFACE [arm && imx21]:

#define TARGET_NAME "i.MX21"

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

INTERFACE [arm && imx51]:

#define TARGET_NAME "i.MX51"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 40,
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 130,
    Max_num_dirqs        = 128,
    Vkey_irq             = 128,
    Tbuf_irq             = 129,
  };
};

