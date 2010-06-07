IMPLEMENTATION [ux]:

#include <sys/ptrace.h>
#include <sys/wait.h>

#include "cpu_lock.h"
#include "hostproc.h"
#include "lock_guard.h"
#include "map_util.h"
#include "mem_layout.h"

IMPLEMENT
void
Task::host_init()
{
  mem_space()->set_pid (Hostproc::create());
}

PRIVATE inline
bool
Task::invoke_arch(L4_msg_tag &tag, Utcb *utcb)
{

  switch (utcb->values[0])
    {
    case Ldt_set_x86:
        {
          enum
          {
            Utcb_values_per_ldt_entry
              = Cpu::Ldt_entry_size / sizeof(utcb->values[0]),
          };
          if (EXPECT_FALSE(tag.words() < 3
                           || tag.words() % Utcb_values_per_ldt_entry))
            {
              tag = commit_result(-L4_err::EInval);
              return true;
            }

          unsigned entry_number  = utcb->values[1];
          unsigned idx           = 2;
          Mword *trampoline_page = (Mword *)Kmem::phys_to_virt
                                       (Mem_layout::Trampoline_frame);

          for (; idx < tag.words()
              ; idx += Utcb_values_per_ldt_entry,
              ++entry_number)
            {
              Gdt_entry *d = (Gdt_entry *)&utcb->values[idx];
              if (!d->limit())
                continue;

              Ldt_user_desc info;
              info.entry_number    = entry_number;
              info.base_addr       =  d->base();
              info.limit           =  d->limit();
              info.seg_32bit       =  d->seg32();
              info.contents        =  d->contents();
              info.read_exec_only  = !d->writable();
              info.limit_in_pages  =  d->granularity();
              info.seg_not_present = !d->present();
              info.useable         =  d->avl();


              // Set up data on trampoline
              for (unsigned i = 0; i < sizeof(info) / sizeof(Mword); i++)
                *(trampoline_page + i + 1) = *(((Mword *)&info) + i);

              // Call modify_ldt for given user process
              Trampoline::syscall(pid(), __NR_modify_ldt,
                                  1, // write LDT
                                  Mem_layout::Trampoline_page + sizeof(Mword),
                                  sizeof(info));

              // Also set this for the fiasco kernel so that
              // segment registers can be set, this is necessary for signal
              // handling, esp. for sigreturn to work in the Fiasco kernel
              // with the context of the client (gs/fs values).
              if (*(trampoline_page + 1))
                Emulation::modify_ldt(*(trampoline_page + 1), // entry
                                      0,                      // base
                                      1);                     // size
            }
        }
      return true;
    }

  return false;
}

/** Map tracebuffer into each userland task for easy access. */
IMPLEMENT
void
Task::map_tbuf ()
{
  return; // FIXME
#if 0
  if (id() != Config::sigma0_taskno)
    {
      mem_map (sigma0_task, 
	  L4_fpage(0, 1, Config::PAGE_SHIFT, 
	    Kmem::virt_to_phys ((const void*)(Mem_layout::Tbuf_status_page))),
	  nonull_static_cast<Space*>(this),    // to: space
	  L4_fpage(0, 0, Config::PAGE_SHIFT,
	    Mem_layout::Tbuf_ustatus_page, L4_fpage::Cached), 0);

      for (Address size=0; size<Jdb_tbuf::size(); size+=Config::PAGE_SIZE)
	{
	  mem_map (sigma0_task,                            // from: space
	      L4_fpage(0, 1, Config::PAGE_SHIFT,
		Kmem::virt_to_phys ((const void*)
		  (Mem_layout::Tbuf_buffer_area+size))),
	      nonull_static_cast<Space*>(this),	   // to: space
	      L4_fpage(0, 0, Config::PAGE_SHIFT, 
		Mem_layout::Tbuf_ubuffer_area+size, L4_fpage::Cached), 0);
	}
    }
#endif
}

IMPLEMENT
Task::~Task()
{
  free_utcbs();
  //_state = Dying;
  //unregister_space();
  //shutdown();
  //dequeue_at_parent();

#if 0
  if (id() == Config::kernel_taskno)
    {
      reset_dirty();		// Must not deallocate kernel pagetable.
      return;			// No need to kill kernel process.
    }
#endif

  Lock_guard<Cpu_lock> guard (&cpu_lock);

  pid_t hostpid = pid();
  ptrace (PTRACE_KILL, hostpid, NULL, NULL);

  while (waitpid (hostpid, NULL, 0) != hostpid)
    ;
}


IMPLEMENT
void
Task::map_utcb_ptr_page()
{
  //Mem_space::Status res =
	mem_space()->v_insert(
	    Mem_space::Phys_addr::create(Mem_layout::Utcb_ptr_frame),
	    Mem_space::Addr::create(Mem_layout::Utcb_ptr_page_user),
	    Mem_space::Size::create(Config::PAGE_SIZE),
	    Mem_space::Page_writable
	    | Mem_space::Page_user_accessible
	    | Mem_space::Page_cacheable);
}

