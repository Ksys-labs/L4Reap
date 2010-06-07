INTERFACE:

#include "spin_lock.h"

template< typename Value_t >
class Mp_fifo
{
  friend class Jdb_mp_request_module;

public:
  typedef Value_t Value;
  class Item
  {
    friend class Mp_fifo<Value>;
    friend class Jdb_mp_request_module;

  public:
    Value value;
  private:
    Item *next;
  };

private:
  Item *_tail;
  Item *_head;
  Spin_lock _lock;
};


IMPLEMENTATION:

#include "lock_guard.h"

PUBLIC template< typename Value_t >
Mp_fifo<Value_t>::Mp_fifo() : _tail(0), _head(0)
{
  _lock.init();
}

PUBLIC inline NEEDS["spin_lock.h","lock_guard.h"]
template< typename Value_t >
void
Mp_fifo<Value_t>::enq(Item *e)
{
  Lock_guard<Spin_lock> guard(&_lock);

  e->next = 0;

  if (_head)
    _tail->next = e;
  else
    _head = e;

  _tail = e;
}


PUBLIC inline NEEDS["spin_lock.h","lock_guard.h"]
template< typename Value_t >
typename Mp_fifo<Value_t>::Item *
Mp_fifo<Value_t>::deq()
{
  Lock_guard<Spin_lock> guard(&_lock);

  Item *h = _head;

  if (_head)
    _head = _head->next;

  return h;
}
