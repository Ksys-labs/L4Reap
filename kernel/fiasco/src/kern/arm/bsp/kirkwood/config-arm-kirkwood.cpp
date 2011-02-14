INTERFACE[arm && kirkwood]:

#define TARGET_NAME "Marvell Kirkwood"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_dirqs        = 64,
    Scheduling_irq       = 1,
  };
};
