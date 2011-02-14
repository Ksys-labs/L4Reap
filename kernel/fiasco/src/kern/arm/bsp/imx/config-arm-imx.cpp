INTERFACE [arm && imx21]:

#define TARGET_NAME "i.MX21"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 26,
    Max_num_dirqs        = 64,
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
    Max_num_dirqs        = 128,
  };
};

