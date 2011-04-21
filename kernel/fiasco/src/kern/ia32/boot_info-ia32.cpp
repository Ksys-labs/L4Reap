/* IA32/AMD64 Specific Boot info */
INTERFACE[ia32,ux,amd64]:

#include "multiboot.h"
#include "types.h"


EXTENSION class Boot_info
{
public:

  static Address mbi_phys();

  static Multiboot_info *mbi_virt();
};


//-------------------------------------------------------------------------
INTERFACE[ia32,amd64]:

EXTENSION class Boot_info
{
private:
  static Address  _mbi_pa;
  static unsigned _flag;
  static unsigned _checksum_ro;
  static unsigned _checksum_rw;
  static Multiboot_info _kmbi;
};


//-------------------------------------------------------------------------
IMPLEMENTATION[ux]:

#include "config.h"
#include "koptions.h"
#include "mem_layout.h"

PUBLIC static
Address
Boot_info::kmem_start(Address mem_max)
{
  Address end_addr = (mbi_virt()->mem_upper + 1024) << 10;
  Address size, base;

  if (end_addr > mem_max)
    end_addr = mem_max;

  size = Koptions::o()->kmemsize << 10;
  if (!size)
    {
      size = end_addr / 100 * Config::kernel_mem_per_cent;
      if (size > Config::kernel_mem_max)
	size = Config::kernel_mem_max;
    }

  base = end_addr - size & Config::PAGE_MASK;
  if (Mem_layout::phys_to_pmem(base) < Mem_layout::Physmem)
    base = Mem_layout::pmem_to_phys(Mem_layout::Physmem);

  return base;
}


//-----------------------------------------------------------------------------
IMPLEMENTATION[ia32,amd64]:

#include <cassert>
#include <cstring>
#include <cstdlib>
#include "checksum.h"
#include "mem_layout.h"

// these members needs to be initialized with some
// data to go into the data section and not into bss
Address  Boot_info::_mbi_pa        = 125;
unsigned Boot_info::_flag          = 3;
unsigned Boot_info::_checksum_ro   = 15;
unsigned Boot_info::_checksum_rw   = 16;

// initialized after startup cleaned out the bss

Multiboot_info Boot_info::_kmbi;


/// \defgroup pre init setup
/**
 * The Boot_info object must be set up with these functions
 * before Boot_info::init() is called!
 * This can be done either in __main, if booted on hardware
 * or in an initializer with a higher priority than BOOT_INFO_INIT_PRIO
 * (e.g UX_STARTUP1_INIT_PRIO) if the kernel runs on software (FIASCO-UX)
 */
//@{

PUBLIC inline static
void Boot_info::set_flags(unsigned aflags)
{  _flag = aflags; }

PUBLIC inline static
void Boot_info::set_checksum_ro(unsigned ro_cs)
{  _checksum_ro = ro_cs; }

PUBLIC inline static
void Boot_info::set_checksum_rw(unsigned rw_cs)
{  _checksum_rw = rw_cs; }
//@}


IMPLEMENT
void
Boot_info::init()
{
  // we assume that we run in 1:1 mapped mode
  _kmbi = *(Multiboot_info *)mbi_phys();
}

PUBLIC inline static
unsigned
Boot_info::get_flags(void)
{
  return _flag;
}

PUBLIC inline static
unsigned
Boot_info::get_checksum_ro(void)
{
  return _checksum_ro;
}

PUBLIC inline static
unsigned
Boot_info::get_checksum_rw(void)
{
  return _checksum_rw;
}

PUBLIC static
void
Boot_info::reset_checksum_ro(void)
{
  set_checksum_ro(Checksum::get_checksum_ro());
}

PUBLIC inline static
void
Boot_info::set_mbi_phys(Address phys)
{
  _mbi_pa = phys;
}

IMPLEMENT inline
Address
Boot_info::mbi_phys(void)
{
  return _mbi_pa;
}

IMPLEMENT inline
Multiboot_info *
Boot_info::mbi_virt()
{
  return &_kmbi;
}
