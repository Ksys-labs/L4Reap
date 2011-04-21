INTERFACE [arm]:

#include "kip.h"

class Kip_init
{
public:
  static void init();
};


//---------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include <cstring>

#include "config.h"
#include "panic.h"
#include "boot_info.h"
#include "kmem.h"


// Make the stuff below apearing only in this compilation unit.
// Trick Preprocess to let the struct reside in the cc file rather
// than putting it into the _i.h file which is perfectly wrong in 
// this case.
namespace KIP_namespace
{
  enum
  {
    Num_mem_descs = 20,
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
	/* 00 */ L4_KERNEL_INFO_MAGIC,
	         Config::kernel_version_id,
	         (Size_mem_descs + sizeof(Kip)) >> 4,
	         0,
	/* 10 */ {},
	/* 20 */ 0, 0, {},
	/* 30 */ 0, 0, {},
	/* 40 */ 0, 0, {},
	/* 50 */ 0, (sizeof(Kip) << (sizeof(Mword)*4)) | Num_mem_descs, 0, 0,
	/* 60 */ 0, 0, {},
	/* A0 */ 0, {},
	/* B0 */ 0, 0, {},
	/* C0 */ {},
	/* E0 */ 0, 0, {},
	/* F0 */ { },
      },
      "",
    };

};

IMPLEMENT
void Kip_init::init()
{
  Kip *kinfo = reinterpret_cast<Kip*>(&KIP_namespace::my_kernel_info_page);
  Kip::init_global_kip(kinfo);
  kinfo->add_mem_region(Mem_desc(0, Mem_layout::User_max - 1, 
	                  Mem_desc::Conventional, true));
}
