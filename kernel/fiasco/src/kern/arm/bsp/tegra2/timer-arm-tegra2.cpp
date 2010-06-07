// --------------------------------------------------------------------------
INTERFACE [arm && tegra2 && mptimer]:

#include "irq_chip.h"
#include "irq_pin.h"

EXTENSION class Timer
{
private:
  static Irq_base *irq;
  enum
  {
    Interval = 249999,
  };
};

// -----------------------------------------------------------------------
IMPLEMENTATION [arm && tegra2]:

Irq_base *Timer::irq;
