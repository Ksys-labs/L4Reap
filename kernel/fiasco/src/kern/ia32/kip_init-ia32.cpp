INTERFACE [ia32,ux,amd64]:

#include "initcalls.h"
#include "types.h"

class Cpu;

class Kip_init
{
public:
  /**
   * Insert memory descriptor for the Kmem region and finish the memory
   * info field.
   * @post no more memory descriptors may be added
   */
  static void setup_kmem_region (Address kmem_base, Address kmem_size);
};

IMPLEMENTATION [ia32,ux,amd64]:

#include <cstring>
#include "boot_info.h"
#include "config.h"
#include "cpu.h"
#include "div32.h"
#include "kip.h"
#include "kmem.h"
#include "panic.h"


/** KIP initialization. */
PUBLIC static FIASCO_INIT
void
Kip_init::init_freq(Cpu const &cpu)
{
  Kip::k()->frequency_cpu	= div32(cpu.frequency(), 1000);
}


namespace KIP_namespace
{
  enum
  {
    Num_mem_descs = 50,
    Max_len_version = 512,

    Size_mem_descs = sizeof(Mword) * 2 * Num_mem_descs,
  };

  struct KIP
  {
    Kip kip;
    char mem_descs[Size_mem_descs];
  };

  KIP my_kernel_info_page asm("my_kernel_info_page") __attribute__((section(".kernel_info_page"))) =
    {
      {
	/* 00/00  */ L4_KERNEL_INFO_MAGIC,
	             Config::kernel_version_id,
	             (Size_mem_descs + sizeof(Kip)) >> 4,
	             {}, 0, {},
	/* 10/20  */ 0, 0, 0, 0,
	/* 20/40  */ 0, 0, {},
	/* 30/60  */ 0, 0, {},
	/* 40/80  */ 0, 0, {},
	/* 50/A0  */ 0, (sizeof(Kip) << (sizeof(Mword)*4)) | Num_mem_descs, 0, 0,
	/* 60/C0  */ 0, 0, {},
	/* A0/140 */ 0, 0,
	/* B0/160 */ 0, 0, 0,
	/* C0/180 */ {},
	/* D0/1A0 */ {},
	/* E0/1C0 */ 0, 0, {},
	/* F0/1D0 */ { 0, 0, 0 }, { "" },
      },
      {}
    };
};

PUBLIC static FIASCO_INIT
//IMPLEMENT
void Kip_init::init()
{
  Kip *kinfo = reinterpret_cast<Kip*>(&KIP_namespace::my_kernel_info_page);
  Kip::init_global_kip(kinfo);

  Kip::k()->clock = 0;
  Kip::k()->sched_granularity = Config::scheduler_granularity;

  setup_user_virtual();

  reserve_amd64_hole();


  Mem_desc *md = kinfo->mem_descs();
  Mem_desc *end = md + kinfo->num_mem_descs();

  extern char _boot_sys_start[];
  extern char _boot_sys_end[];

  for (;md != end; ++md)
    {
      if (md->type() != Mem_desc::Reserved || md->is_virtual())
	continue;

      if (md->start() == (Address)_boot_sys_start
	  && md->end() == (Address)_boot_sys_end - 1)
	md->type(Mem_desc::Undefined);

      if (md->contains(Kmem::kernel_image_start())
	  && md->contains(Kmem::kcode_end()-1))
	{
	  *md = Mem_desc(Kmem::kernel_image_start(), Kmem::kcode_end() -1,
	      Mem_desc::Reserved);
	}
    }
}


IMPLEMENTATION [amd64]:

PRIVATE static inline NOEXPORT NEEDS["kip.h"]
void
Kip_init::reserve_amd64_hole()
{
  enum { Trigger = 0x0000800000000000UL };
  Kip::k()->add_mem_region(Mem_desc(Trigger, ~Trigger, 
	                   Mem_desc::Reserved, true));
}

IMPLEMENTATION [!amd64]:

PRIVATE static inline NOEXPORT
void
Kip_init::reserve_amd64_hole()
{}

IMPLEMENTATION [!ux]:

PUBLIC static FIASCO_INIT
void
Kip_init::setup_user_virtual()
{
  Kip *kinfo = reinterpret_cast<Kip*>(&KIP_namespace::my_kernel_info_page);
  kinfo->add_mem_region(Mem_desc(0, Mem_layout::User_max - 1,
                        Mem_desc::Conventional, true));
}

