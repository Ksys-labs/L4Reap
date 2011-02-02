IMPLEMENTATION [log]:

#include <alloca.h>
#include <cstring>
#include "config.h"
#include "jdb_trace.h"
#include "jdb_tbuf.h"
#include "types.h"
#include "cpu_lock.h"

PUBLIC static inline NEEDS["jdb_trace.h"]
int
Thread::log_page_fault()
{
  return Jdb_pf_trace::log();
}


/** Page-fault logging.
 */
void
Thread::page_fault_log(Address pfa, unsigned error_code, unsigned long eip)
{
  if (Jdb_pf_trace::check_restriction(current_thread()->dbg_id(), pfa))
    {
      Lock_guard <Cpu_lock> guard (&cpu_lock);

      Tb_entry_pf *tb = static_cast<Tb_entry_pf*>
	(EXPECT_TRUE(Jdb_pf_trace::log_buf()) ? Jdb_tbuf::new_entry()
				    : alloca(sizeof(Tb_entry_pf)));
      tb->set(this, eip, pfa, error_code, current()->space());

      if (EXPECT_TRUE(Jdb_pf_trace::log_buf()))
	Jdb_tbuf::commit_entry();
      else
	Jdb_tbuf::direct_log_entry(tb, "PF");
    }
}

