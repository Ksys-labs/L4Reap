
#include <l4/sys/ipc_gate>
#include <stdio.h>
#include <unistd.h>
#include <l4/sys/kdebug.h>

#include "globals.h"
#include "page_alloc.h"

extern "C" {
#include <private/gc_priv.h>
}

extern char _etext[];
extern char _end[];
extern void *__libc_stack_end;

extern "C" {

void GC_register_data_segments(void);
ptr_t GC_get_main_stack_base(void);
struct hblk *GC_get_mem(size_t bytes);

}

void GC_register_data_segments(void)
{
  GC_add_roots_inner((ptr_t)&_etext, (ptr_t)&_end, FALSE);
}

ptr_t GC_get_main_stack_base(void)
{
  return (ptr_t)__libc_stack_end;
}



static void GC_default_push_other_roots(void)
{
  l4_msg_regs_t *mr = l4_utcb_mr();
  static l4_umword_t b[2];
  b[0] = mr->mr[0];
  b[1] = mr->mr[1];
  l4_cap_idx_t const start = Cap_alloc::Gc_cap_0 << L4_CAP_SHIFT;
  l4_cap_idx_t const end = (Cap_alloc::Gc_cap_0 + Cap_alloc::Gc_caps) << L4_CAP_SHIFT;
  l4_umword_t o = 0;
  for (l4_cap_idx_t c = start; c < end; c += L4_CAP_OFFSET)
    {
      L4::Cap<L4::Ipc_gate> g(c);
      if (!object_pool.cap_alloc()->is_allocated(g))
	continue;

      if (l4_error(g->get_infos(&o)) < 0)
	continue;

      o &= ~3UL;
      GC_push_one(o);
      //printf("found so @ %lx\n", o);
    }
  //GC_push_one();
  mr->mr[0] = b[0];
  mr->mr[1] = b[1];
}

GC_INNER void (*GC_push_other_roots)(void) = GC_default_push_other_roots;

hblk *GC_get_mem(size_t s)
{
  //printf("MOE: ps=%lx\n", GC_page_size);
  if (s & (GC_page_size - 1))
    {
      //printf("MOE: stupidly sized memory requested\n");
      enter_kdebug("XX");
      return 0;
    }

  //printf("MOE: real_malloc(%zx)\n", s);
  return (hblk*)Single_page_alloc_base::_alloc(Single_page_alloc_base::nothrow, s, GC_page_size);
}

/* Find the page size */
GC_INNER word GC_page_size = L4_PAGESIZE;
GC_INNER GC_bool GC_dirty_maintained = FALSE;

extern "C" GC_INNER void GC_setpagesize(void);
GC_INNER void GC_setpagesize() {}


extern "C" GC_INNER void GC_dirty_init(void);
GC_INNER void GC_dirty_init()
{
  if (GC_print_stats == VERBOSE)
    GC_log_printf("Initializing DEFAULT_VDB...\n");
  //GC_dirty_maintained = TRUE;
}

extern "C" GC_INNER void GC_read_dirty(void);
GC_INNER void GC_read_dirty() {}

extern "C" GC_INNER GC_bool GC_page_was_dirty(struct hblk *h);

GC_INNER GC_bool GC_page_was_dirty(struct hblk *h)
{
  (void)h;
  return TRUE;
}

extern "C" GC_INNER void GC_remove_protection(struct hblk *h, word nblocks,
                                   GC_bool is_ptrfree);
GC_INNER void GC_remove_protection(struct hblk *h, word nblocks,
                                   GC_bool is_ptrfree)
{
  (void)h;
  (void)nblocks;
  (void)is_ptrfree;
}
