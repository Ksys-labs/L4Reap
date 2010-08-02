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
