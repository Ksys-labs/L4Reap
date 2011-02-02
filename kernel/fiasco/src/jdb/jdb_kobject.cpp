INTERFACE:

#include "jdb_module.h"
#include "jdb_list.h"
#include "kobject.h"

class Kobject;
class Jdb_kobject_handler;

class Jdb_kobject : public Jdb_module
{
public:
  Jdb_kobject();

private:
  Jdb_kobject_handler *_first;
  Jdb_kobject_handler **_tail;
  static Kobject *kobj;
};


class Jdb_kobject_handler
{
  friend class Jdb_kobject;

public:
  Jdb_kobject_handler(char const *type) : kobj_type(type) {}
  char const *kobj_type;
  virtual bool show_kobject(Kobject_common *o, int level) = 0;
  virtual int show_kobject_short(char *, int, Kobject_common *) { return 0; }
  virtual Kobject_common *follow_link(Kobject_common *o) { return o; }
  virtual ~Jdb_kobject_handler() {}
  virtual bool invoke(Kobject_common *o, Syscall_frame *f, Utcb *utcb);
  virtual bool handle_key(Kobject_common *, int /*keycode*/) { return false; }
  virtual Kobject *parent(Kobject_common *) { return 0; }
  virtual char const *kobject_type() const { return kobj_type; }

protected:
  enum {
    Op_set_name     = 0,
    Op_global_id    = 1,
    Op_kobj_to_id   = 2,
    Op_query_typeid = 3,
    Op_switch_log   = 4,
    Op_get_name     = 5,
  };
private:
  Jdb_kobject_handler *_next;
};

class Jdb_kobject_extension : public Kobject_dbg::Dbg_extension
{
public:
  virtual ~Jdb_kobject_extension() {}
  virtual char const *type() const = 0;
};

class Jdb_kobject_list : public Jdb_list
{
public:
  typedef bool Filter_func(Kobject_common const *);

  struct Mode
  {
    char const *name;
    Filter_func *filter;
    Mode const *next;
    static Mode *first;

    Mode(char const *name, Filter_func *filter)
    : name(name), filter(filter)
    {
      // make sure that non-filtered mode is first in the list so that we
      // get this one displayed initially
      if (!filter || !first)
        {
	  next = first;
	  first = this;
	}
      else
        {
	  next = first->next;
	  first->next = this;
	}
    }
  };

  void *get_head() const
  { return Kobject::from_dbg(Kobject_dbg::_jdb_head.get_unused()); }

private:
  Mode const *_current_mode;

  Filter_func *_filter;
};

//--------------------------------------------------------------------------
IMPLEMENTATION:

#include <climits>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "entry_frame.h"
#include "jdb.h"
#include "jdb_core.h"
#include "jdb_module.h"
#include "jdb_screen.h"
#include "kernel_console.h"
#include "kobject.h"
#include "keycodes.h"
#include "ram_quota.h"
#include "simpleio.h"
#include "space.h"
#include "static_init.h"

Jdb_kobject_list::Mode *Jdb_kobject_list::Mode::first;

class Jdb_kobject_id_hdl : public Jdb_kobject_handler
{
public:
  Jdb_kobject_id_hdl() : Jdb_kobject_handler(0) {}
  virtual bool show_kobject(Kobject_common *, int) { return false; }
  virtual ~Jdb_kobject_id_hdl() {}
};

PUBLIC
bool
Jdb_kobject_id_hdl::invoke(Kobject_common *o, Syscall_frame *f, Utcb *utcb)
{
  if (   utcb->values[0] != Op_global_id
      && utcb->values[0] != Op_kobj_to_id)
    return false;

  if (utcb->values[0] == Op_global_id)
    utcb->values[0] = o->dbg_info()->dbg_id();
  else
    utcb->values[0] = Kobject_dbg::pointer_to_id((void *)utcb->values[1]);
  f->tag(Kobject_iface::commit_result(0, 1));
  return true;
}


PRIVATE
void *
Jdb_kobject_list::get_first()
{
  Kobject *f = static_cast<Kobject*>(get_head());
  while (f && _filter && !_filter(f))
    f = Kobject::from_dbg(f->dbg_info()->_next);
  return f;
}


PUBLIC explicit
Jdb_kobject_list::Jdb_kobject_list(Filter_func *filt)
: Jdb_list(), _current_mode(0), _filter(filt)
{ set_start(get_first()); }

PUBLIC
Jdb_kobject_list::Jdb_kobject_list()
: Jdb_list(), _current_mode(Mode::first), _filter(0)
{
  if (_current_mode)
    _filter = _current_mode->filter;

  set_start(get_first());
}

PUBLIC
int
Jdb_kobject_list::show_item(char *buffer, int max, void *item) const
{
  return Jdb_kobject::obj_description(buffer, max, false, static_cast<Kobject*>(item));
}

PUBLIC
bool
Jdb_kobject_list::enter_item(void *item) const
{
  Kobject *o = static_cast<Kobject*>(item);
  return Jdb_kobject::module()->handle_obj(o, 1);
}

