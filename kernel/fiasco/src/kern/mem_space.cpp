INTERFACE:

#include "paging.h"		// for page attributes
#include "mem_layout.h"
#include "member_offs.h"
#include "pages.h"
#include "per_cpu_data.h"
#include "ram_quota.h"

class Space;

/**
 * An address space.
 *
 * Insertion and lookup functions.
 */
class Mem_space
{
  MEMBER_OFFSET();

public:
  typedef int Status;

  void *operator new (size_t, void *p)
  { return p; }

  void operator delete (void *)
  {}

  static char const * const name;

  typedef Pdir::Va Vaddr;
  typedef Pdir::Va Vsize;

  typedef Virt_addr Addr;
  typedef Virt_size Size;
  typedef Page_addr<Config::PAGE_SHIFT> Phys_addr;
  typedef void Reap_list;

  // Each architecture must provide these members:
  void switchin_context(Mem_space *from);

  /** Insert a page-table entry, or upgrade an existing entry with new
   *  attributes.
   *
   * @param phys Physical address (page-aligned).
   * @param virt Virtual address for which an entry should be created.
   * @param size Size of the page frame -- 4KB or 4MB.
   * @param page_attribs Attributes for the mapping (see
   *                     Mem_space::Page_attrib).
   * @return Insert_ok if a new mapping was created;
   *         Insert_warn_exists if the mapping already exists;
   *         Insert_warn_attrib_upgrade if the mapping already existed but
   *                                    attributes could be upgraded;
   *         Insert_err_nomem if the mapping could not be inserted because
   *                          the kernel is out of memory;
   *         Insert_err_exists if the mapping could not be inserted because
   *                           another mapping occupies the virtual-address
   *                           range
   * @pre phys and virt need to be size-aligned according to the size argument.
   */
  Status v_insert(Phys_addr phys, Vaddr virt, Vsize size,
                  unsigned page_attribs, bool upgrade_ignore_size = false);

  /** Look up a page-table entry.
   *
   * @param virt Virtual address for which we try the look up.
   * @param phys Meaningful only if we find something (and return true).
   *             If not 0, we fill in the physical address of the found page
   *             frame.
   * @param page_attribs Meaningful only if we find something (and return
   *             true). If not 0, we fill in the page attributes for the
   *             found page frame (see Mem_space::Page_attrib).
   * @param size If not 0, we fill in the size of the page-table slot.  If an
   *             entry was found (and we return true), this is the size
   *             of the page frame.  If no entry was found (and we
   *             return false), this is the size of the free slot.  In
   *             either case, it is either 4KB or 4MB.
   * @return True if an entry was found, false otherwise.
   */
  bool v_lookup(Vaddr virt, Phys_addr *phys = 0, Size *size = 0,
                unsigned *page_attribs = 0);

  /** Delete page-table entries, or some of the entries' attributes.
   *
   * This function works for one or multiple mappings (in contrast to
   * v_insert!).
   *
   * @param virt Virtual address of the memory region that should be changed.
   * @param size Size of the memory region that should be changed.
   * @param page_attribs If nonzero, delete only the given page attributes.
   *                     Otherwise, delete the whole entries.
   * @return Combined (bit-ORed) page attributes that were removed.  In
   *         case of errors, ~Page_all_attribs is additionally bit-ORed in.
   */
  unsigned long v_delete(Vaddr virt, Vsize size,
                         unsigned long page_attribs = Page_all_attribs);

  /** Set this memory space as the current on on this CPU. */
  void make_current();

  /**
   * Update this address space with an entry from the kernel's shared
   * address space.  The kernel maintains a 'master' page directory in
   * class Kmem.  Use this function when an entry from the master directory
   * needs to be copied into this address space.
   * @param addr virtual address for which an entry should be copied from the
   *             shared page directory.
   */
  void kmem_update (void *addr);

  static Mem_space *kernel_space() { return _kernel_space; }
  static inline Mem_space *current_mem_space(unsigned cpu);

  virtual Page_number map_max_address() const
  { return Addr::create(Map_max_address); }

  static Address superpage_size()  { return Map_superpage_size; }

