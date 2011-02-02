IMPLEMENTATION:

#include <cstdio>
#include <cstring>

#include "config.h"
#include "globals.h"
#include "ipc_timeout.h"
#include "jdb.h"
#include "jdb_kobject.h"
#include "jdb_kobject_names.h"
#include "jdb_module.h"
#include "jdb_screen.h"
#include "kernel_console.h"
#include "keycodes.h"
#include "kmem.h"
#include "simpleio.h"
#include "static_init.h"
#include "timeout.h"
#include "timeslice_timeout.h"
#include "thread.h"

class Jdb_list_timeouts : public Jdb_module
{
public:
  Jdb_list_timeouts() FIASCO_INIT;
private:
  enum
    {
      Timeout_ipc		= 1,
      Timeout_deadline		= 2,
      Timeout_timeslice		= 3,
      Timeout_root		= 4,
    };
};

class Jdb_timeout_list
{
private:
  static int	_count;
  static Timeout *_t_start;
};


// available from the jdb_tcb module
extern int jdb_show_tcb(Thread *thread, int level) __attribute__((weak));

int      Jdb_timeout_list::_count;
Timeout *Jdb_timeout_list::_t_start;

PUBLIC static
void
Jdb_timeout_list::init(Timeout *t_head)
{
  _t_start = t_head;
}

static
bool
Jdb_timeout_list::iter(int count, Timeout **t_start,
		       void (*iter)(Timeout *t) = 0)
{
  int i = 0;
  int forw = count >= 0;
  Timeout *t_new = *t_start;

  if (count == 0)
    return false;

  if (count < 0)
    count = -count;

  if (iter)
    iter(t_new);

  for (; count; count--)
    {
      if (forw)
	{
	  if (t_new->_next == Timeout_q::timeout_queue.cpu(0).first())
	    break;
	  t_new = t_new->_next;
	}
      else
	{
	  if (t_new->_prev == Timeout_q::timeout_queue.cpu(0).first())
	    break;
	  t_new = t_new->_prev;
	}

      if (iter)
	iter(t_new);

      i++;
    };

  _count = i;
  bool changed = *t_start != t_new;
  *t_start = t_new;

  return changed;
}

// _t_start-- if possible
PUBLIC static
int
Jdb_timeout_list::line_back(void)
{
  return _t_start ? iter(-1, &_t_start) : 0;
}

// _t_start++ if possible
PUBLIC static
int
Jdb_timeout_list::line_forw(void)
{
  Timeout *t = _t_start;
  if (t)
    {
      iter(+Jdb_screen::height()-2, &_t_start);
      iter(-Jdb_screen::height()+3, &_t_start);
    }
  return t != _t_start;
}

// _t_start -= 24 if possible
PUBLIC static
int
Jdb_timeout_list::page_back(void)
{
  return _t_start ? iter(-Jdb_screen::height()+2, &_t_start) : 0;
}

// _t_start += 24 if possible
PUBLIC static
int
Jdb_timeout_list::page_forw(void)
{
  Timeout *t = _t_start;
  if (t)
    {
      iter(+Jdb_screen::height()*2-5, &_t_start);
      iter(-Jdb_screen::height()+3,   &_t_start);
    }
  return t != _t_start;
}

// _t_start = first element of list
PUBLIC static
int
Jdb_timeout_list::goto_home(void)
{
  return _t_start ? iter(-9999, &_t_start) : 0;
}

// _t_start = last element of list
PUBLIC static
int
Jdb_timeout_list::goto_end(void)
{
  Timeout *t = _t_start;
  if (t)
    {
      iter(+9999, &_t_start);
      iter(-Jdb_screen::height()+2, &_t_start);
    }
  return t != _t_start;
}

PUBLIC static
int
Jdb_timeout_list::lookup(Timeout *t_search)
{
  unsigned i;
  Timeout *t;

  for (i=0, t=_t_start; i<Jdb_screen::height()-3; i++)
    {
      if (t == t_search)
	break;
      iter(+1, &t);
    }

  return i;
}

PUBLIC static
Timeout*
Jdb_timeout_list::index(int y)
{
  Timeout *t = _t_start;

  if (!t)
    return 0;

  iter(y, &t);
  return t;
}

PUBLIC static
int
Jdb_timeout_list::page_show(void (*show)(Timeout *t))
{
  Timeout *t = _t_start;

  iter(Jdb_screen::height()-3, &t, show);
  return _count;
}

// use implicit knowledge to determine the type of a timeout because we
// cannot use dynamic_cast (we compile with -fno-rtti)
static
int
Jdb_list_timeouts::get_type(Timeout *t)
{
  Address addr = (Address)t;

  if (t == timeslice_timeout.cpu(0))
    // there is only one global timeslice timeout
    return Timeout_timeslice;

  if (Timeout_q::timeout_queue.cpu(0).is_root_node(addr))
    return Timeout_root;

  if ((addr % Config::thread_block_size) >= sizeof(Thread))
    // IPC timeouts are located at the kernel stack
    return Timeout_ipc;

  // unknown
  return 0;
}

static
Thread*
Jdb_list_timeouts::get_owner(Timeout *t)
{
  switch (get_type(t))
    {
      case Timeout_ipc:
        return static_cast<Thread*>(context_of(t));
      case Timeout_deadline:
        return static_cast<Thread*>(context_of(t));
      case Timeout_timeslice:
	return kernel_thread;
	// XXX: current_sched does not work from the debugger
        if (Context::current_sched())
          return static_cast<Thread*>(Context::current_sched()->context());
      default:
        return 0;
    }
}

