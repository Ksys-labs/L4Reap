INTERFACE:

#include "kobject.h"
#include "mapping_tree.h"
#include "obj_helping_lock.h"
#include "kmem_slab.h"

class Ram_quota;

class U_lock : public Kobject
{
  FIASCO_DECLARE_KOBJ();

private:
  typedef slab_cache_anon Allocator;

  Ram_quota *_q;
  unsigned long _cnt;
  mutable Obj_helping_lock _l;
public:
  Context *lockers;

public:
  virtual ~U_lock() {}
};


IMPLEMENTATION:

FIASCO_DEFINE_KOBJ(U_lock);

PUBLIC inline
U_lock::U_lock(Ram_quota *q) : _q(q), _cnt(0), lockers(0) {}

PUBLIC inline
Obj_helping_lock::Status 
U_lock::lock()
{ return _l.lock(); }

PUBLIC inline
void
U_lock::clear()
{ _l.clear(); }

PUBLIC inline
unsigned
U_lock::dec_ref_cnt()
{
  Lock_guard<Cpu_lock> guard(&cpu_lock);
  --_cnt;
  if (_cnt == 0 && Mappable::no_mappings())
    return 0;
  else
    return 1;
}

PUBLIC inline
void
U_lock::inc_ref_cnt()
{ ++_cnt; }

PUBLIC inline
bool 
U_lock::no_mappings() const
{
  Lock_guard<Cpu_lock> guard(&cpu_lock);
  if (Mappable::no_mappings())
    {
      _l.invalidate();
      return !_cnt;
    }
  return 0;
}



PUBLIC static
U_lock*
U_lock::alloc(Ram_quota *q)
{
  void *nq;
  if (q->alloc(sizeof(U_lock)) && (nq = allocator()->alloc()))
    return new (nq) U_lock(q);

  return 0;
}

PUBLIC
void *
U_lock::operator new (size_t, void *p)
{ return p; }

PUBLIC 
void
U_lock::operator delete (void *_l)
{
  U_lock *l = reinterpret_cast<U_lock*>(_l);
  if (l->_q)
    l->_q->free(sizeof(U_lock));

  allocator()->free(l);
}

static Kmem_slab_t<U_lock> _ulock_allocator("U_lock");

PRIVATE static
U_lock::Allocator *
U_lock::allocator()
{ return &_ulock_allocator; }

PUBLIC
void
U_lock::invoke(Syscall_frame *, Utcb *)
{
  printf("hoooo\n");
}
