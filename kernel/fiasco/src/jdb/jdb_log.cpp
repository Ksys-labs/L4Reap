IMPLEMENTATION:

#include <climits>
#include <cstring>
#include <cstdio>

#include "jdb.h"
#include "jdb_core.h"
#include "jdb_module.h"
#include "jdb_list.h"
#include "jdb_screen.h"
#include "kernel_console.h"
#include "keycodes.h"
#include "mem_unit.h"
#include "ram_quota.h"
#include "simpleio.h"
#include "task.h"
#include "static_init.h"


class Jdb_log_list : public Jdb_list
{
public:
  void *get_head() const { return _log_table; }
  char const *show_head() const { return "[Log]"; }

private:
  static Tb_log_table_entry *_end;
};

Tb_log_table_entry *Jdb_log_list::_end;

PUBLIC
int
Jdb_log_list::show_item(char *buffer, int max, void *item) const
{
  Tb_log_table_entry const *e = static_cast<Tb_log_table_entry const*>(item);
  char const *sc = e->name;
  sc += strlen(e->name) + 1;
  int pos = snprintf(buffer, max, "%s %s (%s)",
                     *(e->patch) ? "[on ]" : "[off]",  e->name, sc);
  return pos;
}

PUBLIC
bool
Jdb_log_list::enter_item(void *item) const
{
  Tb_log_table_entry const *e = static_cast<Tb_log_table_entry const*>(item);
  patch_item(e, *(e->patch) ? 0 : (e - _log_table) + Tbuf_dynentries);
  return true;
}

PRIVATE static
void
Jdb_log_list::patch_item(Tb_log_table_entry const *e, unsigned char val)
{
  if (e->patch)
    {
      *(e->patch) = val;
      Mem_unit::clean_dcache(e->patch);
    }

  for (Tb_log_table_entry *x = _end; x < _log_table_end; ++x)
    {
      if (equal(x, e) && x->patch)
        {
          *(x->patch) = val;
          Mem_unit::clean_dcache(x->patch);
        }
    }
}

PRIVATE static
bool
Jdb_log_list::equal(Tb_log_table_entry const *a, Tb_log_table_entry const *b)
{
  if (strcmp(a->name, b->name))
    return false;

  char const *sca = a->name; sca += strlen(sca) + 1;
  char const *scb = b->name; scb += strlen(scb) + 1;

  if (strcmp(sca, scb))
    return false;

  return a->fmt == b->fmt;
}

PRIVATE
bool
Jdb_log_list::next(void **item)
{
  Tb_log_table_entry *e = static_cast<Tb_log_table_entry*>(*item);

  while (e + 1 < _log_table_end)
    {
#if 0
      if (equal(e, e+1))
	++e;
      else
#endif
	{
	  *item  = e+1;
	  return true;
	}
    }

  return false;
}

PRIVATE
bool
Jdb_log_list::pref(void **item)
{
  Tb_log_table_entry *e = static_cast<Tb_log_table_entry*>(*item);

  if (e > _log_table)
    --e;
  else
    return false;
#if 0
  while (e > _log_table)
    {
      if (equal(e, e-1))
	--e;
      else
	break;
    }
#endif

  *item  = e;
  return true;
}

PUBLIC
int
Jdb_log_list::seek(int cnt, void **item)
{
  Tb_log_table_entry *e = static_cast<Tb_log_table_entry*>(*item);
  if (cnt > 0)
    {
      if (e + cnt >= _end)
	cnt = _end - e - 1;
    }
  else if (cnt < 0)
    {
      if (e + cnt < _log_table)
	cnt = _log_table - e;
    }

  if (cnt)
    {
      *item = e + cnt;
      return cnt;
    }

  return 0;
}

class Jdb_log : public Jdb_module
{
public:
  Jdb_log() FIASCO_INIT;
private:
};


static void swap(Tb_log_table_entry *a, Tb_log_table_entry *b)
{
  Tb_log_table_entry x = *a;
  *a = *b;
  *b = x;
}

static bool lt_cmp(Tb_log_table_entry *a, Tb_log_table_entry *b)
{
  if (strcmp(a->name, b->name) < 0)
    return true;
  else
    return false;
}

static void sort_tb_log_table()
{
  for (Tb_log_table_entry *p = _log_table; p < _log_table_end; ++p)
    {
      for (Tb_log_table_entry *x = _log_table_end -1; x > p; --x)
	if (lt_cmp(x, x - 1))
	  swap(x - 1, x);
    }
}

PUBLIC
static
void
Jdb_log_list::move_dups()
{
  _end = _log_table_end;
  for (Tb_log_table_entry *p = _log_table + 1; p < _end;)
    {
      if (equal(p-1, p))
	{
	  --_end;
	  if (p < _end)
	    {
	      Tb_log_table_entry tmp = *p;
	      memmove(p, p + 1, sizeof(Tb_log_table_entry) * (_log_table_end - p - 1));
	      *(_log_table_end - 1) = tmp;
	    }
	  else
	    break;
	}
      else
	++p;
    }
}

#if 0
static void disable_all()
{
  for (Tb_log_table_entry *p = _log_table; p < _log_table_end; ++p)
    *(p->patch) = 0;
}
#endif


IMPLEMENT
Jdb_log::Jdb_log()
  : Jdb_module("MONITORING")
{
  //disable_all();
  sort_tb_log_table();
  Jdb_log_list::move_dups();
}

PUBLIC
Jdb_module::Action_code
Jdb_log::action(int, void *&, char const *&, int &)
{
  if (_log_table >= _log_table_end)
    return NOTHING;

  Jdb_log_list list;
  list.set_start(_log_table);
  list.do_list();

  return NOTHING;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_log::cmds() const
{
  static Cmd cs[] =
    {
	{ 0, "O", "log", "", "O\tselect log events", 0 },
    };
  return cs;
}

PUBLIC
int
Jdb_log::num_cmds() const
{ return 1; }

static Jdb_log jdb_log INIT_PRIORITY(JDB_MODULE_INIT_PRIO);

