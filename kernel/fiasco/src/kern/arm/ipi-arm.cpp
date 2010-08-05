IMPLEMENTATION [!mp]:

PUBLIC static inline
bool Ipi::is_ipi(unsigned /*irq*/)
{ return false; }


//-------------------------------------------------------------------------
INTERFACE [mp]:

EXTENSION class Ipi
{
private:
  Unsigned32 _phys_id;

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
#include "processor.h"

PUBLIC inline
Ipi::Ipi() : _phys_id(~0)
{}

IMPLEMENT inline NEEDS["processor.h"]
void
Ipi::init()
{
  _phys_id = Proc::cpu_id();
}

PUBLIC static
void Ipi::ipi_call_debug_arch()
{
}

PUBLIC static inline
void Ipi::eoi(Message)
{
  // with the ARM-GIC we have to do the EOI right after the ACK
  stat_received();
}

PUBLIC inline NEEDS["gic.h"]
void Ipi::send(Message m)
{
  Gic_pin::_gic[0].softint_cpu(1 << _phys_id, m);
  stat_sent();
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
