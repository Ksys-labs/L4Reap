INTERFACE [ia32 || amd64]:

class Cpu;

EXTENSION class Utcb_init
{
public:
  /**
   * Value for GS and FS.
   * @return Value the GS and FS register has to be loaded with when
   *         entering user mode.
   */
  static Unsigned32 utcb_segment();
};

//-----------------------------------------------------------------------------
IMPLEMENTATION [ia32 || amd64]:

#include <cstdio>
#include "gdt.h"
#include "paging.h"
#include "panic.h"
#include "space.h"
#include "vmem_alloc.h"

IMPLEMENT static inline NEEDS ["gdt.h"]
Unsigned32
Utcb_init::utcb_segment()
{ return Gdt::gdt_utcb | Gdt::Selector_user; }

PUBLIC static
void
Utcb_init::init_ap(Cpu const &cpu)
{
  cpu.get_gdt()->set_entry_byte(Gdt::gdt_utcb / 8,
                                Address(&Mem_layout::user_utcb_ptr(cpu.id()))
                                 - Mem_layout::Utcb_ptr_offset,
                                sizeof(Address) - 1,
                                Gdt_entry::Access_user
                                 | Gdt_entry::Access_data_write
                                 | Gdt_entry::Accessed,
                                Gdt_entry::Size_32);

  cpu.set_gs(utcb_segment());
  cpu.set_fs(utcb_segment());
}

IMPLEMENT
void
Utcb_init::init()
{
  if (!Vmem_alloc::page_alloc((void *)Mem_layout::Utcb_ptr_page,
                              Vmem_alloc::ZERO_FILL, Vmem_alloc::User))
    panic("UTCB pointer page allocation failure");

  init_ap(*Cpu::boot_cpu());
}
