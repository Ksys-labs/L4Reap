INTERFACE:

extern "C" void vcpu_resume(Trap_state *, Return_frame *sp)
   FIASCO_FASTCALL FIASCO_NORETURN;


// --------------------------------------------------------------------------
IMPLEMENTATION:

#include "logdefs.h"
#include "vcpu.h"

PUBLIC inline NEEDS["logdefs.h", "vcpu.h"]
bool
Thread::vcpu_pagefault(Address pfa, Mword err, Mword ip)
{
  (void)ip;
  Vcpu_state *vcpu = access_vcpu();
  if (vcpu_pagefaults_enabled(vcpu))
    {
      spill_user_state();
      vcpu_enter_kernel_mode(vcpu);
      LOG_TRACE("VCPU events", "vcpu", this, __context_vcpu_log_fmt,
	  Vcpu_log *l = tbe->payload<Vcpu_log>();
	  l->type = 3;
	  l->state = vcpu->_saved_state;
	  l->ip = ip;
	  l->sp = pfa;
	  l->space = vcpu_user_space() ? static_cast<Task*>(vcpu_user_space())->dbg_id() : ~0;
	  );
      vcpu->_ts.set_pagefault(pfa, err);
      vcpu_save_state_and_upcall();
      return true;
    }

  return false;
}


