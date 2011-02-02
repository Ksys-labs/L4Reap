INTERFACE:

#include "task.h"

class Vm : public Task
{
public:
  ~Vm() {}

  int resume_vcpu(Context *, Vcpu_state *, bool) = 0;
};

template< typename VM >
struct Vm_allocator
{
  static Kmem_slab_t<VM> a;
};

template<typename VM>
Kmem_slab_t<VM> Vm_allocator<VM>::a("Vm");

// ------------------------------------------------------------------------
IMPLEMENTATION:

#include "cpu.h"

class Mem_space_vm : public Mem_space
{
public:
  Mem_space_vm(Ram_quota *q) : Mem_space(q, false) {}
  virtual Page_number map_max_address() const
  { return Page_number::create(1UL << (MWORD_BITS - Page_shift)); }
};

struct Vm_space_factory
{
  /** Create a usual Mem_space object. */
  template< typename A1 >
  static void create(Mem_space *v, A1 a1)
  { new (v) Mem_space_vm(a1); }

  template< typename S >
  static void create(S *v)
  { new (v) S(); }
};


PUBLIC
Vm::Vm(Ram_quota *q) : Task(Vm_space_factory(), q)
{}

PUBLIC static
template< typename VM >
slab_cache_anon *
Vm::allocator()
{ return &Vm_allocator<VM>::a; }


// ------------------------------------------------------------------------
IMPLEMENTATION [ia32]:

PROTECTED static inline
bool
Vm::is_64bit()
{ return false; }

// ------------------------------------------------------------------------
IMPLEMENTATION [amd64]:

PROTECTED static inline
bool
Vm::is_64bit()
{ return true; }
