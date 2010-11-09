INTERFACE:

#include "thread_object.h"

class Jdb_thread
{
};

IMPLEMENTATION:

#include "irq.h"
#include "jdb_kobject.h"
#include "kobject.h"
#include "thread_state.h"
#include "vlog.h"

#include <cstdio>

PUBLIC static
void
Jdb_thread::print_snd_partner(Thread *t, int task_format = 0)
{
  if (t->state() & Thread_send_in_progress)
    Jdb_kobject::print_uid(t->lookup(static_cast<Thread*>(t->receiver()))->kobject(), task_format);
  else
    // receiver() not valid
    putstr("       ");
}

PUBLIC static
void
Jdb_thread::print_partner(Thread *t, int task_format = 0)
{
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

  if (Kobject::is_kobj(o = Kobject::pointer_to_obj(t->partner())))
    {
      char flag = '?';
      const char *n = o->kobj_type();

      if (n == Thread_object::static_kobj_type)
        flag = ' ';
      else if (n == Irq::static_kobj_type)
        flag = '*';

      printf("%*.lx%c", task_format, o->dbg_info()->dbg_id(), flag);
    }
  else
    printf("\033[31;1m%p\033[m ", t->partner());
}
