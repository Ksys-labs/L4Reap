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

/** IPC logging.
    called from interrupt gate.
 */
PUBLIC inline NOEXPORT ALWAYS_INLINE
void
Thread::sys_ipc_log()
{
  Entry_frame   *regs      = reinterpret_cast<Entry_frame*>(this->regs());
  Syscall_frame *ipc_regs  = reinterpret_cast<Syscall_frame*>(this->regs());

  Mword entry_event_num    = (Mword)-1;
  Unsigned8 have_snd       = (ipc_regs->ref().flags() & L4_obj_ref::Ipc_send)
                             || (ipc_regs->ref().flags() == L4_obj_ref::Ipc_call);
  Unsigned8 is_next_period = ipc_regs->next_period();
  int do_log               = Jdb_ipc_trace::log() &&
				Jdb_ipc_trace::check_restriction (dbg_id(),
					 static_cast<Task*>(space())->dbg_id(),
					 ipc_regs, 0);

  if (Jdb_nextper_trace::log() && is_next_period)
    {
      Tb_entry_ipc *tb = static_cast<Tb_entry_ipc*>(Jdb_tbuf::new_entry());
      tb->set(this, regs->ip(), ipc_regs, access_utcb(),
	      0, sched_context()->left());
      Jdb_tbuf::commit_entry();
      goto skip_ipc_log;
    }

  if (do_log)
    {
      Mword dbg_id;
	{
	  Obj_cap r = ipc_regs->ref();
	  unsigned char rights;
	  Kobject_iface *o = r.deref(&rights, true);
	  if (o)
	    dbg_id = o->dbg_info()->dbg_id();
	  else
	    dbg_id = ~0UL;
	}
      Tb_entry_ipc *tb = static_cast<Tb_entry_ipc*>
	(EXPECT_TRUE(Jdb_ipc_trace::log_buf()) ? Jdb_tbuf::new_entry()
					   : alloca(sizeof(Tb_entry_ipc)));
      tb->set(this, regs->ip(), ipc_regs, access_utcb(),
	      dbg_id, sched_context()->left());

      entry_event_num = tb->number();

      if (EXPECT_TRUE(Jdb_ipc_trace::log_buf()))
	Jdb_tbuf::commit_entry();
      else
	Jdb_tbuf::direct_log_entry(tb, "IPC");
    }

skip_ipc_log:

  // now pass control to regular sys_ipc()
  ipc_short_cut_wrapper();

  if (Jdb_nextper_trace::log() && is_next_period)
    {
      Tb_entry_ipc_res *tb =
	    static_cast<Tb_entry_ipc_res*>(Jdb_tbuf::new_entry());
      tb->set(this, regs->ip(), ipc_regs, access_utcb(), 0,
	      entry_event_num, have_snd, is_next_period);
      Jdb_tbuf::commit_entry();
      goto skip_ipc_res_log;
    }

  if (Jdb_ipc_trace::log() && Jdb_ipc_trace::log_result() && do_log)
    {
      Tb_entry_ipc_res *tb = static_cast<Tb_entry_ipc_res*>
	(EXPECT_TRUE(Jdb_ipc_trace::log_buf()) ? Jdb_tbuf::new_entry()
					    : alloca(sizeof(Tb_entry_ipc_res)));
      tb->set(this, regs->ip(), ipc_regs, access_utcb(), access_utcb()->error.raw(),
	      entry_event_num, have_snd, is_next_period);

      if (EXPECT_TRUE(Jdb_ipc_trace::log_buf()))
	Jdb_tbuf::commit_entry();
      else
	Jdb_tbuf::direct_log_entry(tb, "IPC result");
    }

skip_ipc_res_log:
  ;
}

/** IPC tracing.
 */
PUBLIC inline NOEXPORT ALWAYS_INLINE
void
Thread::sys_ipc_trace()
{
  Entry_frame *ef      = nonull_static_cast<Entry_frame*>(this->regs());
  Syscall_frame *regs  = this->regs();

  //Mword      from_spec = regs->from_spec();
  L4_obj_ref snd_dst   = regs->ref();

  Unsigned64 orig_tsc  = Cpu::rdtsc();

  // first try the fastpath, then the "slowpath"
  ipc_short_cut_wrapper();

  // kernel is locked here => no Lock_guard <...> needed
  Tb_entry_ipc_trace *tb =
    static_cast<Tb_entry_ipc_trace*>(Jdb_tbuf::new_entry());

  tb->set(this, ef->ip(), orig_tsc, snd_dst, regs->from_spec(),
          L4_msg_tag(0,0,0,0), 0, 0);

  Jdb_tbuf::commit_entry();
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


extern "C" void sys_ipc_log_wrapper(void)
{
  current_thread()->sys_ipc_log();
}

extern "C" void sys_ipc_trace_wrapper(void)
{
  current_thread()->sys_ipc_trace();
}

