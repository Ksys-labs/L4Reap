INTERFACE[ia32,amd64,ux]:

#include "irq_chip.h"
#include "irq_pin.h"


EXTENSION class Dirq_pic_pin
{
public:
  class Chip : public Irq_chip
  {
  public:
    static unsigned vector(unsigned irq)
    {
      return (irq < 0x10) ? irq + 0x20 : irq + 0x30;
    }

    bool reserve(unsigned irq);
    void reset(unsigned irq);
    Irq_base *irq(unsigned irq);
    bool is_free(unsigned irq);
    bool alloc(Irq_base *irq, unsigned irqnum);
    bool free(Irq_base *irq, unsigned irqnum);
    void setup(Irq_base *irq, unsigned irqnum);

    unsigned nr_irqs() const { return 16; }

    bool valloc(Irq_base *irq, unsigned vector);
    static bool vfree(Irq_base *irq, unsigned vector);
    virtual void disable_irq(unsigned vector);

  protected:
    Irq_base *virq(unsigned v);
  };
};


IMPLEMENTATION [ia32,ux]:
enum { Register_arg0 = 0 }; // eax

IMPLEMENTATION [amd64]:
enum { Register_arg0 = 7 }; // rdi

IMPLEMENTATION[ia32,amd64,ux]:

#include <cassert>

#include "cpu_lock.h"
#include "globalconfig.h"
#include "globals.h"
#include "irq_chip.h"
#include "logdefs.h"
#include "thread.h"

#include "idt.h"

class Entry_code
{
private:
  struct
  {
    char push;
    char mov;
    Signed32 irq_adr;
    char jmp;
    Unsigned32 jmp_adr;
    char pad;
  } __attribute__((packed)) _d;

public:
  void irq(Irq_base *irq)
  {
    _d.irq_adr = (Address)irq;
  }

  Irq *irq() const
  { return nonull_static_cast<Irq*>((Irq_base*)((Smword)(_d.irq_adr))); }

  bool is_free() const
  { return !_d.push; }

  void free()
  {
    _d.push = 0;
  }

};

static Entry_code _entry_code[256-0x20];

PUBLIC
void
Entry_code::setup()
{
  extern char __generic_irq_entry[];
  // push %eax/%rdi
  _d.push = 0x50 + Register_arg0;

  // mov imm32, %eax/%rdi
  _d.mov = 0xb8 + Register_arg0;
  irq(0);

  // jmp __generic_irq_entry
  _d.jmp = 0xe9;
  _d.jmp_adr = (Address)__generic_irq_entry - (Address)&_d - 11;
}

/** The corresponding hardware interrupt occurred -- handle it. 
    This method checks whether the attached receiver is ready to receive 
    an IRQ message, and if so, restarts it so that it can pick up the IRQ
    message using ipc_receiver_ready().
 */
#if 0
asm (
    ".global __irq_entry_template     \n"
    ".global __irq_entry_template_end \n"
    ".align 4                         \n"
    "__irq_entry_template:            \n"
    "  pushq %rdi \n"
    "  mov $0, %edi \n"
//    "  push %eax                      \n"
//    "  mov $0, %eax                   \n"
    "  jmp __generic_irq_entry        \n"
    "__irq_entry_template_end:        \n"
    );
#endif

IMPLEMENT
bool
Dirq_pic_pin::Chip::is_free(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  unsigned v = vector(irqn);
  return _entry_code[v -0x20].is_free();
}

IMPLEMENT
Irq_base *
Dirq_pic_pin::Chip::virq(unsigned v)
{
  return _entry_code[v -0x20].irq();
}

IMPLEMENT
Irq_base *
Dirq_pic_pin::Chip::irq(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return 0;

  unsigned v = vector(irqn);

  return virq(v);
}


IMPLEMENT
void
Dirq_pic_pin::Chip::disable_irq(unsigned vector)
{
  extern char entry_int_pic_ignore[];
  Idt::set_entry(vector, Address(&entry_int_pic_ignore), false);
}

