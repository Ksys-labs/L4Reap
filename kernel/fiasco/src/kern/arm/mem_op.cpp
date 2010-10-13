INTERFACE [arm]:

#include "types.h"

class Mem_op
{
public:
  enum Op_cache
  {
    Op_cache_clean_data        = 0x00,
    Op_cache_flush_data        = 0x01,
    Op_cache_inv_data          = 0x02,
    Op_cache_coherent          = 0x03,
    Op_cache_dma_coherent      = 0x04,
    Op_cache_dma_coherent_full = 0x05,
  };

  enum Op_mem
  {
    Op_mem_read_data     = 0x10,
    Op_mem_write_data    = 0x11,
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
#include "space.h"

PUBLIC static void
Mem_op::arm_mem_cache_maint(int op, void const *start, void const *end)
{
  Context *c = current();

  c->set_mem_op_in_progress(true);

  switch (op)
    {
    case Op_cache_clean_data:
      Mem_unit::clean_dcache(start, end);
      break;

    case Op_cache_flush_data:
      Mem_unit::flush_dcache(start, end);
      break;

    case Op_cache_inv_data:
      Mem_unit::inv_dcache(start, end);
      break;

    case Op_cache_coherent:
      Mem_unit::clean_dcache(start, end);
      Mem::dsb();
      Mem_unit::btc_inv();
      break;

    case Op_cache_dma_coherent:
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
    case Op_cache_dma_coherent_full:
      Mem_unit::flush_dcache();
      Outer_cache::flush();
      break;

    default:
      break;
    };

  c->set_mem_op_in_progress(false);
}

PUBLIC static void
Mem_op::arm_mem_access(Mword *r)
{
  Address  a = r[1];
  unsigned w = r[2];

  if (w > 2)
    return;

  if (!current()->space()->is_user_memory(a, 1 << w))
    return;

  current()->set_mem_op_in_progress(true);

  switch (r[0])
    {
    case Op_mem_read_data:
      switch (w)
	{
	case 0:
	  r[3] = *(unsigned char *)a;
	  break;
	case 1:
	  r[3] = *(unsigned short *)a;
	  break;
	case 2:
	  r[3] = *(unsigned int *)a;
	  break;
	default:
	  break;
	};
      break;

    case Op_mem_write_data:
      switch (w)
	{
	case 0:
	  *(unsigned char *)a = r[3];
	  break;
	case 1:
	  *(unsigned short *)a = r[3];
	  break;
	case 2:
	  *(unsigned int *)a = r[3];
	  break;
	default:
	  break;
	};
      break;

    default:
      break;
    };

  current()->set_mem_op_in_progress(false);
}

extern "C" void sys_arm_mem_op()
{
  Entry_frame *e = current()->regs();
  if (EXPECT_FALSE(e->r[0] & 0x10))
    Mem_op::arm_mem_access(e->r);
  else
    Mem_op::arm_mem_cache_maint(e->r[0], (void *)e->r[1], (void *)e->r[2]);
}
