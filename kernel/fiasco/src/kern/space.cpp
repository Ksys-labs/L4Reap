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
  Spin_lock_coloc<State>    _state;

  Mem_space_container _mem_space;
  Obj_space _obj_space;

  Address _utcb_area_start;
  Address _utcb_kernel_area_start;
  unsigned _utcb_area_size;

  void switchin_ldt() const;

protected:
  void utcb_area(Address user_va, Address kern_va, unsigned long size);

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


PROTECTED inline
void
Space::set_kern_utcb_area(Address kern_va)
{ _utcb_kernel_area_start = kern_va; }

/**
 * Get size of UTCB area in bytes.
 *
 * @return the size of the UTCB area in bytes.
 */
PUBLIC inline
unsigned long
Space::utcb_area_size() const
{ return _utcb_area_size; }

PUBLIC inline
Address
Space::kern_utcb_area() const
{ return _utcb_kernel_area_start; }

/**
 * Get the start of the UTCB area in the user address-space.
 *
 * @return the start address of the UTCB area in trhe user address-space.
 */
PUBLIC inline
Address
Space::user_utcb_area() const
{ return _utcb_area_start; }

PRIVATE inline
void
Space::user_utcb_area(Address user_va)
{ _utcb_area_start = user_va; }

/**
 * Make the size and the start address of the UTCB area sane.
 *
 * This means we must have at least Config::PAGE_SIZE as size
 * and the start address must be size aligned.
 *
 * \todo We might ensure also a power of two as size.
 */
PRIVATE inline
void
Space::sanitize_utcb_area()
{
  if (_utcb_area_size > 0 && _utcb_area_size < Config::PAGE_SIZE)
    _utcb_area_size = Config::PAGE_SIZE;

  _utcb_area_start &= ~(Address(_utcb_area_size) - 1);
}

PUBLIC inline
bool
Space::is_utcb_valid(void *utcb, unsigned nr_utcbs = 1)
{
  Address u = Address(utcb);
  if (EXPECT_FALSE(u < user_utcb_area()))
    return false;

  return EXPECT_TRUE(u + sizeof(Utcb) * nr_utcbs
                     <= user_utcb_area() + utcb_area_size());
}

PUBLIC inline
Utcb *
Space::kernel_utcb(void *user_utcb)
{
  // user_utcb == 0 for all kernel threads
  assert (!user_utcb || is_utcb_valid(user_utcb));
  Address const user_va = Address(user_utcb);
  return (Utcb*)(_utcb_kernel_area_start + user_va - user_utcb_area());
}

//@}

PUBLIC inline
virtual
Space::~Space()
{
}



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
inline NEEDS[Space::sanitize_utcb_area]
Space::Space(SPACE_FACTORY const &sf, Ram_quota *q, L4_fpage const &utcb_area)
  : _mem_space(sf, q),
    _utcb_area_start(Virt_addr(utcb_area.mem_address()).value()),
    _utcb_area_size(utcb_area.is_valid() ? (1UL << utcb_area.order()) : 0)
#ifdef CONFIG_IO_PROT
    , _io_space(sf)
#endif
{
  _state.init();

  if (!utcb_area.is_mempage())
    {
      _utcb_area_size = 0;
      _utcb_area_start = 0;
    }

  sanitize_utcb_area();
}


PROTECTED template<typename SPACE_FACTORY>
Space::Space (SPACE_FACTORY const &sf, Ram_quota *q, Mem_space::Dir_type* pdir)
  : _mem_space(sf, q, pdir),
    _utcb_area_size(0)
#ifdef CONFIG_IO_PROT
    , _io_space(sf)
#endif
{
  _state.init();
}


PUBLIC inline
Ram_quota *
Space::ram_quota() const
{ return _mem_space.get()->ram_quota(); }

PROTECTED
void
Space::reset_dirty ()
{
  _mem_space.get()->reset_dirty ();
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

/// Return memory space.
PUBLIC inline
Mem_space const *
Space::mem_space () const
{ return _mem_space.get(); }

PUBLIC inline
Mem_space*
Space::mem_space ()
{
  return _mem_space.get();
}

PUBLIC inline
bool
Space::lookup_space (Mem_space** out_space)
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
Space::lookup_space (Obj_space** out_cap_space)
{
  *out_cap_space = obj_space();
  return true;
}

PUBLIC inline
Space::State
Space::state() const
{ return _state.get_unused(); }

PUBLIC inline
void
Space::set_state(State s)
{ _state.set_unused(s); }

PUBLIC inline
Spin_lock_coloc<Space::State> *
Space::state_lock()
{ return &_state; }


//////////////////////////////////////////////////////////////////////

IMPLEMENTATION [!io && !ux]:

/// Is this task a privileged one?
PUBLIC static inline 
bool
Space::has_io_privileges()
{ return true; }

