INTERFACE:

#include "l4_types.h"

class Kobject;
class Kobject_dbg;
class Kobject_mappable;

class Space;
class Syscall_frame;
class Utcb;

class Kobject_common
{
public:
  virtual char const *kobj_type() const = 0;
  virtual Address kobject_start_addr() const = 0;

  virtual bool is_local(Space *) const  = 0;
  virtual Mword obj_id() const  = 0;
  virtual void initiate_deletion(Kobject ***) = 0;
  virtual void destroy(Kobject ***) = 0;
  virtual bool put() = 0;

  virtual Kobject_mappable *map_root() = 0;

  virtual Kobject_dbg *dbg_info() = 0;
  virtual Kobject_dbg const *dbg_info() const = 0;

  virtual Kobject *kobject() = 0;
  virtual Kobject const *kobject() const = 0;
};

class Kobject_iface : public virtual Kobject_common
{
public:
  virtual void invoke(L4_obj_ref self, Mword rights, Syscall_frame *, Utcb *) = 0;
  virtual Kobject_iface *downgrade(unsigned long del_attribs)
  { (void)del_attribs; return this; }
};

IMPLEMENTATION:

PUBLIC static inline
L4_msg_tag
Kobject_iface::commit_result(Mword error,
                             unsigned words = 0, unsigned items = 0)
{
  return L4_msg_tag(words, items, 0, error);
}

PUBLIC static inline
L4_msg_tag
Kobject_iface::commit_error(Utcb const *utcb, L4_error const &e,
                            L4_msg_tag const &tag = L4_msg_tag(0, 0, 0, 0))
{
  const_cast<Utcb*>(utcb)->error = e;
  return L4_msg_tag(tag, L4_msg_tag::Error);
}


