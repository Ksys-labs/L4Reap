IMPLEMENTATION:

#include "config.h"
#include "mapdb.h"

/** Map the region described by "fp_from" of address space "from" into
    region "fp_to" at offset "offs" of address space "to", updating the
    mapping database as we do so.
    @param from source address space
    @param fp_from_{page, size, write, grant} flexpage description for
	virtual-address space range in source address space
    @param to destination address space
    @param fp_to_{page, size} flexpage descripton for virtual-address
	space range in destination address space
    @param offs sender-specified offset into destination flexpage
    @return IPC error code that describes the status of the operation
 */
L4_error
mem_map(Space *from, L4_fpage const &fp_from,
        Space *to, L4_fpage const &fp_to, Mword control)
{
  typedef Map_traits<Mem_space> Mt;

  if (EXPECT_FALSE(fp_from.order() < L4_fpage::Mem_addr::Shift
                   || fp_to.order() < L4_fpage::Mem_addr::Shift))
    return L4_error::None;

  // loop variables
  Mt::Addr rcv_addr = fp_to.mem_address();
  Mt::Addr snd_addr = fp_from.mem_address();
  Mt::Addr offs = Virt_addr(control & (~0UL << L4_fpage::Control_addr_shift));

  Mt::Size snd_size = Mt::Size::from_shift(fp_from.order() - L4_fpage::Mem_addr::Shift);
  Mt::Size rcv_size = Mt::Size::from_shift(fp_to.order() - L4_fpage::Mem_addr::Shift);

  // calc size in bytes from power of tows
  snd_addr = snd_addr.trunc(snd_size);
  rcv_addr = rcv_addr.trunc(rcv_size);
  Mt::constraint(snd_addr, snd_size, rcv_addr, rcv_size, offs);

  if (snd_size == 0)
    {
      if (Config::conservative)
	kdb_ke("fpage transfer = nop");
      return L4_error::None;
    }

  bool fp_from_grant = control & 2;

  unsigned long del_attribs, add_attribs;
  Mt::attribs(control, fp_from, &del_attribs, &add_attribs);

  return map(mapdb_instance(),
	     from->mem_space(), from, snd_addr,
	     snd_size, to->mem_space(), to,
	     rcv_addr, fp_from_grant, add_attribs, del_attribs,
	     (Mem_space::Reap_list**)0);
}

/** Unmap the mappings in the region described by "fp" from the address
    space "space" and/or the address spaces the mappings have been
    mapped into.
    @param space address space that should be flushed
    @param fp    flexpage descriptor of address-space range that should
                 be flushed
    @param me_too If false, only flush recursive mappings.  If true,
                 additionally flush the region in the given address space.
    @param restriction Only flush specific task ID.
    @param flush_mode determines which access privileges to remove.
    @return combined (bit-ORed) access status of unmapped physical pages
*/
unsigned
mem_fpage_unmap(Space *space, L4_fpage fp, L4_map_mask mask)
{
  typedef Map_traits<Mem_space> Mt;
  Mt::Size size = Mt::Size::from_shift(fp.order() - L4_fpage::Mem_addr::Shift);
  Mt::Addr start = Mt::get_addr(fp);

  start = start.trunc(size);

  return unmap(mapdb_instance(), space->mem_space(), space,
               start, size,
               fp.rights(), mask, (Mem_space::Reap_list**)0);
}

static inline
void
save_access_attribs(Mapdb* mapdb, const Mapdb::Frame& mapdb_frame,
                    Mapping* mapping, Mem_space* space, unsigned page_rights, 
                    Mem_space::Addr virt, Mem_space::Phys_addr phys,
                    Mem_space::Size size,
                    bool me_too)
{
  typedef Mem_space::Size Size;
  typedef Mem_space::Addr Addr;
  typedef Mem_space::Phys_addr Phys_addr;

  if (unsigned page_accessed
      = page_rights & (Mem_space::Page_referenced | Mem_space::Page_dirty))
    {
      Mem_space::Status status;
      Mem_space *s;

      // When flushing access attributes from our space as well,
      // cache them in parent space, otherwise in our space.
      if (! me_too || !mapping->parent())
	{
	  status = space->v_insert(phys, virt, size,
				   page_accessed);

	  s = space;
	}
      else
	{
	  Mapping *parent = mapping->parent();

	  Mem_space *parent_space = parent->space()->mem_space();

	  Address parent_shift = mapdb->shift(mapdb_frame, parent) + Mem_space::Page_shift;
	  Address parent_address = parent->page().value() << parent_shift;

	  status =
	    parent_space->v_insert(phys.trunc(Phys_addr::create(1UL << parent_shift)),
				   Addr::create(parent_address),
				   Size::create(1UL << parent_shift),
				   page_accessed, true);

	  s = parent_space;
	}

      assert (status == Mem_space::Insert_ok
	      || status == Mem_space::Insert_warn_exists
	      || status == Mem_space::Insert_warn_attrib_upgrade
	      || s->is_sigma0());
      // Be forgiving to sigma0 because it's address
      // space is not kept in sync with its mapping-db
      // entries.
    }
}

//
// Mapdb instance for memory mappings
//

Mapdb *mapdb_instance() FIASCO_CONST;

//---------------------------------------------------------------------------
IMPLEMENTATION[!64bit]:

/** The mapping database.
    This is the system's instance of the mapping database.
 */
Mapdb *
mapdb_instance()
{
  static const size_t page_sizes[]
    = { Config::SUPERPAGE_SHIFT - Config::PAGE_SHIFT, 0 };

  static Mapdb mapdb(sigma0_task,
      Page_number::create(1U << (32 - Config::SUPERPAGE_SHIFT)),
      page_sizes, 2);

  return &mapdb;
}

//---------------------------------------------------------------------------
IMPLEMENTATION[amd64]:

#include "cpu.h"

/** The mapping database.
    This is the system's instance of the mapping database.
 */
Mapdb *
mapdb_instance()
{
  static const size_t amd64_page_sizes[] =
  { Config::PML4E_SHIFT - Config::PAGE_SHIFT,
    Config::PDPE_SHIFT - Config::PAGE_SHIFT,
    Config::SUPERPAGE_SHIFT - Config::PAGE_SHIFT,
    0};

  static const unsigned num_page_sizes = Cpu::boot_cpu()->phys_bits() >= 42 ?  4 : 3;

  static const Address largest_page_max_index = num_page_sizes == 4 ?
    Config::PML4E_MASK + 1ULL
    : 1ULL << (Cpu::boot_cpu()->phys_bits() - Config::PDPE_SHIFT);

  static const size_t* page_sizes = num_page_sizes == 4 ? 
    amd64_page_sizes : amd64_page_sizes + 1;

  static Mapdb mapdb((Space*)sigma0_task,
      Page_number::create(largest_page_max_index),
      page_sizes, num_page_sizes);
  return &mapdb;
}