PUBLIC
void *
Jdb_kobject_list::follow_link(void *item)
{
  Kobject *o = static_cast<Kobject*>(item);
  Jdb_kobject_handler *h = Jdb_kobject::module()->find_handler(o);
  if (h)
    return h->follow_link(o);

  return item;
}

PUBLIC
bool
Jdb_kobject_list::handle_key(void *item, int keycode)
{
  Kobject *o = static_cast<Kobject*>(item);
  Jdb_kobject_handler *h = Jdb_kobject::module()->first_global_handler();
  bool handled = false;
  while (h)
    {
      handled |= h->handle_key(o, keycode);
      h = h->next_global();
    }

  h = Jdb_kobject::module()->find_handler(o);
  if (h)
    handled |= h->handle_key(o, keycode);

  return handled;
}

PRIVATE inline NOEXPORT
Kobject *
Jdb_kobject_list::next(Kobject *o)
{
  if (!o)
    return 0;

  do
    {
      o = Kobject::from_dbg(o->dbg_info()->_next);
    }
  while (o && _filter && !_filter(o));
  return o;
}

PRIVATE inline NOEXPORT
Kobject *
Jdb_kobject_list::prev(Kobject *o)
{
  if (!o)
    return 0;

  do
    {
      o = Kobject::from_dbg(o->dbg_info()->_pref);
    }
  while (o && _filter && !_filter(o));
  return o;
}

PUBLIC
int
Jdb_kobject_list::seek(int cnt, void **item)
{
  Kobject *c = static_cast<Kobject*>(*item);
  int i;
  if (cnt > 0)
    {
      for (i = 0; i < cnt; ++i)
	{
	  Kobject *n = next(c);
	  if (!n)
	    break;
	  c = n;
	}
    }
  else if (cnt < 0)
    {
      for (i = 0; i < -cnt; ++i)
	{
	  Kobject *n = prev(c);
	  if (!n)
	    break;
	  c = n;
	}
    }
  else
    return 0;

  if (*item != c)
    {
      *item = c;
      return i;
    }

  return 0;
}

PUBLIC
char const *
Jdb_kobject_list::show_head() const
{
  return "[Objects]";
}


PUBLIC
char const *
Jdb_kobject_list::get_mode_str() const
{
  if (!_current_mode)
    return "[Objects]";
  return _current_mode->name;
}



PUBLIC
void
Jdb_kobject_list::next_mode()
{
  if (!_current_mode)
    return;

  _current_mode = _current_mode->next;
  if (!_current_mode)
    _current_mode = Mode::first;

  _filter = _current_mode->filter;
}

/* When the mode changes the current object may get invisible,
 * get a new visible one */
PUBLIC
void *
Jdb_kobject_list::get_valid(void *o)
{
  if (!_current_mode)
    return o;

  if (_filter && _filter(static_cast<Kobject*>(o)))
    return o;
  return get_first();
}

IMPLEMENT
bool
Jdb_kobject_handler::invoke(Kobject_common *, Syscall_frame *, Utcb *)
{ return false; }

PUBLIC
Jdb_kobject_handler *
Jdb_kobject_handler::next_global()
{
  Jdb_kobject_handler *h = this->_next;
  while (h)
    {
      if (!h->kobj_type)
	return h;
      h = h->_next;
    }
  return 0;
}

Kobject *Jdb_kobject::kobj;

IMPLEMENT
Jdb_kobject::Jdb_kobject()
  : Jdb_module("INFO"),
    _first(0),
    _tail(&_first)
{
}


PUBLIC
void
Jdb_kobject::register_handler(Jdb_kobject_handler *h)
{
  h->_next = 0;
  *_tail = h;
  _tail = &h->_next;
}

PUBLIC inline
Jdb_kobject_handler *
Jdb_kobject::first_global_handler() const
{
  if (!_first || !_first->kobj_type)
    return _first;
  return _first->next_global();
}

PUBLIC
Jdb_kobject_handler *
Jdb_kobject::find_handler(Kobject_common *o)
{
  Jdb_kobject_handler *h = _first;
  while (h)
    {
      if (o->kobj_type() == h->kobj_type)
	return h;
      h = h->_next;
    }
  return 0;
}

PUBLIC
bool
Jdb_kobject::handle_obj(Kobject *o, int lvl)
{
  Jdb_kobject_handler *h = find_handler(o);
  if (h)
    return h->show_kobject(o, lvl);

  return true;
}

PUBLIC static
char const *
Jdb_kobject::kobject_type(Kobject_common *o)
{
  Jdb_kobject_handler *h = module()->find_handler(o);
  if (h)
    return h->kobject_type();
  return o->kobj_type();
}


