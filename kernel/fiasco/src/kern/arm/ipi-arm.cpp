IMPLEMENTATION [!mp]:

PUBLIC static inline
bool Ipi::is_ipi(unsigned /*irq*/)
{ return false; }


//-------------------------------------------------------------------------
INTERFACE [mp]:

EXTENSION class Ipi
{
public:
  enum Message {
    Ipi_start = 1,
    Global_request = Ipi_start, Request, Debug,
    Ipi_end
  };
};


//-------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include "cpu.h"
#include "gic.h"

PUBLIC static
void Ipi::ipi_call_debug_arch()
{
}

PUBLIC static inline NEEDS["gic.h"]
void Ipi::eoi(Message)
{
  // with the ARM-GIC we have to do the EOI right after the ACK
  stat_received();
}

PUBLIC static
void Ipi::send(int logical_cpu, Message m)
{
  Gic_pin::_gic[0].softint_cpu(1 << Cpu::cpus.cpu(logical_cpu).phys_id(), m);
  stat_sent(logical_cpu);
}

PUBLIC static inline
void
Ipi::bcast(Message m)
{
  Gic_pin::_gic[0].softint_bcast(m);
}

PUBLIC static inline
bool Ipi::is_ipi(unsigned irq)
{ return irq >= Ipi_start && irq < Ipi_end; }
