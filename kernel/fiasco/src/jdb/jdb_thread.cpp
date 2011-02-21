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
Jdb_thread::print_state_long(Thread *t, unsigned cut_on_len = 0)
{
  static char const * const state_names[] =
    {
      "ready",         "drq_rdy",       "rcv",         "poll",
      "ipc_progr",     "snd_progr",     "cancel",      "<unk>",
      "<unk>",         "dead",          "suspended",   "<unk>",
      "<unk>",         "delayed_deadl", "delayed_ipc", "fpu",
      "alien",         "dealien",       "exc_progr",   "transfer",
      "drq",           "lock_wait",     "vcpu",        "",
      "vcpu_fpu_disabled", "vcpu_ext"
    };

  unsigned chars = 0;
  bool comma = false;

  Mword bits = t->state(false);

  for (unsigned i = 0; i < sizeof (state_names) / sizeof (char *);
       i++, bits >>= 1)
    {
      if (!(bits & 1))
        continue;

      if (cut_on_len)
        {
          unsigned add = strlen(state_names[i]) + comma;
          if (chars + add > cut_on_len)
            {
              if (chars < cut_on_len - 4)
                putstr(",...");
              break;
            }
          chars += add;
        }

      printf("%s%s", "," + !comma, state_names[i]);

      comma = 1;
    }
}

PUBLIC static
void
Jdb_thread::print_snd_partner(Thread *t, int task_format = 0)
{
  if (t->state(false) & Thread_send_in_progress)
    Jdb_kobject::print_uid(static_cast<Thread*>(t->receiver()), task_format);
  else
    // receiver() not valid
    putstr("       ");
}

PUBLIC static
void
Jdb_thread::print_partner(Thread *t, int task_format = 0)
{
  Sender *p = t->partner();

  if (!(t->state(false) & Thread_receiving))
    {
      printf("%*s ", task_format, " ");
      return;
    }

  if (!p)
    {
      printf("%*s ", task_format, "-");
      return;
    }

  if (Kobject *o = Kobject::pointer_to_obj(p))
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
    printf("\033[31;1m%p\033[m ", p);
}
