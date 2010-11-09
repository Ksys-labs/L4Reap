INTERFACE [arm && kirkwood]:

#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    Main_Irq_cause_low_reg     = Mem_layout::Pic_map_base + 0x20200,
    Main_Irq_mask_low_reg      = Mem_layout::Pic_map_base + 0x20204,
    Main_Fiq_mask_low_reg      = Mem_layout::Pic_map_base + 0x20208,
    Endpoint_irq_mask_low_reg  = Mem_layout::Pic_map_base + 0x2020c,
    Main_Irq_cause_high_reg    = Mem_layout::Pic_map_base + 0x20210,
    Main_Irq_mask_high_reg     = Mem_layout::Pic_map_base + 0x20214,
    Main_Fiq_mask_high_reg     = Mem_layout::Pic_map_base + 0x20218,
    Endpoint_irq_mask_high_reg = Mem_layout::Pic_map_base + 0x2021c,

    Bridge_int_num = 1,

    Multi_irq_pending = 0,
    No_irq_pending = 1023,
  };
};

//-------------------------------------------------------------------
IMPLEMENTATION [arm && kirkwood]:

#include <cstring>
#include <cstdio>

#include "config.h"
#include "initcalls.h"
#include "io.h"
#include "irq.h"
#include "irq_pin.h"
#include "irq_chip_generic.h"
#include "types.h"
#include "vkey.h"

class Kirkwood_pin : public Irq_pin
{
public:
  unsigned irq() const { return payload()[0]; }
};

class Kirkwood_pin_low : public Kirkwood_pin
{
public:
  explicit Kirkwood_pin_low(unsigned irq) { payload()[0] = irq; }
};

class Kirkwood_pin_high : public Kirkwood_pin
{
public:
  explicit Kirkwood_pin_high(unsigned irq) { payload()[0] = irq; }
};


PUBLIC
void
Kirkwood_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Kirkwood_pin_low::do_mask()
{
  assert (cpu_lock.test());
  Io::clear<Unsigned32>(1 << irq(), Pic::Main_Irq_mask_low_reg);
}

PUBLIC
void
Kirkwood_pin_high::do_mask()
{
  assert (cpu_lock.test());
  Io::clear<Unsigned32>(1 << (irq() - 32), Pic::Main_Irq_mask_high_reg);
}

PUBLIC
void
Kirkwood_pin_low::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::clear<Unsigned32>(1 << irq(), Pic::Main_Irq_mask_low_reg);
  // ack is empty
}

PUBLIC
void
Kirkwood_pin_high::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  Io::clear<Unsigned32>(1 << (irq() - 32), Pic::Main_Irq_mask_high_reg);
  // ack is empty
}

PUBLIC
void
Kirkwood_pin::ack()
{
  // ack is empty
}

PUBLIC
void
Kirkwood_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Kirkwood_pin_low::do_unmask()
{
  assert (cpu_lock.test());
  Io::set<Unsigned32>(1 << irq(), Pic::Main_Irq_mask_low_reg);
}

PUBLIC
void
Kirkwood_pin_high::do_unmask()
{
  assert (cpu_lock.test());
  Io::set<Unsigned32>(1 << (irq() - 32), Pic::Main_Irq_mask_high_reg);
}

PUBLIC
bool
Kirkwood_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}

PUBLIC
void
Kirkwood_pin::set_cpu(unsigned)
{
}


class Irq_chip_kirkwood : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_kirkwood::setup(Irq_base *irq, unsigned irqnum)
{
  if (irqnum < 32)
    irq->pin()->replace<Kirkwood_pin_low>(irqnum);
  else if (irqnum < Config::Max_num_dirqs)
    irq->pin()->replace<Kirkwood_pin_high>(irqnum);
}

IMPLEMENT FIASCO_INIT
void Pic::init()
{
  static Irq_chip_kirkwood _ia;
  Irq_chip::hw_chip = &_ia;

  // Disable all interrupts
  Io::write<Unsigned32>(0U, Main_Irq_mask_low_reg);
  Io::write<Unsigned32>(0U, Main_Fiq_mask_low_reg);
  Io::write<Unsigned32>(0U, Main_Irq_mask_high_reg);
  Io::write<Unsigned32>(0U, Main_Fiq_mask_high_reg);

  // enable bridge (chain) IRQ
  Io::set<Unsigned32>(1 << Bridge_int_num, Main_Irq_mask_low_reg);
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all(Status)
{}

PUBLIC static inline NEEDS[<cstdio>,"io.h"]
Unsigned32 Pic::pending()
{
  Unsigned32 v;

  v = Io::read<Unsigned32>(Main_Irq_cause_low_reg);
  if (v & 1)
    {
      v = Io::read<Unsigned32>(Main_Irq_cause_high_reg);
      for (int i = 1; i < 32; ++i)
	if ((1 << i) & v)
	  return 32 + i;
    }
  for (int i = 1; i < 32; ++i)
    if ((1 << i) & v)
      return i;

  return No_irq_pending;
}

PUBLIC static inline
Mword Pic::is_pending(Mword &irqs, Mword irq)
{ return irqs == irq; }

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && kirkwood]:

PUBLIC
char const *
Kirkwood_pin::pin_type() const
{ return "HW Kirkwood IRQ"; }