PUBLIC static
int
Jdb_kobject::obj_description(char *buffer, int max, bool dense, Kobject_common *o)
{
  int pos = snprintf(buffer, max,
                     dense ? "%lx %lx [%-*s]" : "%8lx %08lx [%-*s]",
                     o->dbg_info()->dbg_id(), (Mword)o, 7, kobject_type(o));

  Jdb_kobject_handler *h = Jdb_kobject::module()->first_global_handler();
  while (h)
    {
      pos += h->show_kobject_short(buffer + pos, max-pos, o);
      h = h->next_global();
    }

  Jdb_kobject_handler *oh = Jdb_kobject::module()->find_handler(o);
  if (oh)
    pos += oh->show_kobject_short(buffer + pos, max-pos, o);

  return pos;
}

PRIVATE static
void
Jdb_kobject::print_kobj(Kobject *o)
{
  printf("%p [type=%s]", o, o->kobj_type());
}

PUBLIC
Jdb_module::Action_code
Jdb_kobject::action(int cmd, void *&, char const *&, int &)
{
  if (cmd == 0)
    {
      puts("");
      if (!handle_obj(kobj, 0))
	printf("Kobj w/o handler: ");
      print_kobj(kobj);
      puts("");
      return NOTHING;
    }
  else if (cmd == 1)
    {
      Jdb_kobject_list list;
      list.do_list();
    }
  return NOTHING;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_kobject::cmds() const
{
  static Cmd cs[] =
    {
	{ 0, "K", "kobj", "%p", "K<kobj_ptr>\tshow information for kernel object", 
	  &kobj },
	{ 1, "Q", "listkobj", "", "Q\tshow information for kernel objects", 0 },
    };
  return cs;
}

PUBLIC
int
Jdb_kobject::num_cmds() const
{ return 2; }

STATIC_INITIALIZE_P(Jdb_kobject, JDB_MODULE_INIT_PRIO);

PRIVATE static
int
Jdb_kobject::fmt_handler(char /*fmt*/, int *size, char const *cmd_str, void *arg)
{
  char buffer[20];

  int pos = 0;
  int c;
  Address n;

  *size = sizeof(void*);

  while((c = Jdb_core::cmd_getchar(cmd_str)) != ' ' && c!=KEY_RETURN)
    {
      if(c==KEY_ESC)
	return 3;

      if(c==KEY_BACKSPACE && pos>0)
	{
	  putstr("\b \b");
	  --pos;
	}

      if (pos < (int)sizeof(buffer) - 1)
	{
	  putchar(c);
	  buffer[pos++] = c;
	  buffer[pos] = 0;
	}
    }

  void **a = (void**)arg;

  if (!pos)
    {
      *a = 0;
      return 0;
    }

  char const *num = buffer;
  if (buffer[0] == 'P')
    num = buffer + 1;

  n = strtoul(num, 0, 16);

  Kobject *ko;

  if (buffer[0] != 'P')
    ko = Kobject::id_to_obj(n);
  else
    ko = Kobject::pointer_to_obj((void*)n);

  *a = ko;

  return 0;
}

PUBLIC static
void
Jdb_kobject::init()
{
  module();

  Jdb_core::add_fmt_handler('q', fmt_handler);

//  static Jdb_handler enter(at_jdb_enter);

  static Jdb_kobject_id_hdl id_hdl;
  module()->register_handler(&id_hdl);
}

PUBLIC static
Jdb_kobject *
Jdb_kobject::module()
{
  static Jdb_kobject jdb_kobj_module;
  return &jdb_kobj_module;
}

// Be robust if this object is invalid
PUBLIC static
void
Jdb_kobject::print_uid(Kobject_common *o, int task_format = 0)
{
  if (!o)
    {
      printf("%*.s", task_format, "---");
      return;
    }

  if (Kobject_dbg::is_kobj(o))
    {
      printf("%*.lx", task_format, o->dbg_info()->dbg_id());
      return;
    }

  printf("\033[31;1m%*s%p\033[m", task_format, "???", o);
  return;
}


extern "C" void
sys_invoke_debug(Kobject_iface *o, Syscall_frame *f)
{
  if (!o)
    {
      f->tag(Kobject_iface::commit_result(-L4_err::EInval));
      return;
    }

  Utcb *utcb = current_thread()->utcb().access();
  //printf("sys_invoke_debug: [%p] -> %p\n", o, f);
  Jdb_kobject_handler *h = Jdb_kobject::module()->find_handler(o);
  if (h && h->invoke(o, f, utcb))
    return;

  h = Jdb_kobject::module()->first_global_handler();
  while (h)
    {
      if (h->invoke(o, f, utcb))
	return;
      h = h->next_global();
    }

  f->tag(Kobject_iface::commit_result(-L4_err::ENosys));
}

PUBLIC
template< typename T >
static
T *
Jdb_kobject_extension::find_extension(Kobject_common const *o)
{
  Kobject_dbg::Dbg_extension *ex = o->dbg_info()->_jdb_data;
  while (ex)
    {
      Jdb_kobject_extension *je = static_cast<Jdb_kobject_extension*>(ex);
      if (je->type() == T::static_type)
	return static_cast<T*>(je);

      ex = ex->next();
    }

  return 0;
}

static Jdb_kobject_list::Mode INIT_PRIORITY(JDB_MODULE_INIT_PRIO) all("[ALL]", 0);


