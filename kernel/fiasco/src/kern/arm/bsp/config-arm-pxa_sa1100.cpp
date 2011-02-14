INTERFACE[arm && pxa]:
#define TARGET_NAME "XScale"

INTERFACE[arm && sa1100]:
#define TARGET_NAME "StrongARM"


INTERFACE [arm && (pxa || sa1100)]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 26,
    Max_num_dirqs        = 32,
  };
};

