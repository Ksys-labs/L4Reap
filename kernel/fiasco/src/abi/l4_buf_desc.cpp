INTERFACE:

#include "types.h"

class L4_buf_desc
{
public:
  enum Flags
  {
    Inherit_fpu = (1UL << 24)
  };

  L4_buf_desc() {}

  L4_buf_desc(unsigned mem, unsigned io, unsigned obj,
              unsigned flags = 0)
  : _raw(mem | (io << 5) | (obj << 10) | flags)
  {}

  unsigned mem() const { return _raw & ((1UL << 5)-1); }
  unsigned io()  const { return (_raw >> 5) & ((1UL << 5)-1); }
  unsigned obj() const { return (_raw >> 10) & ((1UL << 5)-1); }
  Mword flags() const { return _raw; }

  Mword raw() const { return _raw; }

private:
  Mword _raw;
};
