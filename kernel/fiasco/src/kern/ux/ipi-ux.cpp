INTERFACE [mp]:

EXTENSION class Ipi
{
public:
  enum Message { Request = 'r', Global_request = 'g', Debug = 'd' };
};

//---------------------------------------------------------------------------
IMPLEMENTATION[!mp]:

//---------------------------------------------------------------------------
IMPLEMENTATION[mp]:

#include <cstdio>

#include "cpu.h"
#include "pic.h"

IMPLEMENT static
void
Ipi::init(unsigned lcpu)
{
  Pic::setup_ipi(lcpu, Cpu::cpus.cpu(lcpu).phys_id());
}

PUBLIC static inline NEEDS[<cstdio>]
void
Ipi::eoi(Message)
{
}

PUBLIC static inline NEEDS[<cstdio>, "pic.h"]
void
Ipi::send(int lcpu, Message m)
{
  Pic::send_ipi(lcpu, m);
}

PUBLIC static inline NEEDS[<cstdio>, "cpu.h", "pic.h"]
void
Ipi::bcast(Message m)
{
  for (unsigned i = 0; i < Config::Max_num_cpus; ++i)
    if (Cpu::online(i))
      Pic::send_ipi(i, m);
}

PUBLIC static
unsigned long
Ipi::gate(unsigned char data)
{
  Message m = (Message)data;

  switch (m)
    {
      case Global_request: return APIC_IRQ_BASE + 2;
      case Request:        return APIC_IRQ_BASE - 1;
      case Debug:          return APIC_IRQ_BASE - 2;
      default:
         printf("Unknown request: %c(%d)\n", m, m);
         break;
    }
  return ~0UL;
}
