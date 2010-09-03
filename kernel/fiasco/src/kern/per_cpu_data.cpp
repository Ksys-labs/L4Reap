INTERFACE:

#include "static_init.h"

#define DEFINE_PER_CPU_P(p) __attribute__((section(".per_cpu.data"),init_priority(0xfffe - p)))
#define DEFINE_PER_CPU      DEFINE_PER_CPU_P(9)
#define DEFINE_PER_CPU_LATE DEFINE_PER_CPU_P(19)

class Mapped_allocator;

class Per_cpu_data
{
public:
  static void init_ctors(Mapped_allocator *a);
  static void run_ctors(unsigned cpu);
  static void run_late_ctors(unsigned cpu);
  static bool valid(unsigned cpu);
};

template< typename T >
class Per_cpu : private Per_cpu_data
{
public:
  T const &cpu(unsigned) const;
  T &cpu(unsigned);

  Per_cpu();
  explicit Per_cpu(bool);

};


//---------------------------------------------------------------------------
INTERFACE [!mp]:

EXTENSION
class Per_cpu
{
private:
  T _d;
};


//---------------------------------------------------------------------------
IMPLEMENTATION [!mp]:

IMPLEMENT inline
bool
Per_cpu_data::valid(unsigned cpu)
{
#if defined NDEBUG
  (void)cpu;
  return 1;
#else
  return cpu == 0;
#endif
}

IMPLEMENT inline
template< typename T >
T const &Per_cpu<T>::cpu(unsigned) const { return _d; }

IMPLEMENT inline
template< typename T >
T &Per_cpu<T>::cpu(unsigned) { return _d; }

IMPLEMENT
template< typename T >
Per_cpu<T>::Per_cpu()
{}

IMPLEMENT
template< typename T >
Per_cpu<T>::Per_cpu(bool) : _d(0)
{}

IMPLEMENT
void
Per_cpu_data::init_ctors(Mapped_allocator *)
{
}

IMPLEMENT inline
void
Per_cpu_data::run_ctors(unsigned)
{
  typedef void (*ctor)(void);
  extern ctor __PER_CPU_CTORS_LIST__[];
  extern ctor __PER_CPU_CTORS_END__[];
  for (unsigned i = __PER_CPU_CTORS_LIST__ - __PER_CPU_CTORS_END__; i > 0; --i)
    {
      //printf("Per_cpu: init ctor %u (%p)\n", i-1, &__PER_CPU_CTORS_END__[i-1]);
      __PER_CPU_CTORS_END__[i-1]();
    }
}

IMPLEMENT inline
void
Per_cpu_data::run_late_ctors(unsigned)
{
  typedef void (*ctor)(void);
  extern ctor __PER_CPU_LATE_CTORS_LIST__[];
  extern ctor __PER_CPU_LATE_CTORS_END__[];
  for (unsigned i = __PER_CPU_LATE_CTORS_LIST__ - __PER_CPU_LATE_CTORS_END__; i > 0; --i)
    {
      //printf("Per_cpu: init ctor %u (%p)\n", i-1, &__PER_CPU_LATE_CTORS_END__[i-1]);
      __PER_CPU_LATE_CTORS_END__[i-1]();
    }
}


//---------------------------------------------------------------------------
INTERFACE [mp]:

#include <cstddef>
#include "config.h"

EXTENSION
class Per_cpu
{
private:
  T _d; // __attribute__((aligned(__alignof__(T))));
};

EXTENSION
class Per_cpu_data
{
private:
  struct Ctor
  {
    void (*func)(void *, unsigned);
    void *base;
  };

  class Ctor_vector
  {
  public:
    Ctor_vector() : _len(0), _capacity(10), _v(0) {}
    void push_back(void (*func)(void*,unsigned), void *base, Mapped_allocator *a);
    unsigned len() const { return _len; }
    Ctor const &operator [] (unsigned idx) const { return _v[idx]; }

  private:
    unsigned _len;
    unsigned _capacity;
    Ctor *_v;
  };
protected:
  static long _offsets[Config::Max_num_cpus] asm ("PER_CPU_OFFSETS");
  static unsigned late_ctor_start;
  static Ctor_vector ctors;
  static Mapped_allocator *alloc;
};

//#include <cstdio>
//---------------------------------------------------------------------------
IMPLEMENTATION [mp]:

#include "mapped_alloc.h"
#include <cstring>

long Per_cpu_data::_offsets[Config::Max_num_cpus];
unsigned Per_cpu_data::late_ctor_start;
Per_cpu_data::Ctor_vector Per_cpu_data::ctors INIT_PRIORITY(EARLY_INIT_PRIO);
Mapped_allocator *Per_cpu_data::alloc;

