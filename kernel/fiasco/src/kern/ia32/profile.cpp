INTERFACE[profile]:

#include "types.h"

class Profile
{
  static unsigned long ticks;
  static bool exit;
};


INTERFACE[{ia32,amd64}-!profile]:

class Profile
{
};


IMPLEMENTATION[profile]:

#include <cstdlib>

#include "config.h"
#include "globals.h"
#include "gmon.h"
#include "idt.h"
#include "mem_layout.h"
#include "pic.h"
#include "pit.h"
#include "processor.h"
#include "unistd.h"		// pr_base, pr_off, pr_size, pr_scale
#include "thread.h"		// for stack checking

unsigned long Profile::ticks = 0;
bool          Profile::exit  = false;

static bool profile_active = false;

extern "C" void profile_interrupt_entry();

static void
dump_if_active()
{
  if (profile_active)
    _mcleanup();
}

// We assume that our member functions generally are called in cli
// mode.

// set up profiling and initialize profiling interrupt
PUBLIC static 
void
Profile::init()
{
  atexit(dump_if_active);
  Idt::set_entry (0x20 + Config::profile_irq,
                  (unsigned) profile_interrupt_entry, false);
}

PUBLIC static 
void
Profile::start()
{
  if (! profile_active)
    {
      monstartup((char*)&Mem_layout::start, (char*)&Mem_layout::end, 
		 Config::profiling_rate);
      profile_active = true;
      Pit::init(Config::profiling_rate);
      Pic::enable(Config::profile_irq);
    }
}

PUBLIC static 
void
Profile::stop()
{
  if (profile_active)
    {
      Pic::disable(Config::profile_irq);
      moncontrol(0);
      profile_active = false;
    }
}

PUBLIC static 
void
Profile::stop_and_dump()
{
  if (profile_active)
    {
      Pic::disable(Config::profile_irq);
      _mcleanup();
      profile_active = false;
    }
}

// irq routine invoked by profile_interrupt_entry in entry.S

/*
 * Scale is a fixed-point number with the binary point 16 bits
 * into the value, and is <= 1.0.  pc is at most 32 bits, so the
 * intermediate result is at most 48 bits.
 */
#define PC_TO_INDEX(pc, off, scale)				\
        ((Address)(((unsigned long long)((pc) - (off)) *	\
		(unsigned long long)((scale))) >> 16) & ~1)

PUBLIC static inline NOEXPORT
void
Profile::handle_profile_interrupt(Address pc)
{
  // runs with disabled irqs
  Pic::disable_locked(Config::profile_irq);
  Pic::acknowledge_locked(Config::profile_irq);
  Pic::enable_locked(Config::profile_irq);

  ticks++;

  size_t i;

  if (! pr_scale)
    return;

  if (pc < pr_off 
      || (i = PC_TO_INDEX(pc, pr_off, pr_scale)) >= pr_size)
    return;			// out of range - ignore

  *reinterpret_cast<unsigned short*>(pr_base + i) += 1;

  if (exit)
    ::exit(0);
}

extern "C" FIASCO_FASTCALL
void
profile_interrupt(Address pc)
{
  Profile::handle_profile_interrupt(pc);
}

extern "C" 
void
profile_mcount_wrap(unsigned short *frompcindex, char *selfpc )
{
  // For lack of a better place, so stack checking here:

  static bool overrun = false;
  if (! overrun)
    {
      Address sp = Proc::stack_pointer();
      if (((Address)current()) + sizeof(Thread) + 0x20 > sp)
	{
	  overrun = true;
	  panic("stack overrun: current=0x%x, esp=0x%x", 
		(Address)current(), sp);
	}
    }
  
  __mcount_internal (frompcindex, selfpc);
}

/* The GNU Glibc has this to say:

   We need a special version of the `mcount' function since for ix86 it
   must not clobber any register.  This has several reasons:
     - there is a bug in gcc as of version 2.7.2.2 which prohibits the
       use of profiling together with nested functions
     - the ELF `fixup' function uses GCC's regparm feature
     - some (future) systems might want to pass parameters in registers.  */

#define __STR(x) #x
#define STR(x) __STR(x)

asm(".p2align 4			\n\t"
    ".globl mcount		\n\t"
"mcount:			\n\t"
    "pushl %eax			\n\t"
    "pushl %ecx			\n\t"
    "pushl %edx			\n\t"

    "movl 12(%esp), %eax	\n\t"
    "movl 4(%ebp), %ecx		\n\t"
    "pushl %eax			\n\t"
    "pushl %ecx			\n\t"

    "call " STR(profile_mcount_wrap) "\n\t"
    "addl $8,%esp		\n\t"

    "popl %edx			\n\t"
    "popl %ecx			\n\t"
    "popl %eax			\n\t"
    "ret");


//---------------------------------------------------------------------------
IMPLEMENTATION[{ia32,amd64}-!profile]:

PUBLIC static inline void Profile::init() {}
PUBLIC static inline void Profile::start() {}
PUBLIC static inline void Profile::stop() {}
PUBLIC static inline void Profile::stop_and_dump() {}
