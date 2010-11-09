IMPLEMENTATION:

#include <climits>
#include <cstring>
#include <cstdio>

#include "jdb.h"
#include "jdb_core.h"
#include "jdb_module.h"
#include "jdb_screen.h"
#include "jdb_kobject.h"
#include "kernel_console.h"
#include "keycodes.h"
#include "simpleio.h"
#include "static_init.h"
#include "u_semaphore.h"

class Jdb_semaphore : public Jdb_kobject_handler
{
public:
  Jdb_semaphore() FIASCO_INIT;
};

IMPLEMENT
Jdb_semaphore::Jdb_semaphore()
  : Jdb_kobject_handler(U_semaphore::static_kobj_type)
{
  Jdb_kobject::module()->register_handler(this);
}

PUBLIC
bool
Jdb_semaphore::show_kobject(Kobject *, int )
{
  return true;
}

PUBLIC
char const *
Jdb_semaphore::kobject_type() const
{
  return JDB_ANSI_COLOR(yellow) "Sem" JDB_ANSI_COLOR(default);
}

PUBLIC
int
Jdb_semaphore::show_kobject_short(char *buf, int max, Kobject *o)
{
  U_semaphore *u = Kobject::dcast<U_semaphore*>(o);
  Prio_list_elem *p = u->_queue.head();

  if (!p)
    return snprintf(buf, max, " no waiters");

  int len = snprintf(buf, max, " blocked=");
  while (p)
    {
      Prio_list_elem *s = p->_s_next;
      do
        {
          Thread *t = static_cast<Thread *>(Sender::cast(s));
          len += snprintf(buf + len, max - len,
                          "%s%lx", s == p ? "" : ",", t->dbg_info()->dbg_id());
          s = s->_s_next;
        } while (s != p);
      p = p->_p_next;
    }
  return len;
}

static Jdb_semaphore jdb_semaphore INIT_PRIORITY(JDB_MODULE_INIT_PRIO);
