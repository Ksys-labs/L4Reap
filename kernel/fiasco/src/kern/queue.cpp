INTERFACE:

#include "spin_lock.h"
#include "queue_item.h"
#include "kdb_ke.h"


class Queue
{
public:
  typedef Spin_lock_coloc<Mword> Inner_lock;

private:
  class Lock_n_ptr : public Inner_lock
  {
  public:
    Queue_item *item() const
    { return reinterpret_cast<Queue_item*>(get_unused() & ~5UL); }

    void set_item(Queue_item *i)
    {
      assert_kdb (!(Mword(i) & 5));
      set_unused((Mword)i | (get_unused() & 5));
    }

    bool blocked() const
    { return get_unused() & 1; }

    void block()
    { return set_unused(get_unused() | 1); }

    void unblock()
    { set_unused(get_unused() & ~1); }

    bool invalid() const
    { return get_unused() & 4; }

    void invalidate()
    { set_unused(get_unused() | 4); }
  };

  Lock_n_ptr _m;
};


//--------------------------------------------------------------------------
IMPLEMENTATION:

#include "kdb_ke.h"
#include "std_macros.h"

PUBLIC inline
Queue::Queue()
{ _m.init(); }

PUBLIC inline
Queue::Inner_lock *
Queue::q_lock()
{ return &_m; }

PUBLIC inline NEEDS["kdb_ke.h"]
void
Queue::enqueue(Queue_item *i)
{
  // Queue i at the end of the list
  assert_kdb (_m.test());
  Queue_item *const h = _m.item();
  i->_q = this;
  if (h)
    {
      i->_n = h;
      i->_pn = h->_pn;
      *(h->_pn) = i;
      h->_pn = &i->_n;
    }
  else
    {
      i->_pn = &i->_n;
      i->_n = i;
      _m.set_item(i);
    }
}

PUBLIC inline NEEDS["kdb_ke.h", "std_macros.h"]
bool
Queue::dequeue(Queue_item *i, Queue_item::Status reason)
{
  assert_kdb (_m.test());
  assert_kdb (i->queued());

  if (EXPECT_FALSE(i->_q != this))
    return false;

  if (i->_n == i)
    { // i->_n points to itself, must be the last item !
      assert_kdb (_m.item() == i);
      _m.set_item(0);
      i->_n = 0; // mark item dequeued
      i->_q = (Queue*)reason;
      return true;
    }

  *i->_pn = i->_n; // next ptr of prev to i's next item
  i->_n->_pn = i->_pn; // prev ptr of next to i's prev item

  if (_m.item() == i)
    _m.set_item(i->_n);

  i->_n = 0; // mark item dequeued
  i->_q = (Queue*)reason;
  return true;
}

PUBLIC inline
Queue_item *
Queue::first() const
{ return _m.item(); }

PUBLIC inline
bool
Queue::blocked() const
{ return _m.blocked(); }

PUBLIC inline NEEDS["kdb_ke.h"]
void
Queue::block()
{
  assert_kdb (_m.test());
  _m.block();
}

PUBLIC inline NEEDS["kdb_ke.h"]
void
Queue::unblock()
{
  assert_kdb (_m.test());
  _m.unblock();
}

PUBLIC inline NEEDS["kdb_ke.h"]
bool
Queue::invalid() const
{
  assert_kdb (_m.test());
  return _m.invalid();
}

PUBLIC inline NEEDS["kdb_ke.h"]
void
Queue::invalidate()
{
  assert_kdb (_m.test());
  _m.invalidate();
}

