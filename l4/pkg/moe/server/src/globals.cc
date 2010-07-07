/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "globals.h"
#include <cstdio>

l4_kernel_info_t *_current_kip;
L4::Cap<void> root_name_space_obj;

static Cap_alloc _cap_allocator __attribute__((init_priority(1400)));
Object_pool __attribute__((init_priority(1401))) object_pool(&_cap_allocator);

char log_buffer[1024];
Moe::Dataspace *kip_ds;

Moe::Server_object::~Server_object()
{
  if (obj_cap().is_valid())
    {
      l4_task_unmap(L4_BASE_TASK_CAP,
	  obj_cap().fpage(L4_FPAGE_RWX), L4_FP_ALL_SPACES);
      //printf("free SO cap: %lx\n", obj_cap().cap());
      object_pool.cap_alloc()->free(this);
      //printf("  hint=%lx\n", object_pool.cap_alloc()->hint());
    }
}

extern char const *const PROG = "moe";