  static Phys_addr page_address(Phys_addr o, Size s)
  { return o.trunc(s); }

  static Phys_addr subpage_address(Phys_addr addr, Size offset)
  { return addr | offset; }

  static Mword phys_to_word(Phys_addr a)
  { return a.value(); }

private:
  Ram_quota *_quota;

  // Each architecture must provide these members

  // Page-table ops
  // We'd like to declare current_pdir here, but Dir_type isn't defined yet.
  // static inline Dir_type *current_pdir();

  // Space reverse lookup
  friend inline Mem_space* current_mem_space();	// Mem_space::current_space

  // Mem_space();
  Mem_space(const Mem_space &);	// undefined copy constructor

  static Per_cpu<Mem_space *> _current asm ("CURRENT_MEM_SPACE");
  static Mem_space *_kernel_space;
};

class Mem_space_q_alloc
{
private:
  Mapped_allocator *_a;
  Ram_quota *_q;
};



//---------------------------------------------------------------------------
INTERFACE [mp]:

#include "cpu_mask.h"

EXTENSION class Mem_space
{
public:
  enum { Need_xcpu_tlb_flush = 1 };
};



//---------------------------------------------------------------------------
INTERFACE [!mp]:

EXTENSION class Mem_space
{
public:
  enum { Need_xcpu_tlb_flush = 0 };
};


//---------------------------------------------------------------------------
IMPLEMENTATION:

//
// class Mem_space
//

#include "config.h"
#include "globals.h"
#include "l4_types.h"
#include "mapped_alloc.h"
#include "mem_unit.h"
#include "paging.h"
#include "panic.h"

PUBLIC inline
Mem_space_q_alloc::Mem_space_q_alloc(Ram_quota *q = 0, Mapped_allocator *a = 0)
  : _a(a), _q(q)
{}


PUBLIC inline
bool
Mem_space_q_alloc::valid() const
{ return _a && _q; }

PUBLIC inline
void *
Mem_space_q_alloc::alloc(unsigned long size) const
{
  if (EXPECT_FALSE(!_q->alloc(size)))
    return 0;

  void *b;
  if (EXPECT_FALSE(!(b=_a->unaligned_alloc(size))))
    {
      _q->free(size);
      return 0;
    }

  return b;
}


PUBLIC inline
void
Mem_space_q_alloc::free(void *block, unsigned long size) const
{
  _a->unaligned_free(size, block);
  _q->free(size);
}


char const * const Mem_space::name = "Mem_space";
Mem_space *Mem_space::_kernel_space;



PUBLIC inline
bool
Mem_space::valid() const
{ return _dir; }

PUBLIC inline
Ram_quota *
Mem_space::ram_quota() const
{ return _quota; }


/// Avoid deallocation of page table upon Mem_space destruction.
PUBLIC
void
Mem_space::reset_dirty ()
{
  _dir = 0;
}

PUBLIC inline
Mem_space::Dir_type*
Mem_space::dir ()
{
  return _dir;
}

PUBLIC inline
const Mem_space::Dir_type*
Mem_space::dir() const
{
  return _dir;
}

inline NEEDS[Mem_space::current_mem_space]
Mem_space *
current_mem_space()
{
  return Mem_space::current_mem_space(0);
}

// routines

/**
 * Tests if a task is the sigma0 task.
 * @return true if the task is sigma0, false otherwise.
 */
PUBLIC inline NEEDS ["globals.h","config.h"]
bool Mem_space::is_sigma0 () const
{
  return this == sigma0_space;
}

// Mapping utilities

PUBLIC
virtual bool
Mem_space::v_fabricate(Vaddr address,
                       Phys_addr* phys, Size* size, unsigned* attribs = 0)
{
  return Mem_space::v_lookup(address.trunc(Size(Map_page_size)),
      phys, size, attribs);
}

//---------------------------------------------------------------------------
IMPLEMENTATION [!io]:

PUBLIC inline
Mword
Mem_space::get_io_counter (void)
{ return 0; }

PUBLIC inline
bool
Mem_space::io_lookup (Address)
{ return false; }
