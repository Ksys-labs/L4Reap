INTERFACE:

#include <cassert>

#include "mem_layout.h"
#include "panic.h"
#include "per_cpu_data.h"
#include "types.h"

class Context;
class Mem_space;
class Task;
class Space;
class Thread;
class Timeout;

extern Space *sigma0_task;
extern Task *boot_task;
extern Mem_space *sigma0_space;
extern Per_cpu<Timeout *> timeslice_timeout;
extern bool running;
extern unsigned boot_stack;

/* the check macro is like assert(), but it evaluates its argument
   even if NDEBUG is defined */
#ifndef check
#ifdef NDEBUG
# define check(expression) ((void)(expression))
#else /* ! NDEBUG */
# ifdef ASSERT_KDB_KE
#  define check(expression) assert(expression)
# else
#  define check(expression) \
          ((void)((expression) ? 0 : \
                 (panic(__FILE__":%u: failed check `"#expression"'", \
                         __LINE__), 0)))
# endif
#endif /* ! NDEBUG */
#endif /* check */

// nil_thread and kernel_thread might have different values on a SMP system
extern Thread *nil_thread;
static Thread *&kernel_thread = nil_thread;


class Global_context_data
{
public:
  virtual ~Global_context_data() {}

protected:
  Mword _state;
};


//---------------------------------------------------------------------------
INTERFACE [mp]:

EXTENSION class Global_context_data
{
protected:
  friend unsigned &__cpu_of(const void *);
  unsigned _cpu;
};

//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "config.h"
#include "processor.h"

Space *sigma0_task;
Task *boot_task;
Mem_space *sigma0_space;
Thread *nil_thread;
bool running = true;
unsigned boot_stack;

inline NEEDS ["config.h"]
Context *context_of(const void *ptr)
{
  return reinterpret_cast<Context *>
    (reinterpret_cast<unsigned long>(ptr) & ~(Config::thread_block_size - 1));
}

inline NEEDS [context_of, "processor.h"]
Context *current()
{ return context_of ((void *)Proc::stack_pointer()); }



IMPLEMENTATION [!mp]:

inline
void set_cpu_of(const void *ptr, unsigned cpu)
{ (void)ptr; (void)cpu; }

inline
unsigned cpu_of(const void *)
{ return 0; }

inline
unsigned current_cpu()
{ return 0; }


IMPLEMENTATION [mp]:

inline NEEDS ["config.h"]
unsigned &__cpu_of(const void *ptr)
{ return reinterpret_cast<Global_context_data*>(context_of(ptr))->_cpu; }

inline NEEDS [__cpu_of]
void set_cpu_of(const void *ptr, unsigned cpu)
{ __cpu_of(ptr) = cpu; }


inline NEEDS [__cpu_of]
unsigned cpu_of(const void *ptr)
{ return __cpu_of(ptr); }

inline NEEDS [current, cpu_of]
unsigned current_cpu()
{ return cpu_of(current()); }

