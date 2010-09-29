//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "space.h"
#include "task.h"
#include "thread.h"

class Syscalls : public Thread { };

extern "C" void sys_invoke_debug(Kobject *o, Syscall_frame *f);

PUBLIC inline NOEXPORT ALWAYS_INLINE
void
Syscalls::sys_invoke_debug()
{
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
IMPLEMENTATION [!debug]:

#include "thread.h"

extern "C" void sys_invoke_debug_wrapper() {}

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

