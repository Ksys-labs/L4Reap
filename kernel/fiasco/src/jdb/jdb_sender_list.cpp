IMPLEMENTATION:

#include <cstdio>
#include <cstring>

#include "thread_object.h"
#include "jdb.h"
#include "jdb_kobject.h"
#include "jdb_module.h"


class Jdb_sender_list : public Jdb_module, public Jdb_kobject_handler
{
public:
  Jdb_sender_list() FIASCO_INIT;

  virtual bool show_kobject(Kobject_common *, int) { return true; }
private:
  static Kobject *thread;
};

static Jdb_sender_list jdb_sender_list INIT_PRIORITY(JDB_MODULE_INIT_PRIO);

Kobject *Jdb_sender_list::thread;


PRIVATE
void
Jdb_sender_list::show_sender_list(Thread *t, int printlines)
{
  puts(printlines ? Jdb_screen::Line : "");
  Jdb::clear_to_eol();
  printf("Thread: %lx\n", t->dbg_info()->dbg_id());

  Prio_list_elem *p = t->sender_list()->head();
  if (!p)
    {
      Jdb::clear_to_eol();
      printf("Nothing in sender list\n");
      if (printlines)
        puts(Jdb_screen::Line);
      return;
    }

  while (p)
    {
      Jdb::clear_to_eol();
      printf("%02x: ", p->prio());
      Prio_list_elem *s = p;
      do
        {
          Thread *ts = static_cast<Thread *>(Sender::cast(s));
          printf("%s %lx", s == p ? "" : ",", ts->dbg_info()->dbg_id());
          s = s->_s_next;
        } while (s != p);
      puts("");
      p = p->_p_next;
    }

  if (printlines)
    puts(Jdb_screen::Line);
}

PUBLIC
Jdb_module::Action_code
Jdb_sender_list::action(int cmd, void *&, char const *&, int &)
{
  if (cmd)
    return NOTHING;

  Thread *t = Kobject::dcast<Thread_object *>(thread);
  if (!t)
    {
      printf(" Invalid thread\n");
      return NOTHING;
    }

  show_sender_list(t, 0);

  return NOTHING;
}

PUBLIC
bool
Jdb_sender_list::handle_key(Kobject_common *o, int keycode)
{
  if (keycode != 'S')
    return false;

  Thread *t = Kobject::dcast<Thread_object *>(o);
  if (!t)
    return false;

  show_sender_list(t, 1);
  Jdb::getchar();
  return true;
}

PUBLIC
int Jdb_sender_list::num_cmds() const
{ return 1; }

PUBLIC
Jdb_module::Cmd const * Jdb_sender_list::cmds() const
{
  static Cmd cs[] =
    {
	{ 0, "ls", "senderlist", "%q",
          "senderlist\tshow sender-list of thread", &thread }
    };

  return cs;
}

IMPLEMENT
Jdb_sender_list::Jdb_sender_list()
  : Jdb_module("INFO"), Jdb_kobject_handler(0)
{
  Jdb_kobject::module()->register_handler(this);
}
