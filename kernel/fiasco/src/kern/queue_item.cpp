INTERFACE:

class Queue;

class Queue_item
{
  friend class Queue;
public:
  enum Status { Ok, Retry, Invalid };

private:
  Queue_item **_pn, *_n;
  Queue *_q;
} __attribute__((aligned(16)));


//--------------------------------------------------------------------------
IMPLEMENTATION:

#include "kdb_ke.h"
#include "std_macros.h"

PUBLIC inline
Queue_item::Queue_item()
  : _n(0)
{}

PUBLIC inline
bool
Queue_item::queued() const
{ return _n; }

PUBLIC inline NEEDS["kdb_ke.h"]
Queue *
Queue_item::queue() const
{
  assert_kdb (queued());
  return _q;
}

PUBLIC inline NEEDS["kdb_ke.h"]
Queue_item::Status
Queue_item::status() const
{
  assert_kdb (!queued());
  return Status((unsigned long)_q);
}

