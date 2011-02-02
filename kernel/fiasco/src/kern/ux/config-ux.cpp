/*
 * Fiasco-UX
 * Architecture specific config code
 */

INTERFACE:

#include "idt_init.h"

EXTENSION class Config
{
public:
  enum
  {
    // cannot access user memory directly
    Access_user_mem = No_access_user_mem,

    SCHED_PIT = 0,
  };

  static const unsigned scheduler_mode		= SCHED_PIT;
  static const unsigned scheduler_irq_vector	= 0x20U;
  static const unsigned scheduler_granularity	= 10000U;
  static const unsigned default_time_slice	= 10 * scheduler_granularity;

  enum {
    // Size of the host address space, change the following if your host
    // address space size is different, do not go below 2GB (0x80000000)
    Host_as_size       = 0xc0000000U
  };

  static const bool hlt_works_ok		= true;
  static const bool getchar_does_hlt		= false;
  static const bool getchar_does_hlt_works_ok   = false;
  static const bool pic_prio_modify		= true;
  static const bool enable_io_protection	= false;
  static const bool kinfo_timer_uses_rdtsc	= false;
  static const bool scheduler_one_shot          = false;
  static const bool old_sigma0_adapter_hack     = false;

  static const char char_micro;

  enum {
    Scheduling_irq = 0,
    Max_num_dirqs = 16,
    Max_num_irqs  = Max_num_dirqs + 4,
    Tbuf_irq = Max_num_dirqs + 1,
    Is_ux = 1,
  };
};

IMPLEMENTATION[ux]:

#include <feature.h>
KIP_KERNEL_FEATURE("io_prot");

char const Config::char_micro = '\265';
const char *const Config::kernel_warn_config_string = 0;

IMPLEMENT FIASCO_INIT
void
Config::init_arch()
{}
