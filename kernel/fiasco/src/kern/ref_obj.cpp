INTERFACE:

#include "types.h"

class Ref_cnt_obj
{
public:
  Ref_cnt_obj() : _ref_cnt(0) {}
private:
  Smword _ref_cnt;
};


// -------------------------------------------------------------------------
IMPLEMENTATION:

#include "atomic.h"

PUBLIC inline
Smword
Ref_cnt_obj::ref_cnt() const
{ return _ref_cnt; }

PUBLIC inline NEEDS["atomic.h"]
void
Ref_cnt_obj::inc_ref()
{ atomic_mp_add((Mword*)&_ref_cnt, 1); }

PUBLIC inline NEEDS["atomic.h"]
Smword
Ref_cnt_obj::dec_ref()
{
  Smword old;
  do
    old = _ref_cnt;
  while (!mp_cas(&_ref_cnt, old, old -1));

  return old -1;
}
