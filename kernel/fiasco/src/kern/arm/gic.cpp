INTERFACE [arm && pic_gic]:

#include "kmem.h"
#include "irq_chip.h"
#include "irq_pin.h"


class Gic
{
private:
  Address _cpu_base;
  Address _dist_base;

  enum
  {
    DIST_CTRL         = 0x000,
    DIST_CTR          = 0x004,
    DIST_IRQ_SEC      = 0x080,
    DIST_ENABLE_SET   = 0x100,
    DIST_ENABLE_CLEAR = 0x180,
    DIST_PRI          = 0x400,
    DIST_TARGET       = 0x800,
    DIST_CONFIG       = 0xc00,
    DIST_SOFTINT      = 0xf00,

    CPU_CTRL          = 0x00,
    CPU_PRIMASK       = 0x04,
    CPU_BPR           = 0x08,
    CPU_INTACK        = 0x0c,
    CPU_EOI           = 0x10,
    CPU_RUNINT        = 0x14,
    CPU_PENDING       = 0x18,

    CPU_CTRL_ENABLE          = 1,
    CPU_CTRL_USE_FIQ_FOR_SEC = 8,
  };
};

class Gic_pin : public Irq_pin
{
public:
  Gic_pin(unsigned gic_idx, unsigned irq)
  { payload()[0] = (gic_idx << 16) | irq; }

  unsigned irq() const { return payload()[0] & 0xffff; }
  Gic *gic() const { return &_gic[payload()[0] >> 16]; }

  static Gic _gic[];
};

class Gic_cascade_pin : public Gic_pin
{
public:
  Gic_cascade_pin(unsigned gic_idx, unsigned irq)
    : Gic_pin(gic_idx, irq) {}
};

class Gic_cascade_irq : public Irq_base
{
public:
  explicit Gic_cascade_irq(Gic *child_gic, Unsigned32 irq_offset)
    : _child_gic(child_gic), _irq_offset(irq_offset) {}

  Unsigned32 irq_offset() const { return _irq_offset; }
  Gic *child_gic() const { return _child_gic; }
private:
  Gic *_child_gic;
  Unsigned32 _irq_offset;
};

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic]:

#include <cstring>
#include <cstdio>

#include "io.h"
#include "irq.h"
#include "irq_chip_generic.h"
#include "panic.h"
#include "vkey.h"

PUBLIC inline NEEDS["io.h"]
unsigned
Gic::nr_irqs()
{ return ((Io::read<Mword>(_dist_base + DIST_CTR) & 0x1f) + 1) * 32; }

PUBLIC inline NEEDS["io.h"]
bool
Gic::has_sec_ext()
{ return Io::read<Mword>(_dist_base + DIST_CTR) & (1 << 10); }

PUBLIC inline
void Gic::softint_cpu(unsigned callmap, unsigned m)
{
  Io::write<Mword>((callmap & 0xff) << 16 | m, _dist_base + DIST_SOFTINT);
}

PUBLIC inline
void Gic::softint_bcast(unsigned m)
{ Io::write<Mword>(1 << 24 | m, _dist_base + DIST_SOFTINT); }

PUBLIC
void
Gic::init_ap()
{
  Io::write<Mword>(CPU_CTRL_ENABLE, _cpu_base + CPU_CTRL);
  Io::write<Mword>(0xf0, _cpu_base + CPU_PRIMASK);
}

PUBLIC
void
Gic::init(Address cpu_base, Address dist_base)
{
  _cpu_base = cpu_base;
  _dist_base = dist_base;

  Io::write<Mword>(0, _dist_base + DIST_CTRL);

  unsigned num = nr_irqs();
  printf("Number of IRQs available at this GIC: %d\n", num);

  unsigned int intmask = 1 << Proc::cpu_id();
  intmask |= intmask << 8;
  intmask |= intmask << 16;

  for (unsigned i = 32; i < num; i += 16)
    Io::write<Mword>(0, _dist_base + DIST_CONFIG + i * 4 / 16);
  for (unsigned i = 32; i < num; i += 4)
    Io::write<Mword>(intmask, _dist_base + DIST_TARGET + i);
  for (unsigned i = 0; i < num; i += 4)
    Io::write<Mword>(0xa0a0a0a0, _dist_base + DIST_PRI + i);
  for (unsigned i = 0; i < num; i += 32)
    Io::write<Mword>(0xffffffff, _dist_base + DIST_ENABLE_CLEAR + i * 4 / 32);

  Io::write<Mword>(1, _dist_base + DIST_CTRL);

  Io::write<Mword>(CPU_CTRL_ENABLE, _cpu_base + CPU_CTRL);
  Io::write<Mword>(0xf0, _cpu_base + CPU_PRIMASK);

  //enable_tz_support();
}

