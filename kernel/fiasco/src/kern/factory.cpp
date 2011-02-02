INTERFACE:

#include "ram_quota.h"
#include "kobject_helper.h"

class Factory : public Ram_quota, public Kobject_h<Factory>
{
  FIASCO_DECLARE_KOBJ();

private:
  typedef slab_cache_anon Self_alloc;
};

//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "ipc_gate.h"
#include "kmem_slab.h"
#include "task.h"
#include "thread_object.h"
#include "static_init.h"
#include "l4_buf_iter.h"
#include "l4_types.h"
#include "u_semaphore.h"
#include "irq.h"
#include "map_util.h"
#include "logdefs.h"
#include "entry_frame.h"

static Factory _root_factory INIT_PRIORITY(ROOT_FACTORY_INIT_PRIO);
FIASCO_DEFINE_KOBJ(Factory);

PUBLIC inline
Factory::Factory()
  : Ram_quota()
{}

PRIVATE inline
Factory::Factory(Ram_quota *q, unsigned long max)
  : Ram_quota(q, max)
{}


static Kmem_slab_t<Factory> _factory_allocator("Factory");

PRIVATE static
Factory::Self_alloc *
Factory::allocator()
{ return &_factory_allocator; }

PUBLIC static inline
Factory *
Factory::root()
{ return static_cast<Factory*>(Ram_quota::root); }


PRIVATE
Factory *
Factory::create_factory(unsigned long max)
{
  void *nq;
  if (alloc(sizeof(Factory) + max) && (nq = allocator()->alloc()))
    return new (nq) Factory(this, max);

  return 0;
}

PUBLIC
void Factory::operator delete (void *_f)
{
  Factory *f = (Factory*)_f;
  LOG_TRACE("Factory delete", "fa del", ::current(), 0, {});

  if (!f->parent())
    return;

  Ram_quota *p = f->parent();
  if (p)
    p->free(sizeof(Factory) + f->limit());

  allocator()->free(f);
}

PRIVATE
L4_msg_tag
Factory::map_obj(Kobject_iface *o, Mword cap, Space *c_space,
                 Obj_space *o_space)
{
  switch (Mword(o))
    {
    case 0:               return commit_result(-L4_err::ENomem);
    case -L4_err::EInval: return commit_result(-L4_err::EInval);
    case -L4_err::EPerm:  return commit_result(-L4_err::EPerm);
    default:              break;
    };

  Reap_list rl;

  if (!map(o, o_space, c_space, cap, rl.list()))
    {
      delete o;
      // FIXME: reap stuff if needed
      return commit_result(-L4_err::ENomem);
    }

  // FIXME: reap stuff if needed
  return commit_result(0, 0, 1);
}



PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_factory(Utcb const *u)
{
  // XXX: should check for type tag in new call
  return create_factory(u->values[2]);
}


PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_task(Utcb const *u)
{
  L4_fpage utcb_area(0);
  // XXX: should check for type tag in new call
  utcb_area = L4_fpage(u->values[2]);

  Task *new_t = Task::create(Space::Default_factory(), this, utcb_area);

  if (!new_t)
    return 0;

  if (!new_t->valid() || !new_t->initialize())
    {
      delete new_t;
      return 0;
    }

  return new_t;
}

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_thread(Utcb const *)
{
  Thread_object *t = new (this) Thread_object();
  return t;
}

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_gate(L4_msg_tag const &tag, Utcb const *utcb, Obj_space *o_space)
{
  L4_snd_item_iter snd_items(utcb, tag.words());
  Thread *thread = 0;

  if (tag.items() && snd_items.next())
    {
      L4_fpage bind_thread(snd_items.get()->d);
      if (EXPECT_FALSE(!bind_thread.is_objpage()))
	return reinterpret_cast<Kobject_iface*>(-L4_err::EInval);

      unsigned char thread_rights = 0;
      thread = Kobject::dcast<Thread_object*>(o_space->lookup_local(bind_thread.obj_index(), &thread_rights));

      if (EXPECT_FALSE(!(thread_rights & L4_fpage::W)))
	return reinterpret_cast<Kobject_iface*>(-L4_err::EPerm);
    }
#if 0
  if (!thread)
    return reinterpret_cast<Kobject_iface*>(-L4_err::EInval);
#endif
  // should check type tag of varg
  return static_cast<Ipc_gate_ctl*>(Ipc_gate::create(this, thread, utcb->values[2]));
}


PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_semaphore(Utcb const *)
{
  return U_semaphore::alloc(this);
}

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_irq(unsigned w, Utcb const *utcb)
{
  if (w >= 3 && utcb->values[2])
    return Irq::allocate<Irq_muxer>(this);
  else
    return Irq::allocate<Irq_sender>(this);
}

