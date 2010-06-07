/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/util/env_ns>
#include <l4/re/c/namespace.h>
#include <l4/re/dataspace>
#include <l4/re/c/rm.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/sys/factory.h>
#include <l4/sys/ipc.h>
#include <l4/sys/types.h>
#include <l4/sys/vm.h>
#include <l4/sigma0/sigma0.h>
#include <l4/util/util.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

/*
 * This is a simple example to demonstrate how to
 * startup a Linux in Vm.
 *
 * This example has some prerequisites:
 *
 * 1. The Linux image name: 'rom/vm-linux-image'
 *
 * 2. The Linux image link/start address: 0x9000000
 *
 * 3. The Linux RAM disk name: 'rom/linux-initrd'
 *
 * 4. The VM physical memory: 0x8000000 - 0xc000000
 */

static void error(const char *str)
{
  printf("%s", str);
  exit(-1);
}

static void dump_vm_state(l4_vm_state *state)
{
  printf("pc:%08lx cpsr:%08lx\n",
      state->pc, state->cpsr);
  printf("r0:%08lx r1:%08lx r2:%08lx r3:%08lx\n",
      state->r[0], state->r[1], state->r[2], state->r[3]);
  printf("r4:%08lx r5:%08lx r6:%08lx r7:%08lx\n",
      state->r[4], state->r[5], state->r[6], state->r[7]);
  printf("r8:%08lx r9:%08lx r10:%08lx r11:%08lx\n",
      state->r[8], state->r[9], state->r[10], state->r[11]);
  printf("r12:%08lx\n",
      state->r[12]);
  printf("lr_svc:%08lx sp_svc:%08lx spsr_svc:%08lx\n",
      state->lr_svc, state->sp_svc, state->spsr_svc);
}


enum {
    Ram_base = 0x8000000, // 128 MB
    Ram_size = 0x4000000, // 128 MB
    Start_addr = Ram_base + 0x1000000, // +1MB
    Atag_addr = 0x8000000,
    Initrd_addr = Ram_base + 0x2000000, // +2MB
};

#define ATAG_NONE	0x00000000
#define ATAG_CORE	0x54410001
#define ATAG_MEM	0x54410002
#define ATAG_INITRD2	0x54420005
#define ATAG_CMDLINE	0x54410009

#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

struct tag_header
{
  l4_uint32_t size;
  l4_uint32_t tag;
};

struct tag_core
{
  l4_uint32_t flags;
  l4_uint32_t pagesize;
  l4_uint32_t rootdev;
};

struct tag_mem
{
  l4_uint32_t	size;
  l4_uint32_t	start;
};

struct tag_cmdline
{
  char    cmdline[128];
};

struct tag_initrd
{
  l4_uint32_t start;
  l4_uint32_t size;
};

static struct init_tags
{
  struct tag_header hdr1;
  struct tag_core   core;
  struct tag_header hdr2;
  struct tag_mem    mem;
  struct tag_header hdr3;
  struct tag_cmdline cmdline;
  struct tag_header hdr4;
  struct tag_initrd initrd;
  struct tag_header hdr5;
} init_tags = {
	{ tag_size(tag_core), ATAG_CORE },
	{ 1, L4_PAGESIZE, 0xff },
	{ tag_size(tag_mem), ATAG_MEM },
	{ Ram_size, Ram_base },
	{ tag_size(tag_cmdline), ATAG_CMDLINE },
	{ "root=/dev/ram0 earlyprintk=serial console=ttyAMA0 init=/bin/sh" },
	{ tag_size(tag_initrd), ATAG_INITRD2 },
	{ Initrd_addr, 0 },
	{ 0, ATAG_NONE }
};

int main()
{
  L4::Cap<void> sigma0_cap;
  l4_cap_idx_t vm_cap = l4re_util_cap_alloc();
  L4::Cap<L4Re::Dataspace> vm_ds_cap;
  L4::Cap<L4Re::Dataspace> initrd_cap;
  l4_addr_t vm_image_addr = 0;
  l4_size_t vm_image_size = 0;
  l4_addr_t initrd_addr = 0;
  l4_size_t initrd_size = 0;

  printf("Vmm started\n");

  if (l4_ipc_error(l4_factory_create_vm(l4re_env()->factory, vm_cap), l4_utcb()))
    error("Cannot create vm\n");

  if (!(sigma0_cap = L4Re::Env::env()->get_cap<void>("sigma0")))
    error("Cannot query sigma0 cap\n");

  if (l4sigma0_map_iomem(sigma0_cap.cap(), Ram_base, Ram_base, Ram_size, 1))
    error("Cannot map nonsecure memory\n");

  L4Re::Util::Env_ns ns;
  if (!(vm_ds_cap = ns.query<L4Re::Dataspace>("rom/vm-linux-image-tz")))
    error("Cannot query vm image\n");

  vm_image_size = vm_ds_cap->size();

  if (l4re_rm_attach((void**)&vm_image_addr, l4_round_page(vm_image_size),
		L4RE_RM_SEARCH_ADDR | L4RE_RM_READ_ONLY, vm_ds_cap.cap(), 0, 0))
    error("Cannot attach vm image\n");

  if (!(initrd_cap = ns.query<L4Re::Dataspace>("rom/linux-initrd")))
      error("Cannot query initrd image\n");

  initrd_size = initrd_cap->size();
  
  if (l4re_rm_attach((void**)&initrd_addr, l4_round_page(initrd_size),
      		L4RE_RM_SEARCH_ADDR | L4RE_RM_READ_ONLY, initrd_cap.cap(), 0, 0))
    error("Cannot attach initrd image\n");

  // fixup Initrd size in ATAG structure
  init_tags.initrd.size = initrd_size;

  memcpy((void*)Start_addr, (void*)vm_image_addr, vm_image_size);
  memcpy((void*)Initrd_addr, (void*)initrd_addr, initrd_size);
  memcpy((void*)Atag_addr, (void*)&init_tags, sizeof(init_tags));

  // Initialize Vm state according to Linux's requirements
  l4_vm_state *state;
  posix_memalign((void**)&state, L4_PAGESIZE, sizeof(l4_vm_state));
  state->pc = Start_addr;
  state->cpsr = 0x13;
  state->r[0] = 0;		// R0=0 according to the spec
  state->r[1] = 827;	 	// R1=1 Realview machine architecture
  state->r[2] = Atag_addr;	// R2=2 ATAG pointer
  
  while (1)
    {
      printf("VM run\n");
      l4_umword_t label = 0;
      int ret = l4_error_u(l4_vm_run(vm_cap, l4_fpage((unsigned long)state, 12, 0), &label), l4_utcb());
      if (ret < 0)
	{
	  printf("VM run failed with %d\n", ret);
	  return -1;
	}
      else
	printf("VM exit\n");

      // do nothing but dump the state and sleep a second
      dump_vm_state((l4_vm_state *)l4_utcb_mr_u(l4_utcb()));
      l4_sleep(1000);
    }

  return 0;
}
