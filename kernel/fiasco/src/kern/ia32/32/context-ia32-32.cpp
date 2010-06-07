INTERFACE[ia32 && segments]:

#include "x86desc.h"

EXTENSION class Context
{
protected:
  enum { Gdt_user_entries = 4 };
  Gdt_entry	_gdt_user_entries[Gdt_user_entries];
  Unsigned32	_es, _fs, _gs;
};


//-----------------------------------------------------------------------------
IMPLEMENTATION[ia32 && segments]:

#include "cpu.h"
#include "gdt.h"

PROTECTED inline NEEDS ["cpu.h", "gdt.h"]
void
Context::switch_gdt_user_entries(Context *to)
{
  Gdt &gdt = *Cpu::cpus.cpu(to->cpu()).get_gdt();
  for (unsigned i = 0; i < Gdt_user_entries; ++i)
    gdt[(Gdt::gdt_user_entry1 / 8) + i] = to->_gdt_user_entries[i];
}


//---------------------------------------------------------------------------
IMPLEMENTATION [ia32,ux]:


IMPLEMENT inline NEEDS [Context::update_consumed_time,
			Context::store_segments]
void
Context::switch_cpu (Context *t)
{
  Mword dummy1, dummy2, dummy3, dummy4;

  update_consumed_time();

  store_segments();

  switch_gdt_user_entries(t);

  asm volatile
    (
     "   pushl %%ebp			\n\t"	// save base ptr of old thread
     "   pushl $1f			\n\t"	// restart addr to old stack
     "   movl  %%esp, (%0)		\n\t"	// save stack pointer
     "   movl  (%1), %%esp		\n\t"	// load new stack pointer
     						// in new context now (cli'd)
     "   movl  %2, %%eax		\n\t"	// new thread's "this"
     "   call  switchin_context_label	\n\t"	// switch pagetable
     "   popl  %%eax			\n\t"	// don't do ret here -- we want
     "   jmp   *%%eax			\n\t"	// to preserve the return stack
						// restart code
     "  .p2align 4			\n\t"	// start code at new cache line
     "1: popl %%ebp			\n\t"	// restore base ptr

     : "=c" (dummy1), "=S" (dummy2), "=D" (dummy3), "=d" (dummy4)
     : "c" (&_kernel_sp), "S" (&t->_kernel_sp), "D" (t), "d" (this)
     : "eax", "ebx", "memory");
}