PUBLIC
L4_msg_tag
Factory::kinvoke(L4_obj_ref ref, Mword rights, Syscall_frame *f,
                 Utcb const *utcb, Utcb *)
{
  register Context *const c_thread = ::current();
  register Space *const c_space = c_thread->space();
  register Obj_space *const o_space = c_space->obj_space();

  if (EXPECT_FALSE(f->tag().proto() != L4_msg_tag::Label_factory))
    return commit_result(-L4_err::EBadproto);

  if (EXPECT_FALSE(!(rights & L4_fpage::W)))
    return commit_result(-L4_err::EPerm);

  if (EXPECT_FALSE(!ref.have_recv()))
    return commit_result(0);

  if (f->tag().words() < 1)
    return commit_result(-L4_err::EInval);

  L4_fpage buffer(0);

    {
      L4_buf_iter buf(utcb, utcb->buf_desc.obj());
      L4_buf_iter::Item const *const b = buf.get();
      if (EXPECT_FALSE(b->b.is_void()
	    || b->b.type() != L4_msg_item::Map))
	return commit_error(utcb, L4_error(L4_error::Overflow, L4_error::Rcv));

      buffer = L4_fpage(b->d);
    }

  if (EXPECT_FALSE(!buffer.is_objpage()))
    return commit_error(utcb, L4_error(L4_error::Overflow, L4_error::Rcv));

  Kobject_iface *new_o;

  switch ((long)utcb->values[0])
    {
    case 0:  // new IPC Gate
      new_o = new_gate(f->tag(), utcb, o_space);
      break;

    case L4_msg_tag::Label_factory:
      new_o = new_factory(utcb);
      break;

    case L4_msg_tag::Label_task:
      new_o =  new_task(utcb);
      break;

    case L4_msg_tag::Label_thread:
      new_o = new_thread(utcb);
      break;

    case L4_msg_tag::Label_semaphore:
      new_o = new_semaphore(utcb);
      break;

    case L4_msg_tag::Label_irq:
      new_o = new_irq(f->tag().words(), utcb);
      break;

    case L4_msg_tag::Label_vm:
      new_o = new_vm(utcb);
      break;

    default:
      return commit_result(-L4_err::ENodev);
    }

  LOG_TRACE("Kobject create", "new", ::current(), __factory_log_fmt,
    Log_entry *le = tbe->payload<Log_entry>();
    le->op = utcb->values[0];
    le->buffer = buffer.obj_index();
    le->id = dbg_info()->dbg_id();
    le->ram = current();
    le->newo = new_o ? new_o->dbg_info()->dbg_id() : ~0);

  return map_obj(new_o, buffer.obj_index(), c_space, o_space);

}


//----------------------------------------------------------------------------
IMPLEMENTATION [svm || vmx]:

#include "vm_factory.h"

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_vm(Utcb const *)
{
  Vm *new_t = Vm_factory::create(this);

  if (!new_t)
    return 0;

  if (!new_t->valid() || !new_t->initialize())
    {
      delete new_t;
      return 0;
    }

  return new_t;
}

IMPLEMENTATION [tz]:

#include "vm.h"

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_vm(Utcb const *)
{
  return Vm::create(this);
}

//----------------------------------------------------------------------------
IMPLEMENTATION [!svm && !tz && !vmx]:

PRIVATE inline NOEXPORT
Kobject_iface *
Factory::new_vm(Utcb const *)
{ return (Kobject_iface*)(-L4_err::EInval); }

// ------------------------------------------------------------------------
INTERFACE [debug]:

#include "tb_entry.h"

EXTENSION class Factory
{
private:
  struct Log_entry
  {
    Smword op;
    Mword buffer;
    Mword id;
    Mword ram;
    Mword newo;
  };
  static unsigned log_fmt(Tb_entry *, int, char *) asm ("__factory_log_fmt");
};

// ------------------------------------------------------------------------
IMPLEMENTATION [debug]:

IMPLEMENT
unsigned
Factory::log_fmt(Tb_entry *e, int maxlen, char *buf)
{
  static char const *const ops[] =
  { /*   0 */ "gate", "irq", 0, 0, 0, 0, 0, 0,
    /*  -8 */ 0, 0, 0, "task", "thread", 0, 0, "factory",
    /* -16 */ "vm", 0, 0, 0, "sem" }; 
  Log_entry *le = e->payload<Log_entry>();
  char const *op = -le->op <= (int)(sizeof(ops)/sizeof(ops[0]))
    ? ops[-le->op] : "invalid op";
  if (!op)
    op = "(nan)";

  return snprintf(buf, maxlen, "factory=%lx [%s] new=%lx cap=[C:%lx] ram=%lx", le->id, op, le->newo, le->buffer, le->ram);
}