IMPLEMENT
void
Per_cpu_data::Ctor_vector::push_back(void (*func)(void*,unsigned),
                                     void *base, Mapped_allocator *a)
{
  if (!_v)
    _v = (Ctor*)a->unaligned_alloc(1 << _capacity);

  if (_len >= ((1 << _capacity) / sizeof(Ctor)))
    {
      void *b = a->unaligned_alloc(1 << (_capacity+1));
      memcpy(b, _v, 1 << _capacity);
      a->unaligned_free(1 << _capacity, _v);
      _v = (Ctor*)b;
      ++_capacity;
    }

  Ctor &c = _v[_len++];
  c.func = func;
  c.base = base;
}

// the third argument is just there to not have the same type as the normal
// new operator used in standard environments which would clash with these
// (this file is implicitly in unit tests)
inline void *operator new (size_t, void *p, bool) { return p; }
inline void *operator new [] (size_t, void *p, bool) { return p; }

IMPLEMENT inline
bool
Per_cpu_data::valid(unsigned cpu)
{ return cpu < Config::Max_num_cpus && _offsets[cpu] != -1; }

IMPLEMENT inline template< typename T >
T const &Per_cpu<T>::cpu(unsigned cpu) const
{ return *reinterpret_cast<T const *>((char  const *)&_d + _offsets[cpu]); }

IMPLEMENT inline template< typename T >
T &Per_cpu<T>::cpu(unsigned cpu)
{ return *reinterpret_cast<T*>((char *)&_d + _offsets[cpu]); }

IMPLEMENT
template< typename T >
Per_cpu<T>::Per_cpu()
{
  //printf("  Per_cpu<T>() [this=%p])\n", this);
  ctors.push_back(&ctor_wo_arg, this, alloc);
}

IMPLEMENT
template< typename T >
Per_cpu<T>::Per_cpu(bool) : _d(0)
{
  //printf("  Per_cpu<T>(bool) [this=%p])\n", this);
  ctors.push_back(&ctor_w_arg, this, alloc);
}

PRIVATE static
template< typename T >
void Per_cpu<T>::ctor_wo_arg(void *obj, unsigned cpu)
{
  //printf("Per_cpu<T>::ctor_wo_arg(obj=%p, cpu=%u -> %p)\n", obj, cpu, &(reinterpret_cast<Per_cpu<T>*>(obj)->cpu(cpu)));
  new (&reinterpret_cast<Per_cpu<T>*>(obj)->cpu(cpu), true) T();
}

PRIVATE static
template< typename T >
void Per_cpu<T>::ctor_w_arg(void *obj, unsigned cpu)
{
  //printf("Per_cpu<T>::ctor_w_arg(obj=%p, cpu=%u -> %p)\n", obj, cpu, &reinterpret_cast<Per_cpu<T>*>(obj)->cpu(cpu));
  new (&reinterpret_cast<Per_cpu<T>*>(obj)->cpu(cpu), true) T(cpu);
}

IMPLEMENT
void
Per_cpu_data::init_ctors(Mapped_allocator *a)
{
  alloc = a;

  for (unsigned i = 0; i < Config::Max_num_cpus; ++i)
    _offsets[i] = -1;
}

IMPLEMENT inline
void
Per_cpu_data::run_ctors(unsigned cpu)
{
  if (cpu == 0)
    {
      typedef void (*ctor)(void);
      extern ctor __PER_CPU_CTORS_LIST__[];
      extern ctor __PER_CPU_CTORS_END__[];
      for (unsigned i = __PER_CPU_CTORS_LIST__ - __PER_CPU_CTORS_END__; i > 0; --i)
	{
	  //printf("Per_cpu: init ctor %u (%p)\n", i-1, &__PER_CPU_CTORS_END__[i-1]);
	  __PER_CPU_CTORS_END__[i-1]();
	}

      late_ctor_start = ctors.len();
      return;
    }

  for (unsigned i = 0; i < late_ctor_start; ++i)
    ctors[i].func(ctors[i].base, cpu);
}

IMPLEMENT inline
void
Per_cpu_data::run_late_ctors(unsigned cpu)
{
  if (cpu == 0)
    {
      typedef void (*ctor)(void);
      extern ctor __PER_CPU_LATE_CTORS_LIST__[];
      extern ctor __PER_CPU_LATE_CTORS_END__[];
      for (unsigned i = __PER_CPU_LATE_CTORS_LIST__ - __PER_CPU_LATE_CTORS_END__; i > 0; --i)
	{
	  //printf("Per_cpu: init ctor %u (%p)\n", i-1, &__PER_CPU_LATE_CTORS_END__[i-1]);
	  __PER_CPU_LATE_CTORS_END__[i-1]();
	}
      return;
    }

  unsigned c = ctors.len();
  for (unsigned i = late_ctor_start; i < c; ++i)
    ctors[i].func(ctors[i].base, cpu);
}