IMPLEMENT
bool
Dirq_pic_pin::Chip::alloc(Irq_base *irq, unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  unsigned v = vector(irqn);
  if (valloc(irq, v))
    {
      setup(irq, irqn);
      return true;
    }

  return false;
}

IMPLEMENT
bool
Dirq_pic_pin::Chip::valloc(Irq_base *irq, unsigned v)
{
  if (v >= APIC_IRQ_BASE - 0x10)
    return false;

  if (!_entry_code[v -0x20].is_free())
    return false;

  _entry_code[v -0x20].setup();
  _entry_code[v -0x20].irq(irq);


  // force code to memory before setting IDT entry
  asm volatile ( "" : : : "memory" );

  Idt::set_entry(v, (Address)&_entry_code[v -0x20], false);
  return true;
}

IMPLEMENT
bool
Dirq_pic_pin::Chip::free(Irq_base *irq, unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  unsigned v = vector(irqn);
  return vfree(irq, v);
}

IMPLEMENT
bool
Dirq_pic_pin::Chip::vfree(Irq_base *irq, unsigned v)
{
  if (v >= APIC_IRQ_BASE - 0x10)
    return false;

  if (_entry_code[v -0x20].is_free())
    return false;

  if (_entry_code[v-0x20].irq() != irq)
    return false;

  _entry_code[v-0x20].free();

  Idt::set_entry(v, 0, false);
  return true;
}

IMPLEMENT
void
Dirq_pic_pin::Chip::reset(unsigned irqn)
{
  unsigned v = vector(irqn);
  Idt::set_entry(v, (Address)&_entry_code[v -0x20], false);
}

IMPLEMENT
void
Dirq_pic_pin::Chip::setup(Irq_base *irq, unsigned irqnum)
{
  irq->pin()->replace<Dirq_pic_pin>(irqnum);
}

IMPLEMENT
bool
Dirq_pic_pin::Chip::reserve(unsigned irqn)
{
  if (irqn >= Config::Max_num_dirqs)
    return false;

  unsigned v = vector(irqn);
  if (!_entry_code[v -0x20].is_free())
    return false;

  _entry_code[v -0x20].setup();
  return true;
}

PUBLIC inline
unsigned
Dirq_pic_pin::vector() const
{ return irq() < 0x10 ? irq() + 0x20 : irq() + 0x30; }

extern "C" void entry_int_pic_ignore(void);


PROTECTED
void
Dirq_pic_pin::disable_vector()
{
  unsigned vector = this->vector();
  _entry_code[vector -0x20].free();
}
PUBLIC
void
Dirq_pic_pin::disable()
{
  unsigned vector = this->vector();
  Idt::set_entry(vector, Address(&entry_int_pic_ignore), false);
  _entry_code[vector -0x20].free();
}

#if 0
PUBLIC
void
Dirq_pic_pin::setup()
{
  extern char __generic_irq_entry[];
  unsigned vector = this->vector();

  if (vector == Config::scheduler_irq_vector)
    return;

  // push %eax/%rdi
  _entry_code[vector -0x20][0] = 0x50 + Register_arg0;

  // mov imm32, %eax/%rdi
  _entry_code[vector -0x20][1] = 0xb8 + Register_arg0;
  *(Unsigned32 *)(_entry_code[vector -0x20] + 2) = Mword(Irq::self(this));

  // jmp __generic_irq_entry
  _entry_code[vector -0x20][6] = 0xe9;
  *(Unsigned32 *)(_entry_code[vector -0x20] + 7)
    = (Address)__generic_irq_entry - (Address)_entry_code[vector -0x20] - 11;

  if (vector >= APIC_IRQ_BASE - 0x10)
    {
      printf("could not assign IRQ vector for DIRQ %u\n", irq());
      return;
    }

  // force code to memory before setting IDT entry
  asm volatile ( "" : : : "memory" );

  Idt::set_entry(vector, (Address)_entry_code[vector -0x20], false);
}
#endif

IMPLEMENT FIASCO_INIT
void
Dirq_pic_pin::init()
{
  static Chip _ia;
  Irq_chip::hw_chip = &_ia;
}

