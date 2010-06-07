INTERFACE [ppc32 && mpc52xx]:

#include "types.h"

class Irq_base;

EXTENSION class Pic
{
private:
  /** Pic interrupt control registers (incomplete) */
  ///+0x00: Peripheral Interrupt Mask Register
  static Address per_mask() { return _pic_base; }
  ///+0x04: Peripheral Priority & HI/LO Select Register1
  static Address per_prio1() { return _pic_base + 0x04; }
  ///+0x08: Peripheral Priority & HI/LO Select Register2
  static Address per_prio2() { return _pic_base + 0x08; }
  ///+0x0c: Peripheral Priority & HI/LO Select Register3
  static Address per_prio3() { return _pic_base + 0x0c; }
  ///+0x10: External Enable & External Types Register
  static Address ext() { return _pic_base + 0x10; }
  ///+0x14: Critical Priority & Main Inter. Mask Register
  static Address main_mask() { return _pic_base + 0x14; }
  ///+0x18: Main Inter. Priority1
  static Address main_prio1() { return _pic_base + 0x18; }
  ///+0x1c: Main Inter. Priority2
  static Address main_prio2() { return _pic_base + 0x1c; }
  ///+0x24: PerStat, MainStat, CritStat Encoded Registers
  static Address stat() { return _pic_base + 0x24; }


  /** Interrupt lines (sdma is missing) */
  enum Pic_lines
  {
    IRQ_CRIT = 0x0, ///Critical line
    IRQ_MAIN = 0x1, ///Main line
    IRQ_PER  = 0x2, ///Periphal line
    IRQ_SHIFT = 0x4
  };

  enum Pic_num_per_line
  {
    NUM_CRIT = 4,
    NUM_MAIN = 17,
    NUM_PER  = 24,
  };

  /** Interrupt senses */
  enum Pic_sense
  {
    SENSE_LEVEL_HIGH   = 0,
    SENSE_EDGE_RISING  = 1,
    SENSE_EDGE_FALLING = 2,
    SENSE_LEVEL_LOW    = 3
  };

  static Address _pic_base;

public:
  enum { IRQ_MAX  = (IRQ_PER << IRQ_SHIFT) + NUM_PER};
  enum { No_irq_pending = ~0U };
};

//------------------------------------------------------------------------------
IMPLEMENTATION [ppc32 && mpc52xx]:

#include "boot_info.h"
#include "io.h"
#include "irq.h"
#include "irq_pin.h"
#include "irq_chip_generic.h"
#include "mmu.h"
#include "panic.h"
#include "ppc_types.h"
#include "vkey.h"

#include <cassert>
#include <cstdio>

//------------------------------------------------------------------------------
//Irq_pin implementation
//------------------------------------------------------------------------------
class Mpc52xx_pin : public Irq_pin
{
};

PUBLIC
void
Mpc52xx_pin::do_mask()
{
  assert(cpu_lock.test());
  //Pic::disable_locked(irq());
}

PUBLIC
void
Mpc52xx_pin::ack()
{
  assert(cpu_lock.test());
  //Pic::acknowledge_locked(irq());
}

PUBLIC
void
Mpc52xx_pin::do_mask_and_ack()
{
  assert(cpu_lock.test());
  //Pic::disable_locked(irq());
  //Pic::acknowledge_locked(irq());
}

PUBLIC
void
Mpc52xx_pin::do_unmask()
{
  assert(cpu_lock.test());
  //Pic::enable_locked(irq());
}

PUBLIC
void
Mpc52xx_pin::hit()
{
  Irq::self(this)->Irq::hit();
}

PUBLIC
void
Mpc52xx_pin::set_cpu(unsigned)
{}

PUBLIC
void
Mpc52xx_pin::unbind_irq()
{
  mask();
  disable();
  //Irq_chip::hw_chip->free(Irq::self(this), irq());
  replace<Sw_irq_pin>();
}

PUBLIC
bool
Mpc52xx_pin::check_debug_irq()
{
  //return !Vkey::check_(irq());
  return 0;
}

//------------------------------------------------------------------------------
//Pic implementation
//------------------------------------------------------------------------------
Address Pic::_pic_base = 0;

IMPLEMENT FIASCO_INIT
void
Pic::init()
{
  _pic_base = Boot_info::pic_base();
  assert(_pic_base != 0);

  Mmu_guard dcache;
  Io::write_dirty<Unsigned32>(0xffffe000, per_mask());  //disable 0-19
  Io::write_dirty<Unsigned32>(0x00010fff, main_mask()); //disable 15, 20-31

  Unsigned32 ext_val = Io::read_dirty<Unsigned32>(ext());
  ext_val &= 0x00ff0000; //keep sense configuration
  ext_val |= 0x0f000000; //clear IRQ 0-3
  ext_val |= 0x00001000; //enable Master External Enable
  ext_val |= 0x00000f00; //enable all interrupt lines (IRQ 0-3)
  ext_val |= 0x00000001; //CEb route critical interrupt normally
  Io::write_dirty<Unsigned32>(ext_val, ext());

  //zero priority registers
  Io::write_dirty<Unsigned32>(0, per_prio1());
  Io::write_dirty<Unsigned32>(0, per_prio2());
  Io::write_dirty<Unsigned32>(0, per_prio3());
  Io::write_dirty<Unsigned32>(0, main_prio1());
  Io::write_dirty<Unsigned32>(0, main_prio2());

  static Irq_chip_x _ia;
  Irq_chip::hw_chip = &_ia;
}

//------------------------------------------------------------------------------
/**
 * Irq number translations
 */
PUBLIC static inline
unsigned
Pic::irq_num(unsigned line, unsigned pic_num)
{ return (line << IRQ_SHIFT) | pic_num; }


