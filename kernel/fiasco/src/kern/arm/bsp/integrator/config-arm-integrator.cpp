INTERFACE [arm && integrator]:

#define TARGET_NAME "Integrator"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 6,
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 50,
    Max_num_dirqs        = 48,
    Vkey_irq             = 48,
    Tbuf_irq             = 49,
  };
};

