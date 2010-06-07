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
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 64,
    Max_num_dirqs        = 32,

    Vkey_irq             = 27,
    Tbuf_irq             = 28,
  };
};

