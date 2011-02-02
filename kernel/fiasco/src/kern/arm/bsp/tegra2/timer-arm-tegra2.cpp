// --------------------------------------------------------------------------
INTERFACE [arm && tegra2 && mptimer]:

EXTENSION class Timer
{
private:
  enum
  {
    Interval = 249999,
  };
};

