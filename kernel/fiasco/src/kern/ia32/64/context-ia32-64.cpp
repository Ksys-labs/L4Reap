IMPLEMENTATION [amd64]:


IMPLEMENT inline NEEDS [Context::update_consumed_time,
			Context::store_segments]
void
Context::switch_cpu (Context *t)
{
  Mword dummy1, dummy2, dummy3, dummy4;

  update_consumed_time();

  store_segments();

  asm volatile
    (
     "   push %%rbp			\n\t"	// save base ptr of old thread
     "   pushq $1f			\n\t"	// push restart addr on old stack
     "   mov  %%rsp, (%[old_ksp])	\n\t"	// save stack pointer
     "   mov  (%[new_ksp]), %%rsp	\n\t"	// load new stack pointer
     // in new context now (cli'd)
     "   call  switchin_context_label	\n\t"	// switch pagetable
     "   pop   %%rax			\n\t"	// don't do ret here -- we want
     "   jmp   *%%rax			\n\t"	// to preserve the return stack
     // restart code
     "  .p2align 4			\n\t"	// start code at new cache line
     "1: pop %%rbp			\n\t"	// restore base ptr

     : "=c" (dummy1), "=a" (dummy2), "=D" (dummy3), "=S" (dummy4)
     : [old_ksp]    "c" (&_kernel_sp),
       [new_ksp]    "a" (&t->_kernel_sp),
       [new_thread] "D" (t),
       [old_thread] "S" (this)
     : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rbx", "rdx", "memory");
}

