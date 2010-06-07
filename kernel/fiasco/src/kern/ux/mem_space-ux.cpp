/*
 * Fiasco-UX
 * Architecture specific pagetable code
 */

INTERFACE[ux]:
#include <sys/types.h>		// for pid_t
#include "kmem.h"
#include "mem_layout.h"
#include "paging.h"

EXTENSION class Mem_space
{
protected:
  pid_t     _pid;
};

IMPLEMENTATION[ux]:

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include "boot_info.h"
#include "cpu_lock.h"
#include "lock_guard.h"
#include "mem_layout.h"
#include "trampoline.h"
#include <cstring>
#include "config.h"
#include "emulation.h"
#include "logdefs.h"

PRIVATE static inline NEEDS ["kmem.h", "emulation.h"]
Pdir *
Mem_space::current_pdir()
{
  return reinterpret_cast<Pdir *>(Kmem::phys_to_virt (Emulation::pdir_addr()));
}

IMPLEMENT inline NEEDS ["kmem.h", "emulation.h"]
void
Mem_space::make_current()
{
  Emulation::set_pdir_addr (Kmem::virt_to_phys (_dir));
  _current.cpu(current_cpu()) = this;
}

PUBLIC inline
pid_t
Mem_space::pid() const			// returns host pid number
{
  return _pid;
}

PUBLIC inline
void
Mem_space::set_pid(pid_t pid)		// sets host pid number
{
  _pid = pid;
}

IMPLEMENT inline NEEDS["logdefs.h",Mem_space::current_pdir]
void
Mem_space::switchin_context(Mem_space *from)
{
  if (this == from)
    return;

  CNT_ADDR_SPACE_SWITCH;
  make_current();
}

IMPLEMENT inline NEEDS [<asm/unistd.h>, <sys/mman.h>, "boot_info.h",
                        "cpu_lock.h", "lock_guard.h", "mem_layout.h",
                        "trampoline.h"]
void
Mem_space::page_map (Address phys, Address virt, Address size, unsigned attr)
{
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  Mword *trampoline = (Mword *) Mem_layout::kernel_trampoline_page;

  *(trampoline + 1) = virt;
  *(trampoline + 2) = size;
  *(trampoline + 3) = PROT_READ | (attr & Page_writable ? PROT_WRITE : 0);
  *(trampoline + 4) = MAP_SHARED | MAP_FIXED;
  *(trampoline + 5) = Boot_info::fd();

  if (phys >= Boot_info::fb_virt() &&
      phys + size <= Boot_info::fb_virt() +
                     Boot_info::fb_size() +
                     Boot_info::input_size())
    *(trampoline + 6) = Boot_info::fb_phys() + (phys - Boot_info::fb_virt());
  else
    *(trampoline + 6) = phys;

  Trampoline::syscall (pid(), __NR_mmap, Mem_layout::Trampoline_page + 4);
}

IMPLEMENT inline NEEDS [<asm/unistd.h>, "cpu_lock.h", "lock_guard.h",
                        "trampoline.h"]
void
Mem_space::page_unmap (Address virt, Address size)
{
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  Trampoline::syscall (pid(), __NR_munmap, virt, size);
}

IMPLEMENT inline NEEDS [<asm/unistd.h>, "cpu_lock.h", "lock_guard.h",
                        "trampoline.h"]
void
Mem_space::page_protect (Address virt, Address size, unsigned attr)
{
  Lock_guard <Cpu_lock> guard (&cpu_lock);

  Trampoline::syscall (pid(), __NR_mprotect, virt, size,
                       PROT_READ | (attr & Page_writable ? PROT_WRITE : 0));
}

IMPLEMENT inline
void
Mem_space::kmem_update (void *)
{}

PRIVATE inline
void
Mem_space::initial_sync()
{}
