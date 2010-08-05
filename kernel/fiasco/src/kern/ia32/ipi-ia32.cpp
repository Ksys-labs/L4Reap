INTERFACE [mp]:

#include "per_cpu_data.h"

EXTENSION class Ipi
{
private:
  Unsigned32 _apic_id;
  unsigned _count;

public:
  enum Message
  {
    Request        = APIC_IRQ_BASE - 1,
    Global_request = APIC_IRQ_BASE + 2,
    Debug          = APIC_IRQ_BASE - 2
  };
};


//---------------------------------------------------------------------------
IMPLEMENTATION[mp]:

#include <cstdio>
#include "apic.h"
#include "kmem.h"

PUBLIC inline
Ipi::Ipi() : _apic_id(~0)
{}

IMPLEMENT inline NEEDS["apic.h"]
void
Ipi::init()
{
  _apic_id = Apic::get_id();
}


PUBLIC static inline
void
Ipi::ipi_call_debug_arch()
{
  //ipi_call_spin(); // debug
}

PUBLIC static inline NEEDS["apic.h"]
void
Ipi::eoi(Message)
{
  Apic::mp_ipi_ack();
  stat_received();
}

PUBLIC inline NEEDS["apic.h"]
void
Ipi::send(Message m)
{
  Apic::mp_send_ipi(_apic_id, (Unsigned8)m);
  stat_sent();
}

PUBLIC static inline NEEDS["apic.h"]
void
Ipi::bcast(Message m)
{
  Apic::mp_send_ipi(Apic::APIC_IPI_OTHERS, (Unsigned8)m);
}

#if defined(CONFIG_IRQ_SPINNER)

// debug
PRIVATE static
void Ipi::ipi_call_spin()
{
  unsigned cpu;
  Ipi *ipi = 0;
  for (cpu = 0; cpu < Config::Max_num_cpus; ++cpu)
    {
      if (!Per_cpu_data::valid(cpu))
	continue;

      if (Ipi::cpu(cpu)._apic_id == Apic::get_id())
	{
	  ipi = &Ipi::cpu(cpu);
	  break;
	}
    }

  if (!ipi)
    return;

  *(unsigned char*)(Mem_layout::Adap_vram_cga_beg + 22*160 + cpu*+2)
    = '0' + (ipi->_count++ % 10);
}
#endif

