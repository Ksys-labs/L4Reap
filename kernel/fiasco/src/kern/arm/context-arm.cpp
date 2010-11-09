INTERFACE [arm]:

EXTENSION class Context
{
public:
  void set_ignore_mem_op_in_progress(bool val);
  bool is_ignore_mem_op_in_progress() const { return _ignore_mem_op_in_progess; }

private:
  bool _ignore_mem_op_in_progess;
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include <cassert>
#include <cstdio>

#include "globals.h"		// current()
#include "l4_types.h"
#include "cpu_lock.h"
#include "kmem.h"
#include "lock_guard.h"
#include "space.h"
#include "thread_state.h"
#include "utcb_support.h"


IMPLEMENT inline
void
Context::fill_user_state()
{
  // do not use 'Return_frame const *rf = regs();' here as it triggers an
  // optimization bug in gcc-4.4(.1)
  Entry_frame const *ef = regs();
  asm volatile ("ldmia %[rf], {sp, lr}^"
      : : "m"(ef->usp), "m"(ef->ulr), [rf] "r" (&ef->usp));
}

IMPLEMENT inline
void
Context::spill_user_state()
{
  Entry_frame *ef = regs();
  assert_kdb (current() == this);
  asm volatile ("stmia %[rf], {sp, lr}^"
      : "=m"(ef->usp), "=m"(ef->ulr) : [rf] "r" (&ef->usp));
}

IMPLEMENT inline
void
Context::switch_cpu(Context *t)
{
  update_consumed_time();

  spill_user_state();
  t->fill_user_state();

  {
    register Mword _old_this asm("r1") = (Mword)this;
    register Mword _new_this asm("r0") = (Mword)t;
    unsigned long dummy1, dummy2;

    asm volatile
      (// save context of old thread
       "   stmdb sp!, {fp}          \n"
       "   adr   lr, 1f             \n"
       "   str   lr, [sp, #-4]!     \n"
       "   str   sp, [%[old_sp]]    \n"

       // switch to new stack
       "   mov   sp, %[new_sp]      \n"

       // deliver requests to new thread
       "   bl switchin_context_label \n" // call Context::switchin_context(Context *)

       // return to new context
       "   ldr   pc, [sp], #4       \n"
       "1: ldmia sp!, {fp}          \n"

       :
                    "=r" (_old_this),
                    "=r" (_new_this),
       [old_sp]     "=r" (dummy1),
       [new_sp]     "=r" (dummy2)
       :
       "0" (_old_this),
       "1" (_new_this),
       "2" (&_kernel_sp),
       "3" (t->_kernel_sp)
       : "r4", "r5", "r6", "r7", "r8", "r9",
         "r10", "r12", "r14", "memory");
  }
}

/** Thread context switchin.  Called on every re-activation of a
    thread (switch_exec()).  This method is public only because it is
    called by an ``extern "C"'' function that is called
    from assembly code (call_switchin_context).
 */
IMPLEMENT
void Context::switchin_context(Context *from)
{
  assert_kdb (this == current());
  assert_kdb (state() & Thread_ready_mask);

#if 0
  printf("switch in address space: %p\n",_space);
#endif

  // switch to our page directory if nessecary
  vcpu_aware_space()->switchin_context(from->vcpu_aware_space());

  Utcb_support::current((Utcb*)local_id());
}


IMPLEMENT inline
void
Context::set_ignore_mem_op_in_progress(bool val)
{
  _ignore_mem_op_in_progess = val;
  Mem::barrier();
}

//-----------------------------------------------------------------------------
IMPLEMENTATION [arm && vcache]:

PUBLIC inline
Utcb*
Context::access_utcb() const
{
  // Do not use the alias mapping of the UTCB for the current address space
  return Mem_space::current_mem_space(current_cpu()) == mem_space()
         ? (Utcb*)local_id()
         : utcb();
}

PUBLIC inline
Vcpu_state *
Context::access_vcpu(bool is_current = false) const
{
  // Do not use the alias mapping of the UTCB for the current address space
  return is_current || Mem_space::current_mem_space(current_cpu()) == mem_space()
         ? reinterpret_cast<Vcpu_state *>(local_id() + sizeof(Utcb))
         : vcpu_state();
}


//-----------------------------------------------------------------------------
IMPLEMENTATION [arm && !vcache]:

PUBLIC inline
Utcb*
Context::access_utcb() const
{
  return current_cpu() == cpu() && Mem_space::current_mem_space(current_cpu()) == mem_space()
         ? (Utcb*)local_id()
         : utcb();
}

PUBLIC inline
Vcpu_state *
Context::access_vcpu(bool is_current = false) const
{
  // Do not use the alias mapping of the vCPU for the current address space
  return is_current || Mem_space::current_mem_space(current_cpu()) == mem_space()
         ? reinterpret_cast<Vcpu_state *>(local_id() + sizeof(Utcb))
         : vcpu_state();
}
