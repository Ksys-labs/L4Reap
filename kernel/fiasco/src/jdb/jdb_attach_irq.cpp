IMPLEMENTATION:

#include <cstdio>

#include "irq_chip.h"
#include "irq.h"
#include "jdb_module.h"
#include "kernel_console.h"
#include "static_init.h"
#include "thread.h"
#include "types.h"


//===================
// Std JDB modules
//===================

/**
 * 'IRQ' module.
 * 
 * This module handles the 'R' command that
 * provides IRQ attachment and listing functions.
 */
class Jdb_attach_irq : public Jdb_module
{
public:
  Jdb_attach_irq() FIASCO_INIT;
private:
  static char     subcmd;
  static unsigned irq;
};

char     Jdb_attach_irq::subcmd;
unsigned Jdb_attach_irq::irq;
static Jdb_attach_irq jdb_attach_irq INIT_PRIORITY(JDB_MODULE_INIT_PRIO);

IMPLEMENT
Jdb_attach_irq::Jdb_attach_irq()
  : Jdb_module("INFO")
{}

PUBLIC
Jdb_module::Action_code
Jdb_attach_irq::action( int cmd, void *&args, char const *&fmt, int & )
{
  if (cmd!=0)
    return NOTHING;

  if ((char*)args == &subcmd)
    {
      switch(subcmd) 
	{
	case 'a': // attach
	  args = &irq;
	  fmt = " irq: %i\n";
	  return EXTRA_INPUT;
	  
	case 'l': // list
  	    {
  	      Irq *r;
	      putchar('\n');
  	      for (unsigned i = 0; i < Config::Max_num_irqs; ++i)
		{
		  r = static_cast<Irq*>(Irq_chip::hw_chip->irq(i));
		  if (!r)
		    continue;
  		  printf("IRQ %02x/%02d\n", i, i);
		}
	    }
	  return NOTHING;
	}
    }
  else if (args == (void*)&irq)
    {
      Irq *i = static_cast<Irq*>(Irq_chip::hw_chip->irq(irq));
      if (i)
	i->alloc((Receiver*)-1);
    }
  return NOTHING;
}

PUBLIC
int
Jdb_attach_irq::num_cmds() const
{
  return 1;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_attach_irq::cmds() const
{
  static Cmd cs[] =
    {   { 0, "R", "irq", " [l]ist/[a]ttach: %c",
	   "R{l|a<num>}\tlist IRQ threads, attach Jdb to IRQ", &subcmd }
    };

  return cs;
}


IMPLEMENTATION:

#include "jdb_kobject.h"
#include "irq.h"

class Jdb_kobject_irq : public Jdb_kobject_handler
{
};

PUBLIC inline
Jdb_kobject_irq::Jdb_kobject_irq()
  : Jdb_kobject_handler(Irq::static_kobj_type)
{
  Jdb_kobject::module()->register_handler(this);
}

PUBLIC
char const *
Jdb_kobject_irq::kobject_type() const
{
  return JDB_ANSI_COLOR(white) "IRQ" JDB_ANSI_COLOR(default);
}

PUBLIC
Kobject *
Jdb_kobject_irq::follow_link(Kobject *o)
{
  Irq *t = Kobject::dcast<Irq*>(o);
  if (!t->owner() || (Smword)t->owner() == -1)
    return o;

  return static_cast<Thread*>(t->owner())->kobject();
}

PUBLIC
bool
Jdb_kobject_irq::show_kobject(Kobject *, int)
{ return true; }
#if 0
  Thread *t = Kobject::dcast<Thread*>(o);
  return show(t, level);
#endif

PUBLIC
int
Jdb_kobject_irq::show_kobject_short(char *buf, int max, Kobject *o)
{
  Irq *t = Kobject::dcast<Irq*>(o);
  Kobject *d = follow_link(o);
  int cnt = 0;
  return cnt + snprintf(buf, max, " I=%3lx %s L=%lx T=%lx F=%x",
                        t->irq(), t->pin()->pin_type(), t->obj_id(),
                        d ? d->dbg_id() : 0, (unsigned)t->pin()->flags());
}

static
bool
filter_irqs(Kobject const *o)
{ return Kobject::dcast<Irq const *>(o); }

static Jdb_kobject_list::Mode INIT_PRIORITY(JDB_MODULE_INIT_PRIO) tnt("[IRQs]", filter_irqs);

static Jdb_kobject_irq jdb_kobject_irq INIT_PRIORITY(JDB_MODULE_INIT_PRIO);