IMPLEMENTATION [ux]:

#include "boot_info.h"
#include "multiboot.h"
#include <cstring>

PUBLIC static FIASCO_INIT
void
Kip_init::setup_user_virtual()
{
  Kip *kinfo = reinterpret_cast<Kip*>(&KIP_namespace::my_kernel_info_page);
  // start at 64k because on some distributions (like Ubuntu 8.04) it's
  // not allowed to map below a certain treshold
  kinfo->add_mem_region(Mem_desc(Boot_info::min_mappable_address(),
                                 Mem_layout::User_max - 1,
                                 Mem_desc::Conventional, true));
}

PUBLIC static  FIASCO_INIT
void
Kip_init::setup_ux()
{
  Kip::init_global_kip((Kip*)&KIP_namespace::my_kernel_info_page);

  Multiboot_module *mbm = reinterpret_cast <Multiboot_module*>
    (Kmem::phys_to_virt (Boot_info::mbi_virt()->mods_addr));
  Kip::k()->user_ptr = (unsigned long)(Boot_info::mbi_phys());
  Mem_desc *m = Kip::k()->mem_descs();

  // start at 64k because on some distributions (like Ubuntu 8.04) it's
  // not allowed to map below a certain treshold
  *(m++) = Mem_desc(Boot_info::min_mappable_address(),
                    ((Boot_info::mbi_virt()->mem_upper + 1024) << 10) - 1,
                    Mem_desc::Conventional);
  *(m++) = Mem_desc(Kmem::kernel_image_start(), Kmem::kcode_end() - 1, 
      Mem_desc::Reserved);

  mbm++;
  Kip::k()->sigma0_ip		= mbm->reserved;
  if ((Boot_info::sigma0_start() & Config::PAGE_MASK)
      != ((Boot_info::sigma0_end() + (Config::PAGE_SIZE-1))
	   & Config::PAGE_MASK))
    *(m++) = Mem_desc(Boot_info::sigma0_start() & Config::PAGE_MASK,
                      ((Boot_info::sigma0_end() + (Config::PAGE_SIZE-1))
                       & Config::PAGE_MASK) - 1,
                      Mem_desc::Reserved);

  mbm++;
  Kip::k()->root_ip		= mbm->reserved;
  if ((Boot_info::root_start() & Config::PAGE_MASK)
      != ((Boot_info::root_end() + (Config::PAGE_SIZE-1)) & Config::PAGE_MASK))
    *(m++) = Mem_desc(Boot_info::root_start() & Config::PAGE_MASK,
                      ((Boot_info::root_end() + (Config::PAGE_SIZE-1))
                       & Config::PAGE_MASK) - 1,
                      Mem_desc::Bootloader);

  unsigned long version_size = 0;
  for (char const *v = Kip::k()->version_string(); *v; )
    {
      unsigned l = strlen(v) + 1;
      v += l;
      version_size += l;
    }

  version_size += 2;

  Kip::k()->vhw_offset = (Kip::k()->offset_version_strings << 4) + version_size;

  Kip::k()->vhw()->init();

  unsigned long mod_start = ~0UL;
  unsigned long mod_end = 0;

  mbm++;

  if (Boot_info::mbi_virt()->mods_count <= 3)
    return;

  for (unsigned i = 0; i < Boot_info::mbi_virt()->mods_count - 3; ++i)
    {
      if (mbm[i].mod_start < mod_start)
	mod_start = mbm[i].mod_start;

      if (mbm[i].mod_end > mod_end)
	mod_end = mbm[i].mod_end;
    }

  mod_start &= ~(Config::PAGE_SIZE - 1);
  mod_end = (mod_end + Config::PAGE_SIZE -1) & ~(Config::PAGE_SIZE - 1);

  if (mod_end > mod_start)
    *(m++) = Mem_desc(mod_start, mod_end - 1, Mem_desc::Bootloader);

  *(m++) = Mem_desc(Boot_info::mbi_phys(),
      ((Boot_info::mbi_phys() + Boot_info::mbi_size()
       + Config::PAGE_SIZE-1) & Config::PAGE_MASK) -1,
      Mem_desc::Bootloader);
}