PUBLIC inline NEEDS["io.h"]
void Gic::disable_locked( unsigned irq )
{ Io::write<Mword>(1 << (irq % 32), _dist_base + DIST_ENABLE_CLEAR + (irq / 32) * 4); }

PUBLIC inline NEEDS["io.h"]
void Gic::enable_locked(unsigned irq, unsigned /*prio*/)
{ Io::write<Mword>(1 << (irq % 32), _dist_base + DIST_ENABLE_SET + (irq / 32) * 4); }

PUBLIC inline NEEDS [Gic::enable_locked]
void Gic::acknowledge_locked( unsigned irq )
{ Io::write<Mword>(irq, _cpu_base + CPU_EOI); }

PUBLIC
void
Gic_pin::unbind_irq()
{
  mask();
  disable();
  Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
void
Gic_pin::do_mask()
{
  assert (cpu_lock.test());
  gic()->disable_locked(irq());
}

PUBLIC
void
Gic_pin::do_mask_and_ack()
{
  assert (cpu_lock.test());
  __mask();
  gic()->disable_locked(irq());
  gic()->acknowledge_locked(irq());
}

PUBLIC
void
Gic_pin::ack()
{
  gic()->acknowledge_locked(irq());
}

PUBLIC
void
Gic_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Gic_pin::do_unmask()
{
  assert (cpu_lock.test());
  gic()->enable_locked(irq(), 0xa);
}

PUBLIC
void
Gic_pin::do_set_mode(unsigned)
{}

PUBLIC
bool
Gic_pin::check_debug_irq()
{
  return !Vkey::check_(irq());
}


PUBLIC static inline
Gic_cascade_irq *
Gic_cascade_irq::self(Irq_pin const *pin)
{
#define MYoffsetof(TYPE, MEMBER) (((size_t) &((TYPE *)10)->MEMBER) - 10)
  return reinterpret_cast<Gic_cascade_irq*>(reinterpret_cast<Mword>(pin)
                - MYoffsetof(Gic_cascade_irq, _pin));
#undef MYoffsetof

}

PUBLIC
void
Gic_cascade_pin::hit()
{
  Unsigned32 num = Gic_cascade_irq::self(this)->child_gic()->pending();
  if (num == 0x3ff)
    return;

  Irq *i = nonull_static_cast<Irq*>(Irq_chip_gen::lookup(num + Gic_cascade_irq::self(this)->irq_offset()));
  i->pin()->hit();

  gic()->acknowledge_locked(irq());
}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && !mp && pic_gic]:

PUBLIC
void
Gic_pin::set_cpu(unsigned)
{}

PUBLIC inline NEEDS["io.h"]
Unsigned32 Gic::pending()
{ return Io::read<Mword>(_cpu_base + CPU_INTACK) & 0x3ff; }

//-------------------------------------------------------------------
IMPLEMENTATION [arm && mp && pic_gic]:

#include "cpu.h"

PUBLIC inline NEEDS["io.h"]
Unsigned32 Gic::pending()
{
  Unsigned32 ack = Io::read<Mword>(_cpu_base + CPU_INTACK);

  // IPIs/SGIs need to take the whole ack value
  if ((ack & 0x3ff) < 16)
    Io::write<Mword>(ack, _cpu_base + CPU_EOI);

  return ack & 0x3ff;
}

PUBLIC inline NEEDS["cpu.h"]
void
Gic::set_cpu(unsigned irq, unsigned cpu)
{
  Mword reg = _dist_base + DIST_TARGET + (irq & ~3);
  Mword val = Io::read<Mword>(reg);

  int shift = (irq % 4) * 8;
  val = (val & ~(0xf << shift)) | (1 << (Cpu::cpus.cpu(cpu).phys_id() + shift));

  Io::write<Mword>(val, reg);
}

PUBLIC
void
Gic_pin::set_cpu(unsigned cpu)
{
  gic()->set_cpu(irq(), cpu);
}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && tz]:

#if 0
PRIVATE
void
Gic::set_irq_nonsecure(unsigned irqnum)
{
  Io::set<Mword>(1 << (irqnum % 32),
                 _dist_base + DIST_IRQ_SEC + ((irqnum & ~31) / 8));
}
#endif

PUBLIC inline NEEDS[<cstdio>]
void
Gic::enable_tz_support()
{
  if (has_sec_ext())
    printf("GIC:Has security extension\n");

  printf("GIC: Signal secure Interrupts as FIQs!\n");
  Io::write<Mword>(CPU_CTRL_ENABLE | CPU_CTRL_USE_FIQ_FOR_SEC,
                   _cpu_base + CPU_CTRL);
}

//-------------------------------------------------------------------
IMPLEMENTATION [arm && pic_gic && !tz]:

PUBLIC inline
void
Gic::enable_tz_support()
{}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug]:

PUBLIC
char const *
Gic_pin::pin_type() const
{ return "HW GIC IRQ"; }
