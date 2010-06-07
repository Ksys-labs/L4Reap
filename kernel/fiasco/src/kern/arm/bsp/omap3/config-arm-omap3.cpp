INTERFACE[arm && omap3_evm]:
#define TARGET_NAME "OMAP3EVM"

INTERFACE[arm && omap3_beagleboard]:
#define TARGET_NAME "Beagleboard"

INTERFACE [arm && omap3]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 37,
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 98,
    Max_num_dirqs        = 96,

    Vkey_irq             = 96,
    Tbuf_irq             = 97,
  };
};
