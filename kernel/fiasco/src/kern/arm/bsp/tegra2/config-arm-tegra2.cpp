INTERFACE[arm && tegra2]:

#define TARGET_NAME "Tegra2"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_dirqs        = 160,
    Scheduling_irq       = 29,
  };
};
