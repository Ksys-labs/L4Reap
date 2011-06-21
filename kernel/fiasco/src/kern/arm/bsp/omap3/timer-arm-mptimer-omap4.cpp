// --------------------------------------------------------------------------
INTERFACE [arm && mptimer && omap4_pandaboard]:

EXTENSION class Timer
{
private:
  enum { Interval = 499999, };
};
