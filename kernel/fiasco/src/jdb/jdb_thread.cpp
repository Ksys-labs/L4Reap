INTERFACE:

#include "thread.h"

class Jdb_thread
{
};

IMPLEMENTATION:

#include "irq.h"
#include "jdb_kobject.h"
#include "thread_state.h"
#include "vlog.h"

#include <cstdio>

PUBLIC static
void
Jdb_thread::print_snd_partner(Thread *t, int task_format = 0)
{
  if (t->state() & Thread_send_in_progress)
    Jdb_kobject::print_uid(t->lookup(static_cast<Thread*>(t->receiver())), task_format);
  else
    // receiver() not valid
    putstr("       ");
}

PUBLIC static
void
Jdb_thread::print_partner(Thread *t, int task_format = 0)
{
  Thread *p;
  Kobject *o;

  if (!(t->state() & (Thread_receiving | Thread_busy)))
    {
      printf("%*s ", task_format, " ");
      return;
    }

  if (!t->partner())
    {
      printf("%*s ", task_format, "-");
      return;
    }

  // the Thread* cast is not good because we actually need to have a dynamic
  // cast but we don't have this here in this environment. Luckily the cast
  // works...
  if (Kobject::is_kobj(p = static_cast<Thread*>(t->partner())))
    {
      char flag = ' ';
      printf("%*.lx%c", task_format, p->dbg_id(), flag);
    }
  else if ((o = Kobject::pointer_to_obj(t->partner())))
    printf("%*.lx*", task_format, o->dbg_id());
  else
    printf("\033[31;1m%p\033[m ", t->partner());
}
