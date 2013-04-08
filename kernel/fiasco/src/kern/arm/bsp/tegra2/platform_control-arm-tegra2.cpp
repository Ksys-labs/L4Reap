INTERFACE [arm && mp && tegra2]:

#include "mem_layout.h"

EXTENSION class Platform_control
{
private:
  enum
  {
    Reset_vector_addr              = 0x6000f100,
    Clk_rst_ctrl_clk_cpu_cmplx     = 0x6000604c,
    Clk_rst_ctrl_rst_cpu_cmplx_clr = 0x60006344,
    Unhalt_addr                    = 0x60007014,
  };

  static Mword _orig_reset_vector;
};

IMPLEMENTATION [arm && mp && tegra2]:

#include "io.h"
#include "kmem.h"
#include <cstdlib>

Mword Platform_control::_orig_reset_vector;

PRIVATE static
void Platform_control::reset_orig_reset_vector()
{
  Io::write<Mword>(_orig_reset_vector, Kmem::mmio_remap(Reset_vector_addr));
}

PUBLIC static
void
Platform_control::boot_ap_cpus(Address phys_reset_vector)
{
  // remember original reset vector
  _orig_reset_vector = Io::read<Mword>(Kmem::mmio_remap(Reset_vector_addr));

  // set (temporary) new reset vector
  Io::write<Mword>(phys_reset_vector, Kmem::mmio_remap(Reset_vector_addr));

  atexit(reset_orig_reset_vector);

  // clocks on other cpu
  Mword r = Io::read<Mword>(Kmem::mmio_remap(Clk_rst_ctrl_clk_cpu_cmplx));
  Io::write<Mword>(r & ~(1 << 9), Kmem::mmio_remap(Clk_rst_ctrl_clk_cpu_cmplx));
  Io::write<Mword>((1 << 13) | (1 << 9) | (1 << 5) | (1 << 1),
		   Kmem::mmio_remap(Clk_rst_ctrl_rst_cpu_cmplx_clr));

  // kick cpu1
  Io::write<Mword>(0, Kmem::mmio_remap(Unhalt_addr));
}

