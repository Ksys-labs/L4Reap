INTERFACE:

#include "lock.h"
#include "obj_space.h"


class Kobject_mappable
{
private:
  friend class Kobject_mapdb;
  friend class Jdb_mapdb;
  Obj::Mapping *_root;
  Smword _cnt;
  Lock _lock;

public:
  Kobject_mappable() : _root(0), _cnt(0) {}
  bool no_mappings() const { return !_root; }

  /**
   * Insert the root mapping of an object.
   */
  template<typename M>
  bool insert(void *, Space *, M &m)
  {
    m._c->put_as_root(&_root);
    _cnt = 1;
    return true;
  }

  Smword cap_ref_cnt() const { return _cnt; }
};


//----------------------------------------------------------------------------
INTERFACE:

#include "context.h"
#include "irq_pin.h"
#include "kobject_dbg.h"
#include "kobject_iface.h"
#include "l4_error.h"
#include "rcupdate.h"
#include "space.h"

class Kobject :
  public virtual Kobject_common,
  private Kobject_mappable,
  public Kobject_dbg
{
  template<typename T>
  friend class Map_traits;

public:
  Kobject_mappable *map_root() { return this; }
  Kobject_dbg *dbg_info() { return this; }
  Kobject_dbg const *dbg_info() const { return this; }
  Kobject *kobject() { return this; }
  Kobject const *kobject() const { return this; }

private:
  template<typename T>
  class Tconv {};

  template<typename T>
  class Tconv<T*> { public: typedef T Base; };

public:
  bool is_local(Space *) const { return false; }
  Mword obj_id() const { return ~0UL; }
  void initiate_deletion(Kobject ***);
  void destroy(Kobject ***);
  bool put() { return true; }

  template<typename T>
  static T dcast(Kobject_common *_o)
  {
    if (EXPECT_FALSE(!_o))
      return 0;

    if (EXPECT_TRUE(_o->kobj_type() == Tconv<T>::Base::static_kobj_type))
      return reinterpret_cast<T>(_o->kobject_start_addr());
    return 0;
  }

  template<typename T>
  static T dcast(Kobject_common const *_o)
  {
    if (EXPECT_FALSE(!_o))
      return 0;

    if (EXPECT_TRUE(_o->kobj_type() == Tconv<T>::Base::static_kobj_type))
      return reinterpret_cast<T>(_o->kobject_start_addr());
    return 0;
  }

  Lock existence_lock;

public:
  Kobject *_next_to_reap;

public:
  enum Op {
    O_dec_refcnt = 0,
  };

};

#define FIASCO_DECLARE_KOBJ() \
  public: static char const *const static_kobj_type; \
          char const *kobj_type() const { return static_kobj_type; } \
          Address kobject_start_addr() const { return (Address)this; } \
          Mword kobject_size() const { return sizeof(*this); }

#define FIASCO_DEFINE_KOBJ(t) \
  char const *const t::static_kobj_type = #t


//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "logdefs.h"
#include "l4_buf_iter.h"
#include "lock_guard.h"

PUBLIC inline NEEDS["lock_guard.h"]
Smword
Kobject_mappable::dec_cap_refcnt(Smword diff)
{
  Lock_guard<Lock> g(&_lock);
  _cnt -= diff;
  return _cnt;
}


IMPLEMENT inline
void
Kobject::initiate_deletion(Kobject ***reap_list)
{
  existence_lock.invalidate();

  _next_to_reap = 0;
  **reap_list = this;
  *reap_list = &_next_to_reap;
}

IMPLEMENT
void
Kobject::destroy(Kobject ***)
{
  LOG_TRACE("Kobject destroy", "des", current(), __fmt_kobj_destroy,
      Log_destroy *l = tbe->payload<Log_destroy>();
      l->id = dbg_id();
      l->obj = this;
      l->type = kobj_type());
  existence_lock.wait_free();
}

PUBLIC virtual
Kobject::~Kobject()
{
  LOG_TRACE("Kobject delete (generic)", "del", current(), __fmt_kobj_destroy,
      Log_destroy *l = tbe->payload<Log_destroy>();
      l->id = dbg_id();
      l->obj = this;
      l->type = "unk");
}


PRIVATE inline NOEXPORT
L4_msg_tag
Kobject::sys_dec_refcnt(L4_msg_tag tag, Utcb const *in, Utcb *out)
{
  if (tag.words() < 2)
    return Kobject_iface::commit_result(-L4_err::EInval);

  Smword diff = in->values[1];
  out->values[0] = dec_cap_refcnt(diff);
  return Kobject_iface::commit_result(0);
}

PUBLIC
L4_msg_tag
Kobject::kobject_invoke(L4_obj_ref, Mword /*rights*/,
                        Syscall_frame *f,
                        Utcb const *in, Utcb *out)
{
  L4_msg_tag tag = f->tag();

  if (EXPECT_FALSE(tag.words() < 1))
    return Kobject_iface::commit_result(-L4_err::EInval);

  switch (in->values[0])
    {
    case O_dec_refcnt:
      return sys_dec_refcnt(tag, in, out);
    default:
      return Kobject_iface::commit_result(-L4_err::ENosys);
    }

}

PUBLIC static
Kobject *
Kobject::id_to_obj(unsigned long id)
{ return static_cast<Kobject*>(Kobject_dbg::id_to_obj(id)); }

PUBLIC static inline
Kobject *
Kobject::pointer_to_obj(void const *p)
{ return static_cast<Kobject*>(Kobject_dbg::pointer_to_obj(p)); }


//---------------------------------------------------------------------------
INTERFACE [debug]:

#include "tb_entry.h"

EXTENSION class Kobject
{
protected:
  struct Log_destroy
  {
    Kobject    *obj;
    Mword       id;
    char const *type;
    Mword       ram;
  };

  static unsigned log_fmt(Tb_entry *, int max, char *buf) asm ("__fmt_kobj_destroy");
};

//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:


IMPLEMENT
unsigned
Kobject::log_fmt(Tb_entry *e, int max, char *buf)
{
  Log_destroy *l = e->payload<Log_destroy>();
  return snprintf(buf, max, "obj=%lx [%s] (%p) ram=%lx", l->id, l->type, l->obj, l->ram);
}
