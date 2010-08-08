IMPLEMENTATION:

#include <climits>
#include <cstring>
#include <cstdio>

#include "jdb.h"
#include "jdb_core.h"
#include "jdb_module.h"
#include "jdb_screen.h"
#include "jdb_kobject.h"
#include "simpleio.h"
#include "static_init.h"
#include "ipc_gate.h"

class Jdb_ipc_gate : public Jdb_kobject_handler
{
public:
  Jdb_ipc_gate() FIASCO_INIT;
};

IMPLEMENT
Jdb_ipc_gate::Jdb_ipc_gate()
  : Jdb_kobject_handler(Ipc_gate_obj::static_kobj_type)
{
  Jdb_kobject::module()->register_handler(this);
}

PUBLIC
Kobject *
Jdb_ipc_gate::follow_link(Kobject *o)
{
  Ipc_gate_obj *g = Kobject::dcast<Ipc_gate_obj *>(o);
  return g->thread() ? static_cast<Kobject*>(g->thread()) : o;
}

PUBLIC
bool
Jdb_ipc_gate::show_kobject(Kobject *, int)
{ return true; }

PUBLIC
int
Jdb_ipc_gate::show_kobject_short(char *buf, int max, Kobject *o)
{
  Ipc_gate_obj *g = Kobject::dcast<Ipc_gate_obj*>(o);
  if (!g)
    return 0;

  return snprintf(buf, max, " L=%s%08lx\033[0m D=%lx",
                  (g->id() & 3) ? JDB_ANSI_COLOR(lightcyan) : "",
                  g->id(), g->thread() ? g->thread()->dbg_id() : 0);
}

PUBLIC
char const *
Jdb_ipc_gate::kobject_type() const
{
  return JDB_ANSI_COLOR(magenta) "Gate" JDB_ANSI_COLOR(default);
}

static Jdb_ipc_gate jdb_space INIT_PRIORITY(JDB_MODULE_INIT_PRIO);

