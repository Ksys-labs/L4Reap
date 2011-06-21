INTERFACE[arm && omap3_evm]:
#define TARGET_NAME "OMAP3EVM"

INTERFACE[arm && omap3_beagleboard]:
#define TARGET_NAME "Beagleboard"

INTERFACE[arm && omap4_pandaboard]:
#define TARGET_NAME "Pandaboard"

INTERFACE [arm && omap3]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 37,
    Max_num_dirqs        = 96,
  };
};

INTERFACE [arm && omap4_pandaboard]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 29,
    Max_num_dirqs        = 160,
  };
};
