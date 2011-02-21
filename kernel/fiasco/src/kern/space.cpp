INTERFACE [!io]:

class Io_space;

//--------------------------------------------------------------------------
INTERFACE [io]:

#include "config.h"
#include "io_space.h"
#include "l4_types.h"

class Space;

/** Global definition of Io_space for map_util stuff. */
typedef Generic_io_space<Space> Io_space;

//--------------------------------------------------------------------------
INTERFACE:

#include "mem_space.h"
#include "member_offs.h"
#include "obj_space.h"
#include "spin_lock.h"
#include "ref_obj.h"
#include "slab_cache_anon.h"

class Ram_quota;
class Context;
class Kobject;

class Space;

typedef Generic_obj_space<Space> Obj_space;

class Space : public Ref_cnt_obj
{
  MEMBER_OFFSET();
  friend class Jdb_space;

public:

  struct Default_factory
  {
    /** Create a usual Mem_space object. */
    template< typename A1 >
    static void create(Mem_space *v, A1 a1)
    { new (v) Mem_space(a1); }

    /** Create a kernel Mem_space object. */
    template< typename A1, typename A2 >
    static void create(Mem_space *v, A1 a1, A2 a2)
    { new (v) Mem_space(a1, a2); }

    template< typename S >
    static void create(S *v)
    { new (v) S(); }
  };

  /** The interface for Obj_space to get the surrounding Space. */
  static Space *space(Obj_space const *os)
  {
    return reinterpret_cast<Space*>(Address(os) - Address(&reinterpret_cast<Space*>(32)->_obj_space) + 32);
  }

  enum State
  { // we must use values with the two lest significant bits == 0
    Starting    = 0x00,
    Ready       = 0x08,
    In_deletion = 0x10
  };

  struct Ku_mem
  {
    Ku_mem *next;
    User<void>::Ptr u_addr;
    void *k_addr;
    unsigned size;

    static slab_cache_anon *a;

    Ku_mem() : next(0) {}

    void *operator new (size_t, Ram_quota *q) throw()
    { return a->q_alloc(q); }

    void free(Ram_quota *q) throw()
    { a->q_free(q, this); }

    template<typename T>
    T *kern_addr(Smart_ptr<T, Simple_ptr_policy, User_ptr_discr> ua) const
    {
      typedef Address A;
      return (T*)((A)ua.get() - (A)u_addr.get() + (A)k_addr);
    }
  };

private:
  template<typename SPACE>
  class Space_container
  {
  public:
    char __attribute__((aligned(__alignof__(Mword)))) s[sizeof(SPACE)];

    template< typename SF >
    Space_container(SF const &sf)
    { sf.create(get()); }

    template< typename SF, typename A1 >
    Space_container(SF const &sf, A1 a1)
    { sf.create(get(), a1); }

    template< typename SF, typename A1, typename A2 >
    Space_container(SF const &sf, A1 a1, A2 a2)
    { sf.create(get(), a1, a2); }

    ~Space_container()
    { delete get(); }

    SPACE *get()
    { return reinterpret_cast<SPACE*>(s); }

    SPACE const *get() const
    { return reinterpret_cast<SPACE const*>(s); }
  };

  typedef Space_container<Mem_space> Mem_space_container;

  // DATA
  Mem_space_container _mem_space;
  Obj_space _obj_space;

  void switchin_ldt() const;

protected:
  Ku_mem *_ku_mem;
};


//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "atomic.h"
#include "kdb_ke.h"
#include "lock_guard.h"
#include "config.h"
#include "globalconfig.h"
#include "l4_types.h"

//
// class Space
//



/**
 * UTCB area functions.
 */
//@{


/**
 * Get size of UTCB area in bytes.
 *
 * @return the size of the UTCB area in bytes.
 */
PUBLIC inline
unsigned long
Space::utcb_area_size() const
{ return _ku_mem->size; }

PUBLIC inline
Address
Space::kern_utcb_area() const
{ return (Address)_ku_mem->k_addr; }

/**
 * Get the start of the UTCB area in the user address-space.
 *
 * @return the start address of the UTCB area in trhe user address-space.
 */
PUBLIC inline
Address
Space::user_utcb_area() const
{ return (Address)_ku_mem->u_addr.get(); }


//@}

PUBLIC
Space::Ku_mem const *
Space::find_ku_mem(User<void>::Ptr p, unsigned size)
{
  if ((Address)p.get() & (sizeof(double) - 1))
    return 0;

  for (Ku_mem const *f = _ku_mem; f; f = f->next)
    {
      Address a = (Address)f->u_addr.get();
      Address pa = (Address)p.get();
      if (a <= pa && (a + f->size) >= (pa + size))
	return f;
    }

  return 0;
}

PUBLIC virtual
Space::~Space()
{}



/** Constructor.  Creates a new address space and registers it with
  * Space_index.
  *
  * Registration may fail (if a task with the given number already
  * exists, or if another thread creates an address space for the same
  * task number concurrently).  In this case, the newly-created
  * address space should be deleted again.
  *
  * @param number Task number of the new address space
  */
PUBLIC template< typename SPACE_FACTORY >
inline
Space::Space(SPACE_FACTORY const &sf, Ram_quota *q)
  : _mem_space(sf, q), _ku_mem(0)
#ifdef CONFIG_IO_PROT
    , _io_space(sf)
#endif
{}


PROTECTED template<typename SPACE_FACTORY>
Space::Space(SPACE_FACTORY const &sf, Ram_quota *q, Mem_space::Dir_type* pdir)
  : _mem_space(sf, q, pdir)
#ifdef CONFIG_IO_PROT
    , _io_space(sf)
#endif
{}


PUBLIC inline
Ram_quota *
Space::ram_quota() const
{ return _mem_space.get()->ram_quota(); }

PROTECTED
void
Space::reset_dirty()
{
  _mem_space.get()->reset_dirty();
}


PUBLIC inline
void
Space::switchin_context(Space *from)
{
  // XXX: check when activating unbound thread for drq-handling
  // better solution: set freshly created (unbound) threads to kernel_space
  // so that we do not have to make the following 'if':
  if (this)
    {
      _mem_space.get()->switchin_context(from->mem_space());

      if (this != from)
	switchin_ldt();
    }
}


// Mem_space utilities

// Return memory space.
PUBLIC inline
Mem_space const *
Space::mem_space() const
{ return _mem_space.get(); }

PUBLIC inline
Mem_space*
Space::mem_space()
{
  return _mem_space.get();
}

PUBLIC static inline
bool
Space::is_user_memory(Address address, Mword len)
{
  return    address < Mem_layout::User_max
         && address + len <= Mem_layout::User_max;
}

PUBLIC inline
bool
Space::lookup_space(Mem_space** out_space)
{
  *out_space = mem_space();
  return true;
}

PUBLIC inline
Obj_space*
Space::obj_space()
{
  return &_obj_space;
}

PUBLIC inline
bool
Space::lookup_space(Obj_space** out_cap_space)
{
  *out_cap_space = obj_space();
  return true;
}

// ------------------------------------------------------------------------
IMPLEMENTATION [!io && !ux]:

// Is this task a privileged one?
PUBLIC static inline
bool
Space::has_io_privileges()
{ return true; }
