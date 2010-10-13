INTERFACE:

#include "task.h"

class Vm : public Task
{
public:
  ~Vm() {}

  void invoke(L4_obj_ref obj, Mword rights, Syscall_frame *f, Utcb *utcb) = 0;

  enum Operation
  {
    Vm_run_op = Task::Vm_ops + 0,
  };
};

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
Vm::Vm(Ram_quota *q)
  : Task(Vm_space_factory(), q, L4_fpage(0))
{
}

PUBLIC static
template< typename VM >
slab_cache_anon *
Vm::allocator()
{
  static slab_cache_anon *slabs = new Kmem_slab_simple (sizeof (VM),
                                                        sizeof (Mword),
                                                        "Vm");
  return slabs;
}

PUBLIC static
int
Vm::getpage(Utcb *utcb, L4_msg_tag tag, void **addr)
{
  L4_snd_item_iter item(utcb, tag.words());

  if (EXPECT_FALSE(!tag.items() || !item.next()))
    return -L4_err::EInval;

  L4_fpage page(item.get()->d);

  if (EXPECT_FALSE(   !page.is_mempage()
                   || page.order() < 12))
    return -L4_err::EInval;

  unsigned int page_attribs;
  Mem_space::Phys_addr phys;
  Mem_space::Size size;

  if (EXPECT_FALSE(!current()->space()->mem_space()
                        ->v_lookup(Virt_addr(page.mem_address()),
                                   &phys, &size, &page_attribs)))
    return -L4_err::EInval;

  *addr = (void *)Virt_addr(page.mem_address()).value();

  return 0;
}

PUBLIC
template< typename Vm_impl >
void
Vm::vm_invoke(L4_obj_ref obj, Mword rights, Syscall_frame *f, Utcb *utcb)
{
  if (EXPECT_FALSE(f->tag().proto() != L4_msg_tag::Label_task))
    {
      f->tag(commit_result(-L4_err::EBadproto));
      return;
    }

  switch (utcb->values[0])
    {
    case Vm_run_op:
      f->tag(static_cast<Vm_impl *>(this)->sys_vm_run(f, utcb));
      return;
    default:
      Task::invoke(obj, rights, f, utcb);
      return;
    }
}


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
