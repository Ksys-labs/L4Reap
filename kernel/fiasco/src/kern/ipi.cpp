INTERFACE:

class Ipi
{
public:
  void init();
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

  static Per_cpu<Ipi> _ipi;
};


INTERFACE[mp && debug]:

#include "per_cpu_data.h"

EXTENSION class Ipi
{
public:
  Mword _stat_sent;
  Mword _stat_received;
};

// ------------------------------------------------------------------------
IMPLEMENTATION[!mp]:

PUBLIC static inline
Ipi &
Ipi::cpu(unsigned)
{
  return *reinterpret_cast<Ipi*>(0);
}


IMPLEMENT inline
void
Ipi::init()
{}

PUBLIC inline
void
Ipi::send(Message)
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

Per_cpu<Ipi> DEFINE_PER_CPU Ipi::_ipi;

PUBLIC static inline
Ipi &
Ipi::cpu(unsigned cpu)
{ return Ipi::_ipi.cpu(cpu); }

// ------------------------------------------------------------------------
IMPLEMENTATION[!(mp && debug)]:

PUBLIC static inline
void
Ipi::stat_sent()
{}

PUBLIC static inline
void
Ipi::stat_received()
{}

// ------------------------------------------------------------------------
IMPLEMENTATION[mp && debug]:

#include "globals.h"

PUBLIC inline
void
Ipi::stat_sent()
{ atomic_mp_add(&_stat_sent, 1); }

PUBLIC static inline NEEDS["globals.h"]
void
Ipi::stat_received()
{ _ipi.cpu(current_cpu())._stat_received++; }
