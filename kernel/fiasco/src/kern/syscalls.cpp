INTERFACE:

class Sys_ex_regs_frame;


IMPLEMENTATION:

#include <cstdio>

#include "l4_types.h"

#include "config.h"
#include "entry_frame.h"
#include "feature.h"
#include "irq.h"
#include "logdefs.h"
#include "map_util.h"
#include "processor.h"
#include "ram_quota.h"
#include "space.h"
#include "std_macros.h"
#include "task.h"
#include "thread.h"
#include "thread_state.h"
#include "warn.h"

class Syscalls : public Thread
{

public:
  void sys_invoke_object();
protected:
  Syscalls();
};


extern "C" void sys_invoke_debug(Kobject *o, Syscall_frame *f) __attribute__((weak));


PUBLIC inline NOEXPORT ALWAYS_INLINE
void
Syscalls::sys_invoke_debug()
{
  if (!&::sys_invoke_debug)
    return;

  Syscall_frame *f = this->regs();
  //printf("sys_invoke_debugger(f=%p, obj=%lx)\n", f, f->ref().raw());
  Kobject_iface *o = space()->obj_space()->lookup_local(f->ref().cap());
  if (o)
    ::sys_invoke_debug(o->kobject(), f);
  else
    f->tag(commit_error(access_utcb(), L4_error::Not_existent));
}


// these wrappers must come last in the source so that the real sys-call
// implementations can be inlined by g++

extern "C" void sys_invoke_debug_wrapper()
{ static_cast<Syscalls*>(current_thread())->sys_invoke_debug(); }


//---------------------------------------------------------------------------
INTERFACE [ia32 || ux || amd64]:

extern void (*syscall_table[])();


//---------------------------------------------------------------------------
IMPLEMENTATION [ia32 || ux || amd64]:

void (*syscall_table[])() =
{
  sys_ipc_wrapper,
  0,
  sys_invoke_debug_wrapper,
};

