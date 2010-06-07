INTERFACE [arm && mp && realview]:

#include "types.h"

class Boot_mp
{
};

IMPLEMENTATION [arm && mp && realview]:

#include "io.h"
#include "ipi.h"
#include "platform.h"

PUBLIC
void
Boot_mp::start_ap_cpus(Address phys_tramp_mp_addr)
{
  // set physical start address for AP CPUs
  Platform::write(Platform::Sys::Flags_clr, 0xffffffff);
  Platform::write(Platform::Sys::Flags, phys_tramp_mp_addr);

  // wake up AP CPUs
  Ipi::bcast(Ipi::Global_request);
}

PUBLIC
void
Boot_mp::cleanup()
{}
