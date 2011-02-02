INTERFACE:

// 
// Lock_guard: a guard object using a lock such as helping_lock_t
// 
template<typename LOCK>
class Lock_guard
{
  LOCK *_lock;
  typename LOCK::Status _state;
};

template< typename LOCK>
class Lock_guard_2
{
  LOCK *_l1, *_l2;
  typename LOCK::Status _state1, _state2;
};

IMPLEMENTATION:

PUBLIC template<typename LOCK>
inline
Lock_guard<LOCK>::Lock_guard()
  : _lock(0)
#ifndef NDEBUG
    , _state(LOCK::Invalid) // silence GCC warning
#endif
{}

PUBLIC template<typename LOCK>
inline
Lock_guard<LOCK>::Lock_guard(LOCK *l)
  : _lock(l)
{
  _state = _lock->test_and_set();
}

PUBLIC template<typename LOCK>
inline
bool
Lock_guard<LOCK>::lock(LOCK *l)
{
  _lock = l;
  _state = l->test_and_set();
  switch (_state)
    {
    case LOCK::Locked:
      return true;
    case LOCK::Not_locked:
      _lock = l;			// Was not locked -- unlock.
      return true;
    default:
      return false; // Error case -- lock not existent
    }
}

PUBLIC template<typename LOCK>
inline
void
Lock_guard<LOCK>::release()
{
  if (_lock)
    {
      _lock->set(_state);
      _lock = 0;
    }
}

PUBLIC template<typename LOCK>
inline
Lock_guard<LOCK>::~Lock_guard()
{
  if (_lock)
    _lock->set(_state);
}

PUBLIC template<typename LOCK>
inline
bool
Lock_guard<LOCK>::was_set(void)
{
  return _state; //!_lock;
}

PUBLIC template<typename LOCK>
inline
Lock_guard_2<LOCK>::Lock_guard_2()
  : _l1(0), _l2(0)
{}

PUBLIC template<typename LOCK>
inline
Lock_guard_2<LOCK>::Lock_guard_2(LOCK *l1, LOCK *l2)
  : _l1(l1 < l2 ? l1 : l2), _l2(l1 < l2 ? l2 : l1)
{
  _state1 = _l1->test_and_set();
  if (_l1 == _l2)
    _l2 = 0;
  else
    _state2 = _l2->test_and_set();
}


PUBLIC template<typename LOCK>
inline
bool
Lock_guard_2<LOCK>::lock(LOCK *l1, LOCK *l2)
{
  _l1 = l1 < l2 ? l1 : l2;
  _l2 = l1 < l2 ? l2 : l1;
  if ((_state1 = _l1->test_and_set()) == LOCK::Invalid)
    {
      _l1 = _l2 = 0;
      return false;
    }

  if (_l1 == _l2)
    _l2 = 0;
  else if ((_state2 = _l2->test_and_set()) == LOCK::Invalid)
    {
      _l2 = 0;
      return false;
    }

  return true;
}

PUBLIC template<typename LOCK>
inline
Lock_guard_2<LOCK>::~Lock_guard_2()
{
  if (_l2)
    _l2->set(_state2);

  if (_l1)
    _l1->set(_state1);
}
