INTERFACE:

#include "bitmap.h"
#include "config.h"

class Cpu_mask
{
public:
  Cpu_mask() { _b.clear_all(); }

  bool empty() const { return _b.is_empty(); }
  bool get(unsigned cpu) const { return _b[cpu]; }
  void clear(unsigned cpu) { return _b.clear_bit(cpu); }
  void set(unsigned cpu) { _b.set_bit(cpu); };
  bool atomic_get_and_clear(unsigned cpu)
  { return _b.atomic_get_and_clear(cpu); }

private:
  Bitmap<Config::Max_num_cpus> _b;
};