static
void
Jdb_list_timeouts::show_header()
{
  Jdb::cursor();
  printf("%s  type           timeout    owner       name\033[m\033[K\n",
         Jdb::esc_emph);
}

static
void
Jdb_list_timeouts::list_timeouts_show_timeout(Timeout *t)
{
  char const *type;
  char ownerstr[32] = "";
  Thread *owner;
  Signed64 timeout = t->get_timeout(Kip::k()->clock);

  Kconsole::console()->getchar_chance();

  switch (get_type(t))
    {
    case Timeout_ipc:
      type  = "ipc";
      owner = get_owner(t);
      snprintf(ownerstr, sizeof(ownerstr), "  %p", owner);
      break;
    case Timeout_deadline:
      type  = "deadline";
      owner = get_owner(t);
      snprintf(ownerstr, sizeof(ownerstr), "  %p", owner);
      break;
    case Timeout_timeslice:
      type  = "timeslice";
      owner = get_owner(t);
      if (owner)
        snprintf(ownerstr, sizeof(ownerstr), "  %p", owner);
      else
       strcpy (ownerstr, "destruct");
      break;
    case Timeout_root:
      type  = "root";
      owner = 0;
      strcpy(ownerstr, "kern");
      break;
    default:
      snprintf(ownerstr, sizeof(ownerstr), L4_PTR_FMT, (Address)t);
      type  = "???";
      owner = 0;
      break;
    }

  printf("  %-10s   ", type);
  if (timeout < 0)
    putstr("   over     ");
  else
    {
      char time_str[12];
      Jdb::write_ll_ns(timeout * 1000, time_str,
	               11 < sizeof(time_str) - 1 ? 11 : sizeof(time_str) - 1,
                       false);
      putstr(time_str);
    }

  Jdb_kobject_name *nx = 0;

  if (owner)
    nx = Jdb_kobject_extension::find_extension<Jdb_kobject_name>(owner);

  printf(" %s  %s\033[K\n", ownerstr, nx ? nx->name() : "");
}

IMPLEMENT
Jdb_list_timeouts::Jdb_list_timeouts()
  : Jdb_module("INFO")
{}

static
void
Jdb_list_timeouts::list()
{
  unsigned y, y_max;
  Timeout *t_current = Timeout_q::timeout_queue.cpu(0).first();

  Jdb::clear_screen();
  show_header();
  Jdb_timeout_list::init(t_current);

  for (;;)
    {
      y = Jdb_timeout_list::lookup(t_current);

      for (bool resync=false; !resync; )
	{
	  Jdb::cursor(2, 1);
	  y_max = t_current
		      ? Jdb_timeout_list::page_show(list_timeouts_show_timeout)
		      : 0;

	  for (unsigned i=y_max; i<Jdb_screen::height()-3; i++)
	    putstr("\033[K\n");

	  Jdb::printf_statline("timouts", "<CR>=select owner", "_");

	  for (bool redraw=false; !redraw; )
	    {
	      Jdb::cursor(y+2, 1);
	      switch (int c=Jdb_core::getchar())
		{
		case KEY_CURSOR_UP:
		  if (y > 0)
		    y--;
		  else
		    redraw = Jdb_timeout_list::line_back();
		  break;
		case KEY_CURSOR_DOWN:
		  if (y < y_max)
		    y++;
		  else
		    redraw = Jdb_timeout_list::line_forw();
		  break;
		case KEY_PAGE_UP:
		  if (!(redraw = Jdb_timeout_list::page_back()))
		    y = 0;
		  break;
		case KEY_PAGE_DOWN:
		  if (!(redraw = Jdb_timeout_list::page_forw()))
		    y = y_max;
		  break;
		case KEY_CURSOR_HOME:
		  redraw = Jdb_timeout_list::goto_home();
		  y = 0;
		  break;
		case KEY_CURSOR_END:
		  redraw = Jdb_timeout_list::goto_end();
		  y = y_max;
		  break;
		case KEY_RETURN:
		  if (jdb_show_tcb != 0)
		    {
		      Thread *owner;
		      Timeout *t = Jdb_timeout_list::index(y);
		      if (t && (owner = get_owner(t)))
			{
			  if (!jdb_show_tcb(owner, 1))
			    return;
			  show_header();
			  redraw = 1;
			}
		    }
		  break;
		case KEY_ESC:
		  Jdb::abort_command();
		  return;
		default:
		  if (Jdb::is_toplevel_cmd(c))
		    return;
		}
	    }
	}
    }
}

PUBLIC
Jdb_module::Action_code
Jdb_list_timeouts::action(int cmd, void *&, char const *&, int &)
{
  if (cmd == 0)
    list();

  return NOTHING;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_list_timeouts::cmds() const
{
  static Cmd cs[] =
    {
	{ 0, "lt", "timeouts", "", "lt\tshow enqueued timeouts", 0 },
    };

  return cs;
}

PUBLIC
int
Jdb_list_timeouts::num_cmds() const
{
  return 1;
}

static Jdb_list_timeouts jdb_list_timeouts INIT_PRIORITY(JDB_MODULE_INIT_PRIO);
