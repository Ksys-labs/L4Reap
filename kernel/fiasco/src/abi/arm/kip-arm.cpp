/*
 * ARM Kernel-Info Page
 */

INTERFACE [arm]:

#include "types.h"

EXTENSION class Kip
{
public:

  Mword magic;
  Mword version;
  Mword offset_version_strings;
  Mword res0; //offset_memory_descs;

  /* 0x10 */
  Mword _res1[4];


  /* the following stuff is undocumented; we assume that the kernel
     info page is located at offset 0x1000 into the L4 kernel boot
     image so that these declarations are consistent with section 2.9
     of the L4 Reference Manual */

  /* 0x20 */
  Mword sigma0_sp, sigma0_ip;
  Mword _res2[2];

  /* 0x30 */
  Mword sigma1_sp, sigma1_ip;
  Mword _res3[2];

  /* 0x40 */
  Mword root_sp,   root_ip;
  Mword _res4[2];

  /* 0x50 */
  Mword l4_config;
  Mword _mem_info;
  Mword kdebug_config;
  Mword kdebug_permission;

  /* 0x60 */
  Mword total_ram;
  Mword processor_info;
  Mword _res6[14];

  /* 0xA0 */
  volatile Cpu_time clock;
  Mword _res7[2];

  /* 0xB0 */
  Mword frequency_cpu;
  Mword frequency_bus;
  Mword _res10[2];

  /* 0xC0 */
  Mword _res11[8];

  /* 0xE0 */
  Mword user_ptr;
  Mword vhw_offset;
  char _res8[8];

  /* 0xF0 */
  Unsigned32 __reserved[20];
};

//---------------------------------------------------------------------------
IMPLEMENTATION [arm && debug]:

IMPLEMENT inline
void
Kip::debug_print_syscalls() const
{}

