#ifndef TYPES_H__
#define TYPES_H__

#include <stddef.h>
#include "types-arch.h"
#include "std_macros.h"

#ifdef __cplusplus

template< typename a, typename b > inline
a nonull_static_cast( b p )
{
  Address d = reinterpret_cast<Address>
                 (static_cast<a>(reinterpret_cast<b>(10))) - 10;
  return reinterpret_cast<a>( reinterpret_cast<Address>(p) + d);
}

template< typename VALUE, typename TARGET >
class Number
{
public:
  typedef VALUE Value;
  typedef TARGET Target;
  struct Type_conversion_error;

protected:
  Number() {}
  explicit Number(Value v) : _v(v) {}

public:

  static Target create(Value v)
  { return Target(v); }

  Target operator = (Target o)
  { _v = o._v; return *static_cast<Target*>(this); }

  Value value() const { return _v; }
  void set_value(Value v) { _v = v; }

  bool operator < (Target const &o) const { return _v < o._v; }
  bool operator > (Target const &o) const { return _v > o._v; }
  bool operator <= (Target const &o) const { return _v <= o._v; }
  bool operator >= (Target const &o) const { return _v >= o._v; }
  bool operator == (Target const &o) const { return _v == o._v; }
  bool operator != (Target const &o) const { return _v != o._v; }

  operator Type_conversion_error const * () const
  { return (Type_conversion_error const *)_v; }

  Target operator | (Target const &o) const
  { return Target(_v | o._v); }

  Target operator & (Target const &o) const
  { return Target(_v & o._v); }

  Target operator + (Target const &o) const
  { return Target(_v + o._v); }

  Target operator - (Target const &o) const
  { return Target(_v - o._v); }

  Target operator << (unsigned long s) const
  { return Target(_v << s); }

  Target operator >> (unsigned long s) const
  { return Target(_v >> s); }

  void operator += (Target const &o) { _v += o._v; }
  void operator -= (Target const &o) { _v -= o._v; }
  void operator <<= (unsigned long s) { _v <<= s; }
  void operator >>= (unsigned long s) { _v >>= s; }

  Target operator ++ () { return Target(++_v); }
  Target operator ++ (int) { return Target(_v++); }

  Target operator -- () { return Target(--_v); }
  Target operator -- (int) { return Target(_v--); }

  Target trunc(Target const &size) const
  { return Target(_v & ~(size._v - 1)); }

  Target offset(Target const &size) const
  { return Target(_v & (size._v - 1)); }

  static Target from_shift(unsigned char shift)
  {
    if (shift >= (int)sizeof(Value) * 8)
      return Target(Value(1) << Value((sizeof(Value) * 8 - 1)));
    return Target(Value(1) << Value(shift));
  }

protected:
  Value _v;
};

template< int SHIFT >
class Page_addr : public Number<Address, Page_addr<SHIFT> >
{
private:
  typedef Number<Address, Page_addr<SHIFT> > B;

  template< int T >
  class Itt
  {};

  template< int SH >
  Address __shift(Address x, Itt<true>) { return x << SH; }

  template< int SH >
  Address __shift(Address x, Itt<false>) { return x >> (-SH); }

public:
  enum { Shift = SHIFT };

  template< int OSHIFT >
  Page_addr(Page_addr<OSHIFT> o)
  : B(__shift<Shift-OSHIFT>(o.value(), Itt<(OSHIFT < Shift)>()))
  {}

  explicit Page_addr(Address a) : B(a) {}

  Page_addr() {}


};

class Virt_addr : public Page_addr<ARCH_PAGE_SHIFT>
{
public:
  template< int OSHIFT >
  Virt_addr(Page_addr<OSHIFT> o) : Page_addr<ARCH_PAGE_SHIFT>(o) {}

  explicit Virt_addr(unsigned int a) : Page_addr<ARCH_PAGE_SHIFT>(a) {}
  explicit Virt_addr(int a) : Page_addr<ARCH_PAGE_SHIFT>(a) {}
  explicit Virt_addr(long int a) : Page_addr<ARCH_PAGE_SHIFT>(a) {}
  explicit Virt_addr(unsigned long a) : Page_addr<ARCH_PAGE_SHIFT>(a) {}
  explicit Virt_addr(void *a) : Page_addr<ARCH_PAGE_SHIFT>(Address(a)) {}

  Virt_addr() {}
};

typedef Page_addr<ARCH_PAGE_SHIFT> Virt_size;

typedef Page_addr<0> Page_number;
typedef Page_number Page_count;

#endif

/// standard size type
///typedef mword_t size_t;
///typedef signed int ssize_t;

/// momentary only used in UX since there the kernel has a different
/// address space than user mode applications
enum Address_type { ADDR_KERNEL = 0, ADDR_USER = 1, ADDR_UNKNOWN = 2 };

#endif // TYPES_H__

