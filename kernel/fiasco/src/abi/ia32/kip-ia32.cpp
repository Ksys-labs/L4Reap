/*
 * IA-32 Kernel-Info Page
 */

INTERFACE [ux]:

#include "vhw.h"

INTERFACE [ia32 || ux]:

#include "types.h"

EXTENSION class Kip
{
public:

  /* 00 */
  Mword      magic;
  Mword      version;
  Unsigned8  offset_version_strings;
  Unsigned8  fill0[3];
  Unsigned8  kip_sys_calls;
  Unsigned8  fill1[3];

  /* the following stuff is undocumented; we assume that the kernel
     info page is located at offset 0x1000 into the L4 kernel boot
     image so that these declarations are consistent with section 2.9
     of the L4 Reference Manual */

  /* 10 */
  Mword      init_default_kdebug;
  Mword      default_kdebug_exception;
  Mword      sched_granularity;
  Mword      default_kdebug_end;

  /* 20 */
  Mword      sigma0_sp, sigma0_ip;
  Mword      _res2[2];

  /* 30 */
  Mword      sigma1_sp, sigma1_ip;
  Mword      _res3[2];

  /* 40 */
  Mword      root_sp, root_ip;
  Mword	     _res4[2];

  /* 50 */
  Mword      l4_config;
  Mword      _mem_info;
  Mword      kdebug_config;
  Mword      kdebug_permission;

  /* 60 */
  Mword      total_ram;
  Mword      processor_info;
  Mword      _res6[14];

  /* A0 */
  volatile Cpu_time clock;
  Unsigned64 _res7;

  /* B0 */
  Mword      frequency_cpu;
  Mword      frequency_bus;
  Unsigned64 _res8;

  /* C0 */
  Mword      _res9[4];

  /* D0 */
  Mword      _res10[4];

  /* E0 */
  Mword      user_ptr;
  Mword      vhw_offset;
  char       __pad[8];

  /* F0 */
  Unsigned32 __reserved[20];
};

//---------------------------------------------------------------------------
IMPLEMENTATION [ux]:

PUBLIC
Vhw_descriptor *
Kip::vhw() const
{
  return reinterpret_cast<Vhw_descriptor*>(((unsigned long)this) + vhw_offset);
}
