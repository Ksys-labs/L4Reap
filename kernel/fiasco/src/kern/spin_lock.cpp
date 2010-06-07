INTERFACE:

#include "cpu_lock.h"

/**
 * \brief Basic spin lock.
 *
 * Also disables lock IRQs for the time the lock is held.
 * In the UP case it is in fact just the Cpu_lock.
 */
class Spin_lock : private Cpu_lock
{
};

//--------------------------------------------------------------------------
INTERFACE [!mp]:

EXTENSION class Spin_lock
{
public:
  void init() {}

  using Cpu_lock::Status;
  using Cpu_lock::test;
  using Cpu_lock::lock;
  using Cpu_lock::clear;
  using Cpu_lock::test_and_set;
  using Cpu_lock::set;

};

/**
 * \brief Version of a spin lock that is colocated with another value.
 */
template< typename T >
class Spin_lock_coloc : public Spin_lock
{
private:
  enum { Arch_lock = 1 };
  Mword _lock;
};


//--------------------------------------------------------------------------
INTERFACE [mp]:

EXTENSION class Spin_lock
{
public:
  typedef Mword Status;

protected:
  Mword _lock;
};

/**
 * \brief Version of a spin lock that is colocated with another value.
 */
template< typename T >
class Spin_lock_coloc : public Spin_lock
{
};

//--------------------------------------------------------------------------
IMPLEMENTATION:

PUBLIC inline
template< typename T >
T
Spin_lock_coloc<T>::get_unused() const
{ return (T)(_lock & ~Arch_lock); }

PUBLIC inline
template< typename T >
void
Spin_lock_coloc<T>::set_unused(T val)
{ _lock = (_lock & Arch_lock) | (Mword)val; }


//--------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include <cassert>
#include "mem.h"

PUBLIC inline
void
Spin_lock::init()
{
  _lock = 0;
}

PUBLIC inline
Spin_lock::Status
Spin_lock::test() const
{
  return (!!cpu_lock.test()) | (_lock & Arch_lock);
}

PUBLIC inline NEEDS[<cassert>, Spin_lock::lock_arch, "mem.h"]
void
Spin_lock::lock()
{
  assert(!cpu_lock.test());
  cpu_lock.lock();
  lock_arch();
  Mem::mp_mb();
}

PUBLIC inline NEEDS[Spin_lock::unlock_arch, "mem.h"]
void
Spin_lock::clear()
{
  Mem::mp_mb();
  unlock_arch();
  Cpu_lock::clear();
}

PUBLIC inline NEEDS[Spin_lock::lock_arch, "mem.h"]
Spin_lock::Status
Spin_lock::test_and_set()
{
  Status s = !!cpu_lock.test();
  cpu_lock.lock();
  lock_arch();
  Mem::mp_mb();
  return s;
}

PUBLIC inline
void
Spin_lock::set(Status s)
{
  Mem::mp_mb();
  if (!(s & Arch_lock))
    unlock_arch();

  if (!(s & 1))
    cpu_lock.clear();
}


