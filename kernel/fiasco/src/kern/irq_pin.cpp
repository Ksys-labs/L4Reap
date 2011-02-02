INTERFACE:

#include "types.h"
#include "globals.h"

#include <cstddef>
#include <cstring>

class Irq_pin
{
private:
  enum Flags
  {
    F_enabled = 1,
  };

public:
  bool __mask() { bool o = masked(); _flags &= ~F_enabled; return o; }
  bool __unmask() { bool o = masked(); _flags |= F_enabled; return o; }

public:
  void *operator new (size_t, void *p) { return p; }

  virtual void do_mask() = 0;
  virtual void do_unmask() = 0;
  virtual void do_mask_and_ack() = 0;
  virtual void do_set_mode(unsigned) {};

  virtual void ack() = 0;
  virtual void set_cpu(unsigned) = 0;
  virtual bool check_debug_irq() { return true; }
  virtual void disable() {}
  virtual void unbind_irq() = 0;

  void mask() { if (!__mask()) do_mask(); }
  void mask_and_ack() { do_mask_and_ack(); }
  void unmask() { if (__unmask()) do_unmask(); }
  void set_mode(unsigned m)
  { _flags = (_flags & ~6) | (m & 6); do_set_mode(m); }

  unsigned get_mode() const
  { return _flags & 6; }

  bool masked() const { return !(_flags & F_enabled); }
  Mword flags() const { return _flags; }

  Mword *payload() { return _payload; }

private:
  void __redo_flags()
  {
    do_set_mode(_flags & 6);

    if (masked())
      do_mask();
    else
      do_unmask();
  }

public:
  template<typename Pin>
  void replace()
  { new (this) Pin(); __redo_flags(); }

  template<typename Pin, typename Arg>
  void replace(Arg a)
  { new (this) Pin(a); __redo_flags(); }

  template<typename Pin, typename A1, typename A2>
  void replace(A1 a1, A2 a2)
  { new (this) Pin(a1, a2); __redo_flags(); }

  Mword const *payload() const { return _payload; }

private:
  Mword _flags;
  Mword _payload[1];

};


class Irq_pin_dummy : public Irq_pin
{
public:
  void do_unmask() {}
  void do_mask() {}
  void unbind_irq() {}
  void ack() {}
  void do_mask_and_ack() { __mask(); }
  void do_set_mode(unsigned) {}
  void set_cpu(unsigned) {}
  char const *pin_type() const { return "DUMMY"; }
};

class Kobject_iface;

class Sw_irq_pin : public Irq_pin_dummy
{};

class Irq_base
{
public:
  Irq_base() : _next(0)
  {
    memset(&_pin, 0, sizeof(_pin));
    new (&_pin) Irq_pin_dummy();
  }

  Irq_pin *pin() { return (Irq_pin*)_pin; }
  Irq_pin const *pin() const { return (Irq_pin const*)_pin; }
  virtual void hit() {}

protected:
  typedef char Pin[sizeof (Irq_pin)] __attribute__((aligned(__alignof__(Irq_pin))));
  Pin _pin;

public:
  Irq_base *_next;

  static Irq_base *(*dcast)(Kobject_iface *);
};

//----------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Irq_pin
{
public:
  virtual char const *pin_type() const = 0;
};


//----------------------------------------------------------------------------
IMPLEMENTATION:

#include "types.h"
#include "cpu_lock.h"
#include "lock_guard.h"

Irq_base *(*Irq_base::dcast)(Kobject_iface *);

PUBLIC static inline NEEDS["types.h"]
Irq_base *
Irq_base::self(Irq_pin const *pin)
{
#define MYoffsetof(TYPE, MEMBER) (((size_t) &((TYPE *)10)->MEMBER) - 10)
  return reinterpret_cast<Irq_base*>(reinterpret_cast<Mword>(pin)
      - MYoffsetof(Irq_base, _pin));
#undef MYoffsetof
}

PUBLIC inline NEEDS["lock_guard.h", "cpu_lock.h"]
void
Irq_base::destroy()
{
  Lock_guard<Cpu_lock> g(&cpu_lock);
  pin()->unbind_irq();
  pin()->replace<Sw_irq_pin>();
}

PUBLIC
char const *
Sw_irq_pin::pin_type() const
{ return "SW IRQ"; }

