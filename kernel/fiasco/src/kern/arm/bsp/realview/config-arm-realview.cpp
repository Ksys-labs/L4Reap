INTERFACE [arm && realview]:

#define TARGET_NAME "Realview"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_dirqs        = 256,
  };
};

INTERFACE [arm && realview && sp804]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 36,
  };
};

INTERFACE [arm && realview && mptimer]:

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 29,
  };
};

