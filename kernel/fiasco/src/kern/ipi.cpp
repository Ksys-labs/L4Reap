INTERFACE:

class Ipi
{
public:
  static void init(unsigned _lcpu);
};

INTERFACE[!mp]:

EXTENSION class Ipi
{
public:
  enum Message { Request, Global_request, Debug };
};

INTERFACE[mp]:

#include "spin_lock.h"

EXTENSION class Ipi
{
private:

  // remote call
  static Spin_lock _remote_call_lock;
  static void (*_remote_call_func)(void *);
  static void *_remote_call_func_data;
  static unsigned long _remote_call_done;
};


INTERFACE[mp && debug]:

#include "per_cpu_data.h"

EXTENSION class Ipi
{
public:
  static Per_cpu <Mword> _stat_sent;
  static Per_cpu <Mword> _stat_received;
};

// ------------------------------------------------------------------------
IMPLEMENTATION[!mp]:

IMPLEMENT static inline
void
Ipi::init(unsigned)
{}


PUBLIC static inline
void
Ipi::send(int, Message)
{}

PUBLIC static inline
void
Ipi::eoi(Message)
{}

PUBLIC static inline
void
Ipi::bcast(Message)
{}

// ------------------------------------------------------------------------
IMPLEMENTATION[mp]:

#include <cstdio>

#include "cpu.h"
#include "lock_guard.h"

#if 0
PRIVATE static
void
Ipi::invalid_remote_func(void *)
{
  printf("IPI with func?\n");
}

PUBLIC static
void
Ipi::process_remote_call()
{
  if (Ipi::_remote_call_func)
    Ipi::_remote_call_func(Ipi::_remote_call_func_data);

  asm volatile("" : : : "memory");

  _remote_call_done = 1;
}

PUBLIC static
bool
Ipi::remote_call_wait(unsigned this_cpu, unsigned to_cpu,
                      void (*f)(void *), void *data)
{
  if (to_cpu == this_cpu)
    {
      //ipi_call_debug_arch();
      f(data);
      return true;
    }

  if (!Cpu::online(to_cpu))
    return false;

  Lock_guard<Spin_lock> guard(&_remote_call_lock);

  _remote_call_func      = f;
  _remote_call_func_data = data;
  _remote_call_done      = 0;

  send(to_cpu);

  while (!*(volatile unsigned long *)&_remote_call_done) // XXX add timeout?
    ;

  _remote_call_func = invalid_remote_func;

  return true;
}

PUBLIC static
bool
Ipi::remote_call_nowait(unsigned this_cpu, unsigned to_cpu,
                        void (*f)(void *), void *data)
{
  if (to_cpu == this_cpu)
    {
      //ipi_call_debug_arch();
      f(data);
      return true;
    }

  if (!Cpu::online(to_cpu))
    return false;

  _remote_call_func      = f;
  _remote_call_func_data = data;
  _remote_call_done      = 0;

  send(to_cpu);

  return true;
}
#endif


// ------------------------------------------------------------------------
IMPLEMENTATION[!(mp && debug)]:

PUBLIC static inline
void
Ipi::stat_sent(unsigned)
{}

PUBLIC static inline
void
Ipi::stat_received()
{}

// ------------------------------------------------------------------------
IMPLEMENTATION[mp && debug]:

#include "globals.h"

Per_cpu <Mword> DEFINE_PER_CPU Ipi::_stat_sent;
Per_cpu <Mword> DEFINE_PER_CPU Ipi::_stat_received;

PUBLIC static inline
void
Ipi::stat_sent(unsigned to_cpu)
{ atomic_mp_add(&_stat_sent.cpu(to_cpu), 1); }

PUBLIC static inline NEEDS["globals.h"]
void
Ipi::stat_received()
{ _stat_received.cpu(current_cpu())++; }
