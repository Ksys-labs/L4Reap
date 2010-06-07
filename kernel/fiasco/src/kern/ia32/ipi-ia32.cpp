INTERFACE [mp]:

#include "per_cpu_data.h"

EXTENSION class Ipi
{
private:
  static Per_cpu<unsigned> _count;
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

Per_cpu<unsigned> DEFINE_PER_CPU Ipi::_count; // debug

#include <cstdio>
#include "apic.h"
#include "cpu.h"
#include "kmem.h"

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

PUBLIC static inline NEEDS["apic.h", "cpu.h"]
void
Ipi::send(int logical_cpu, Message m)
{
  Apic::mp_send_ipi(Cpu::cpus.cpu(logical_cpu).phys_id(), (Unsigned8)m);
  stat_sent(logical_cpu);
}

PUBLIC static inline NEEDS["apic.h"]
void
Ipi::bcast(Message m)
{
  Apic::mp_send_ipi(Apic::APIC_IPI_OTHERS, (Unsigned8)m);
}

#if defined(CONFIG_IRQ_SPINNER)
#include "apic.h"

// debug
PRIVATE static
void Ipi::ipi_call_spin()
{
  int cpu = Cpu::p2l(Apic::get_id());
  if (cpu >= 0)
    *(unsigned char*)(Mem_layout::Adap_vram_cga_beg + 22*160 + cpu*+2)
      = '0' + (_count.cpu(cpu)++ % 10);
}
#endif

