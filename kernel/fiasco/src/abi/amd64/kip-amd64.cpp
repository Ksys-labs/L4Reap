/*
 * AMD64 Kernel-Info Page
 */

INTERFACE [amd64]:

#include "types.h"

EXTENSION class Kip
{
public:

  /* 00 */
  Mword      magic;
  Mword      version;
  Unsigned8  offset_version_strings;
  Unsigned8  fill2[7];
  Unsigned8  kip_sys_calls;
  Unsigned8  fill3[7];

  /* the following stuff is undocumented; we assume that the kernel
     info page is located at offset 0x1000 into the L4 kernel boot
     image so that these declarations are consistent with section 2.9
     of the L4 Reference Manual */

  /* 20 */
  Mword      init_default_kdebug;
  Mword      default_kdebug_exception;
  Mword      sched_granularity;
  Mword      default_kdebug_end;

  /* 40 */
  Mword      sigma0_sp, sigma0_ip;
  Mword	     res2[2];

  /* 60 */
  Mword      sigma1_sp, sigma1_ip;
  Mword	     res3[2];
  
  /* 80 */
  Mword      root_sp, root_ip;
  Mword	     res4[2];

  /* A0 */
  Mword      l4_config;
  Mword      _mem_info;
  Mword      kdebug_config;
  Mword      kdebug_permission;

  /* C0 */
  Mword      total_ram;
  Mword      processor_info;
  Mword	     res5[14];

  /* 140 */
  volatile Cpu_time clock;
  //Unsigned8  fill4[8];
  volatile Cpu_time switch_time;
  //Unsigned8  fill5[8];

  /* 160 */
  Mword      frequency_cpu;
  Mword      frequency_bus;
  volatile Cpu_time thread_time;
  //Unsigned8  fill6[8];

  /* 180 */
  Mword      _res8[4];

  /* 1A0 */
  Mword      _res9[4];

  /* 1C0 */
  Mword      user_ptr;
  Mword      vhw_offset;
  Mword      __pad[2];

  /* 1E0 */
  Kernel_uart_info  kernel_uart_info;
  Platform_info     platform_info;
};

