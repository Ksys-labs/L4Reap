INTERFACE[ia32,amd64]:

/**
 * A special CPU lock, that disables IRQ's via the PIC.
 */
EXTENSION class Cpu_lock
{
private:
  Unsigned32 pic_status; ///< The save PIC mask.
  Status _is_set;        ///< The current state of the lock.
};


IMPLEMENTATION[ia32,amd64]:

#include "config.h"
#include "pic.h"
#include "processor.h"

// 
// Cpu_lock inlines
// 

IMPLEMENT inline
Cpu_lock::Cpu_lock() 
  : _is_set(0)
{
}

IMPLEMENT inline NEEDS ["config.h","processor.h","pic.h"]
void Cpu_lock::lock()
{
  // When profiling, we use a sligtly different strategy: Instead of
  // disabling interrupts in the CPU, we disable all interrupts but the
  // profiling timer interrupt in the PIC.
  Proc::cli();
  if (! _is_set)
    {
      // mask out all irqs except the profiling timer interrupt
      pic_status = Pic::disable_all_save();
      if(Config::profiling)
	Pic::enable_locked(Config::profile_irq);
      _is_set = 1;
    }
  Proc::sti();
  Proc::irq_chance();	// last chance for an irq to be delivered
}


/** 
 * (IA32 only) Clear the kernel lock, but disable interrupts.
 * There is a difference betweeen ``kernel lock'' and ``disabled interrupts''
 * if the kernel lock is not implemented using CPU-interrupt disabling
 * (but, for example, IRQ disabling in the PIC).
 */
PUBLIC inline NEEDS ["processor.h", "pic.h"]
void Cpu_lock::clear_irqdisable()
{
  Proc::cli();
  if (_is_set)
    {
      Pic::restore_all(pic_status);
      _is_set = 0;
    }
}

IMPLEMENT inline NEEDS ["processor.h",Cpu_lock::clear_irqdisable]
void Cpu_lock::clear()
{
  clear_irqdisable();
  Proc::sti();
}

IMPLEMENT inline 
Cpu_lock::Status Cpu_lock::test() const
{
  return _is_set;
}