PUBLIC static
int
Pic::get_irq_num(const char *name, const char *type)
{
  Of_device *dev = Boot_info::get_device(name, type);

  if(!dev)
    return -1;

  return (int)irq_num(dev->interrupt[0], dev->interrupt[1]);
}

PRIVATE static inline NEEDS[<cassert>]
unsigned
Pic::pic_line(unsigned irq_num)
{
  unsigned line = irq_num >> IRQ_SHIFT;
  assert(line < 3);
  return line;
}

PRIVATE static inline NEEDS[Pic::pic_line]
unsigned
Pic::pic_num(unsigned irq_num)
{
  unsigned line = pic_line(irq_num);
  unsigned num = irq_num & ~(~0U << IRQ_SHIFT);

  switch(line)
    {
    case IRQ_CRIT:
      assert(num < NUM_CRIT);
      break;
    case IRQ_MAIN:
      assert(num < NUM_MAIN);
      break;
    default:
      assert(num < NUM_PER);
    }

  return num;
}


class Irq_chip_x : public Irq_chip_gen
{
};

PUBLIC
void
Irq_chip_x::setup(Irq_base *irq, unsigned irqnum)
{
  //let assertions trigger
  //pic_num(irqnum);
 // irq->pin()->replace<Mpc52xx_pin>(irqnum);
}

//-------------------------------------------------------------------------------
/**
 * Interface implementation
 */
IMPLEMENT inline
void
Pic::block_locked (unsigned irq_num)
{
  disable_locked(irq_num);
}

IMPLEMENT inline NEEDS["io.h", Pic::pic_line, Pic::pic_num, Pic::set_stat_msb]
void
Pic::acknowledge_locked(unsigned irq_num)
{
  unsigned line = pic_line(irq_num);
  unsigned num  = pic_num(irq_num);

  if((line == IRQ_MAIN && (num >= 1 || num <= 3)) ||
     (line == IRQ_CRIT && num == 0))
    Io::set<Unsigned32>(1U << (27 - num), ext());

  set_stat_msb(irq_num);
}

PRIVATE static inline
void
Pic::dispatch_mask(unsigned irq_num, Address *addr, unsigned *bit_offs)
{
  switch(pic_line(irq_num))
    {
    case IRQ_MAIN:
      *addr = main_mask();
      *bit_offs = 16;
      break;
    case IRQ_PER:
      *addr = per_mask();
      *bit_offs = 31;
      break;
    default:
      panic("%s: Cannot handle IRQ %u", __PRETTY_FUNCTION__, irq_num);
    }
}

PRIVATE static inline
void
Pic::set_stat_msb(unsigned irq_num)
{
  switch(pic_line(irq_num))
    {
    case IRQ_MAIN:
      Io::set<Unsigned32>(0x00200000, stat());
      break;
    case IRQ_PER:
      Io::set<Unsigned32>(0x20000000, stat());
      break;
    default:
      panic("CRIT not implemented");
    }
}

IMPLEMENT inline NEEDS[Pic::pic_num, Pic::dispatch_mask]
void
Pic::disable_locked (unsigned irq_num)
{
  Address addr;
  unsigned bit_offs;
  dispatch_mask(irq_num, &addr, &bit_offs);
  Io::set<Unsigned32>(1U << (bit_offs - pic_num(irq_num)), addr);
}

IMPLEMENT inline NEEDS[Pic::dispatch_mask]
void
Pic::enable_locked (unsigned irq_num, unsigned /*prio*/)
{
  Address addr;
  unsigned bit_offs;
  dispatch_mask(irq_num, &addr, &bit_offs);

  Io::clear<Unsigned32>(1U << (bit_offs - pic_num(irq_num)), addr);
}

PUBLIC static inline NEEDS["panic.h"]
unsigned
Pic::nr_irqs()
{ return IRQ_MAX; }

PRIVATE static inline
Unsigned32
Pic::pending_per(Unsigned32 state)
{
  Unsigned32 irq = state >> 24 & 0x1f; //5 bit

  if(irq  == 0)
    panic("No support for bestcomm interrupt, yet\n");

  return irq_num(IRQ_PER, irq);
}

PRIVATE static inline
Unsigned32
Pic::pending_main(Unsigned32 state)
{
  Unsigned32 irq = state >> 16 & 0x1f;

  //low periphal
  if(irq == 4)
    return pending_per(state);

  return irq_num(IRQ_MAIN, irq);
}

PUBLIC static inline NEEDS[Pic::pending_main, Pic::pending_per]
Unsigned32
Pic::pending()
{
  Unsigned32 irq = No_irq_pending;
  Unsigned32 state = Io::read<Unsigned32>(stat());
  
  //critical interupt
  if(state & 0x00000400)
    panic("No support for critical interrupt, yet");
  //main interrupt
  else if(state & 0x00200000)
    irq = pending_main(state);
  //periphal interrupt
  else if(state & 0x20000000)
    irq = pending_per(state);

  return irq;
}

/**
 * disable interrupt lines [0-3]
 */
IMPLEMENT inline
Pic::Status
Pic::disable_all_save()
{
  Status s;
  Mmu_guard dcache;

  s = Io::read_dirty<Unsigned32>(ext());
  Io::write_dirty<Unsigned32>(s & ~0x1f00, ext());

  return s;
}

IMPLEMENT inline
void
Pic::restore_all(Status s)
{
  Io::write<Unsigned32>(s, ext());
}

IMPLEMENTATION [debug && ppc32]:

PUBLIC
char const *
Mpc52xx_pin::pin_type() const
{ return "HW Mpc52xx IRQ"; }
