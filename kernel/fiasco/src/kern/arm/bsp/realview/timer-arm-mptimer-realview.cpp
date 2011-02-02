// --------------------------------------------------------------------------
INTERFACE [arm && mptimer && !realview_pbx]:

EXTENSION class Timer
{
private:
  enum { Interval = 104999, /* assumed 210MHz */};
};

// --------------------------------------------------------------------------
INTERFACE [arm && mptimer && realview_pbx]:

EXTENSION class Timer
{
private:
  enum { Interval = 49999, };
};
