INTERFACE [debug]:

EXTENSION class Thread
{
protected:
  struct Log_thread_exregs
  {
    Mword       id, ip, sp, op;
  };
  static unsigned fmt_exregs(Tb_entry *, int, char *) asm ("__fmt_thread_exregs");
};

//--------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include <cstdio>
#include "config.h"
#include "kmem.h"
#include "mem_layout.h"
#include "simpleio.h"


//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:


PUBLIC
void
Thread::print_state_long (unsigned cut_on_len = 0)
{
  static char const * const state_names[] =
    {
      "ready",         "drq_rdy",       "rcv",         "poll",
      "ipc_progr",     "snd_progr",     "busy",        "lipc_ok",
      "cancel",        "dead",          "suspended",   "<unk>",
      "<unk>",         "delayed_deadl", "delayed_ipc", "fpu",
      "alien",         "dealien",       "exc_progr",   "transfer",
      "drq",           "lock_wait",     "vcpu",        "vcpu_user",
      "vcpu_fpu_disabled"
    };

  Mword i, comma=0, chars=0, bits=state();

  for (i = 0; i < sizeof (state_names) / sizeof (char *); i++, bits >>= 1)
    {
      if (!(bits & 1))
        continue;

      if (cut_on_len)
        {
          unsigned add = strlen (state_names[i]) + comma;
          if (chars + add > cut_on_len)
            {
              if (chars < cut_on_len - 4)
                putstr(",...");
              break;
            }
          chars += add;
        }

      printf ("%s%s", "," + !comma, state_names[i]);

      comma = 1;
    }
}

IMPLEMENT
unsigned
Thread::fmt_exregs(Tb_entry *e, int max, char *buf)
{
  Log_thread_exregs *l = e->payload<Log_thread_exregs>();
  return snprintf(buf, max, "D=%lx ip=%lx sp=%lx op=%s%s%s",
                  l->id, l->ip, l->sp,
                  l->op & Exr_cancel ? "Cancel" : "",
                  ((l->op & (Exr_cancel |
                             Exr_trigger_exception))
                   == (Exr_cancel |
                       Exr_trigger_exception))
                   ? ","
                   : ((l->op & (Exr_cancel |
                                Exr_trigger_exception))
                      == 0 ? "0" : "") ,
                  l->op & Exr_trigger_exception ? "TrExc" : "");
}
