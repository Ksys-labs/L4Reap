INTERFACE:

class Cpu_mask
{
public:
  Cpu_mask() : _m(0) {}

  bool empty() const;
  bool get(unsigned cpu) const;
  void clear(unsigned cpu);
  void set(unsigned cpu);
  bool atomic_get_and_clear(unsigned cpu);

private:
  // currently up to MWORD_BITS cpus are supported by this mask
  unsigned long _m;
};


//---------------------------------------------------------------------------
IMPLEMENTATION:

#include <atomic.h>

IMPLEMENT inline
bool
Cpu_mask::empty() const
{ return !_m; }


IMPLEMENT inline
bool
Cpu_mask::get(unsigned cpu) const
{ return _m & (1UL << cpu); }


IMPLEMENT inline
void
Cpu_mask::clear(unsigned cpu)
{ _m = (_m & ~(1UL << cpu)); }


IMPLEMENT inline
void
Cpu_mask::set(unsigned cpu)
{ _m |= 1UL << cpu; }

IMPLEMENT inline NEEDS [<atomic.h>]
bool
Cpu_mask::atomic_get_and_clear(unsigned cpu)
{
  unsigned long v;
  do
    {
      v = _m;
    }
  while (!mp_cas(&_m, v, v & ~(1UL << cpu)));

  return v & (1UL << cpu);
}

