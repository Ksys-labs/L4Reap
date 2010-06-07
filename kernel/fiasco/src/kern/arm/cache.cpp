INTERFACE [arm]:

class Cache_op
{
public:
  enum
  {
    Op_clean_data        = 0,
    Op_flush_data        = 1,
    Op_inv_data          = 2,
    Op_coherent          = 3,
    Op_dma_coherent      = 4,
    Op_dma_coherent_full = 5,
  };
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include "context.h"
#include "entry_frame.h"
#include "globals.h"
#include "mem.h"
#include "mem_space.h"
#include "mem_unit.h"
#include "outer_cache.h"


PUBLIC static void
Cache_op::arm_cache_maint(int op, void const *start, void const *end)
{
  Context *c = current();

  c->set_cache_op_in_progress(true);

  switch (op)
    {
    case Op_clean_data:
      Mem_unit::clean_dcache(start, end);
      break;

    case Op_flush_data:
      Mem_unit::flush_dcache(start, end);
      break;

    case Op_inv_data:
      Mem_unit::inv_dcache(start, end);
      break;

    case Op_coherent:
      Mem_unit::clean_dcache(start, end);
      Mem::dsb();
      Mem_unit::btc_inv();
      break;

    case Op_dma_coherent:
        {
          Mem_space::Vaddr v = Virt_addr(Address(start));
          Mem_space::Vaddr e = Virt_addr(Address(end));

          Mem_unit::flush_dcache(v, e);
          while (v < e)
            {
              Mem_space::Size phys_size;
              Mem_space::Phys_addr phys_addr;
              unsigned attrs;

              if (   c->mem_space()->v_lookup(v, &phys_addr,
                                              &phys_size, &attrs)
                  && (attrs & Mem_space::Page_user_accessible))
                {
                  Outer_cache::flush(Virt_addr(phys_addr).value(),
                                     Virt_addr(phys_addr).value()
                                     + Virt_size(phys_size).value() - 1,
                                     false);
                }
              v += phys_size;
            }
          Outer_cache::sync();

        }
      break;

    // We might not want to implement this one but single address outer
    // cache flushing can be really slow
    case Op_dma_coherent_full:
      Mem_unit::flush_dcache();
      Outer_cache::flush();
      break;
    };

  c->set_cache_op_in_progress(false);
}

extern "C" void sys_arm_cache_op()
{
  Entry_frame *e = current()->regs();
  Cache_op::arm_cache_maint(e->r[0], (void *)e->r[1], (void *)e->r[2]);
}
