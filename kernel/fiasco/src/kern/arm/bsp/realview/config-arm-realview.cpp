INTERFACE [arm && realview]:

#define TARGET_NAME "Realview"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_irqs         = 258,
    Max_num_dirqs        = 256,

    Vkey_irq             = 256,
    Tbuf_irq             = 257,
  };
};

INTERFACE [arm && realview && sp804]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 36,
    scheduler_irq_vector = Scheduling_irq,
  };
};

INTERFACE [arm && realview && mptimer]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 29,
    scheduler_irq_vector = Scheduling_irq,
  };
};

