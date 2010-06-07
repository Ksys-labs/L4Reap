#include <l4/sys/types.h>

typedef l4_umword_t proc_status;

static inline
proc_status proc_cli_save(void)
{
  proc_status ret;
  asm volatile ( "    mrs    r6, cpsr    \n"
		 "    mov    %0, r6      \n"
		 "    orr    r6,r6,#128  \n"
		 "    msr    cpsr_c, r6  \n"
		 : "=r"(ret) : : "r6"
		 );
  return ret;
}

static inline
void proc_sti_restore(proc_status st)
{
  asm volatile ( "    tst    %0, #128    \n"
		 "    bne    1f          \n"
		 "    mrs    r6, cpsr    \n"
		 "    bic    r6,r6,#128  \n"
		 "    msr    cpsr_c, r6  \n"
		 "1:                     \n"
		 : : "r"(st) : "r6"
		 );
}
