INTERFACE:

EXTENSION class Kernel_thread
{
public:
  static int	init_done();

private:
  static int	free_initcall_section_done;
};

IMPLEMENTATION[ux]:

#include <unistd.h>
#include <sys/mman.h>
#include "boot_info.h"
#include "fb.h"
#include "kdb_ke.h"
#include "net.h"
#include "mem_layout.h"
#include "pic.h"
#include "trap_state.h"
#include "usermode.h"

int Kernel_thread::free_initcall_section_done;

IMPLEMENT inline
int
Kernel_thread::init_done()
{
  return free_initcall_section_done;
}

IMPLEMENT inline NEEDS [<unistd.h>, <sys/mman.h>, "mem_layout.h"]
void
Kernel_thread::free_initcall_section()
{
  munmap ((void*)&Mem_layout::initcall_start, 
          &Mem_layout::initcall_end - &Mem_layout::initcall_start);
  free_initcall_section_done = 1;
}

IMPLEMENT FIASCO_INIT
void
Kernel_thread::bootstrap_arch()
{
  // install slow trap handler
  nested_trap_handler      = Trap_state::base_handler;
  Trap_state::base_handler = thread_handle_trap;

  if (Boot_info::jdb_cmd())
    kdb_ke_sequence (Boot_info::jdb_cmd());

  if (Boot_info::wait())
    kdb_ke ("Wait");

  boot_app_cpus();
}

IMPLEMENT
void
Kernel_thread::arch_exit()
{
  fflush(0);  // Flush output stream
  Pic::irq_prov_shutdown(); // atexit calls are not run with _exit
  _exit(0);   // Don't call destructors
}

//--------------------------------------------------------------------------
IMPLEMENTATION [!mp]:

static void inline
Kernel_thread::boot_app_cpus()
{}


//--------------------------------------------------------------------------
IMPLEMENTATION [mp]:

static void
Kernel_thread::boot_app_cpus()
{
  printf("MP: launching APs...\n");
  if (0)
    {
       extern char _tramp_mp_entry[];
	printf("new child: %d   [%d]\n",
	       Emulation::spawn_cpu_thread((Address)_tramp_mp_entry),
	       Emulation::gettid());
    }
}
