INTERFACE [arm && integrator]:

#define TARGET_NAME "Integrator"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 6,
    Max_num_dirqs        = 48,
  };
};

