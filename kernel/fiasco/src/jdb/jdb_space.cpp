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
#include "kernel_task.h"
#include "keycodes.h"
#include "ram_quota.h"
#include "simpleio.h"
#include "task.h"
#include "thread_object.h"
#include "static_init.h"

class Jdb_space : public Jdb_module, public Jdb_kobject_handler
{
public:
  Jdb_space() FIASCO_INIT;
private:
  static Task *task;
};

Task *Jdb_space::task;

IMPLEMENT
Jdb_space::Jdb_space()
  : Jdb_module("INFO"), Jdb_kobject_handler(Task::static_kobj_type)
{
  Jdb_kobject::module()->register_handler(this);
}

PUBLIC
bool
Jdb_space::show_kobject(Kobject *o, int lvl)
{
  Task *t = Kobject::dcast<Task*>(o);
  show(t);
  if (lvl)
    {
      Jdb::getchar();
      return true;
    }

  return false;
}

PUBLIC
char const *
Jdb_space::kobject_type() const
{
  return JDB_ANSI_COLOR(red) "Task" JDB_ANSI_COLOR(default);
}

PUBLIC
int
Jdb_space::show_kobject_short(char *buf, int max, Kobject *o)
{
  Task *t = Kobject::dcast<Task*>(o);
  int cnt = 0;
  if (t == Kernel_task::kernel_task())
    {
      cnt = snprintf(buf, max, " {KERNEL}");
      max -= cnt;
      buf += cnt;
    }
  return cnt + snprintf(buf, max, " R=%ld", t->ref_cnt());
}

PRIVATE static
void
Jdb_space::print_space(Space *s)
{
  printf("%p", s);
}

PRIVATE
void
Jdb_space::show(Task *t)
{
  printf("Space %p (Kobject*)%p\n", t, static_cast<Kobject*>(t));
  printf("\n  page table: %p\n", t->mem_space());
  obj_space_info(t);
  io_space_info(t);

  printf("  utcb area: user_va=%lx kernel_va=%lx size=%lx\n",
         t->user_utcb_area(), t->_utcb_kernel_area_start,
         t->utcb_area_size());

  unsigned long m = t->ram_quota()->current();
  unsigned long l = t->ram_quota()->limit();
  printf("  mem usage:  %ld (%ldKB) of %ld (%ldKB) @%p\n", 
         m, m/1024, l, l/1024, t->ram_quota());
}

static bool space_filter(Kobject const *o)
{ return Kobject::dcast<Task const *>(o); }

PUBLIC
Jdb_module::Action_code
Jdb_space::action(int cmd, void *&, char const *&, int &)
{
  if (cmd == 0)
    {
      Jdb_kobject_list list(space_filter);
      list.do_list();
    }
  return NOTHING;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_space::cmds() const
{
  static Cmd cs[] =
    {
	{ 0, "s", "spacelist", "", "s\tshow task list", 0 },
    };
  return cs;
}
  
PUBLIC
int
Jdb_space::num_cmds() const
{ return 1; }

PRIVATE
void
Jdb_space::obj_space_info(Space *s)
{
  printf("  obj_space:  %p\n", s->obj_space());
}

static
bool
filter_task_thread(Kobject const *o)
{
  return Kobject::dcast<Task const *>(o) || Kobject::dcast<Thread_object const *>(o);
}
static Jdb_space jdb_space INIT_PRIORITY(JDB_MODULE_INIT_PRIO);
static Jdb_kobject_list::Mode INIT_PRIORITY(JDB_MODULE_INIT_PRIO) tnt("[Tasks + Threads]", filter_task_thread);

IMPLEMENTATION[!io || ux]:

PRIVATE
void
Jdb_space::io_space_info(Space *)
{}


IMPLEMENTATION[io && !ux]:

PRIVATE
void
Jdb_space::io_space_info(Space *s)
{
  printf("  io_space:   %p\n", s->io_space());
}

