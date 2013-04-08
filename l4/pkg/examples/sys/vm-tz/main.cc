/*
 * (c) 2009-2012 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/util/env_ns>
#include <l4/re/c/namespace.h>
#include <l4/re/dataspace>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/error_helper>
#include <l4/sys/factory>
#include <l4/sys/vm>
#include <l4/sigma0/sigma0.h>
#include <l4/util/util.h>
#include <l4/sys/cache.h>
#include <l4/vcpu/vcpu>
#include <l4/sys/thread>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

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
  exit(1);
}

static void dump_vm_state(l4_vm_tz_state *s)
{
  printf("pc:%08lx cpsr:%08lx\n",
         s->pc, s->cpsr);
  printf("r0:%08lx r1:%08lx r2:%08lx r3:%08lx\n",
         s->r[0], s->r[1], s->r[2], s->r[3]);
  printf("r4:%08lx r5:%08lx r6:%08lx r7:%08lx\n",
         s->r[4], s->r[5], s->r[6], s->r[7]);
  printf("r8:%08lx r9:%08lx r10:%08lx r11:%08lx\n",
         s->r[8], s->r[9], s->r[10], s->r[11]);
  printf("r12:%08lx\n",
         s->r[12]);
  printf("lr_svc:%08lx sp_svc:%08lx spsr_svc:%08lx\n",
         s->svc.lr, s->svc.sp, s->svc.spsr);
}


enum {
    //Ram_base    = 0x60000000, //E
    Ram_base    = 0x20000000, // T2
    Ram_size    = 0x00100000,
    Start_addr  = Ram_base + 0x8000,
    Atag_addr   = Start_addr + 0x100,
    Initrd_addr = Ram_base + 0x400000, // +2MB
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

static void copyblob(l4_addr_t dst, l4_addr_t src, size_t sz)
{
  assert(dst > Ram_base && dst + sz <= Ram_base + Ram_size);
  memcpy((void *)dst, (void *)src, sz);
  l4_cache_clean_data(dst, dst + sz);
}

static void setup_linux(l4_vm_tz_state *vmstate)
{
  const L4Re::Env *env = L4Re::Env::env();
  L4::Cap<L4Re::Dataspace> vm_ds_cap;
  L4::Cap<L4Re::Dataspace> initrd_cap;
  l4_addr_t vm_image_addr = 0;
  l4_size_t vm_image_size = 0;
  l4_addr_t initrd_addr = 0;
  l4_size_t initrd_size = 0;

  L4Re::Util::Env_ns ns;
  if (!(vm_ds_cap = ns.query<L4Re::Dataspace>("rom/vm-linux-image-tz")))
    error("Cannot query vm image\n");

  vm_image_size = vm_ds_cap->size();

  if (env->rm()->attach(&vm_image_addr, l4_round_page(vm_image_size),
                        L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
                        vm_ds_cap, 0, 0))
    error("Cannot attach vm image\n");

  if (!(initrd_cap = ns.query<L4Re::Dataspace>("rom/linux-initrd")))
      error("Cannot query initrd image\n");

  initrd_size = initrd_cap->size();

  if (env->rm()->attach(&initrd_addr, l4_round_page(initrd_size),
                        L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
                        initrd_cap, 0, 0))
    error("Cannot attach initrd image\n");

  // fixup Initrd size in ATAG structure
  init_tags.initrd.size = initrd_size;

  copyblob(Start_addr,  vm_image_addr, vm_image_size);
  copyblob(Initrd_addr, initrd_addr, initrd_size);
  copyblob(Atag_addr,   (l4_addr_t)&init_tags, sizeof(init_tags));


  // Initialize Vm state according to Linux's requirements

  vmstate->pc = Start_addr;
  vmstate->cpsr = 0x13;
  vmstate->r[0] = 0;            // R0=0 According to the spec
  vmstate->r[1] = 827;	        // R1=1 Machine architecture
  vmstate->r[2] = Atag_addr;    // R2=2 ATAG pointer
}

int main()
{
  L4::Cap<void> sigma0_cap;
  L4::Cap<L4::Vm> vm = L4Re::chkcap(L4Re::Util::cap_alloc.alloc<L4::Vm>());
  const L4Re::Env *env = L4Re::Env::env();

  printf("Vmm started\n");

  L4Re::chksys(env->factory()->create_vm(vm));

  if (!(sigma0_cap = L4Re::Env::env()->get_cap<void>("sigma0")))
    error("Cannot query sigma0 cap\n");

  l4_addr_t v = Ram_base;
  L4Re::chksys(env->rm()->reserve_area(&v, Ram_size, L4Re::Rm::Reserved));

  if (l4sigma0_map_iomem(sigma0_cap.cap(), Ram_base, Ram_base, Ram_size, 1))
    error("Cannot map nonsecure memory\n");

  L4vcpu::Vcpu *vcpu;
  l4_addr_t _vmstate;

  int r = L4vcpu::Vcpu::ext_alloc(&vcpu, &_vmstate);
  if (r)
    {
      printf("Failed to allocate virtualization data structures: %d\n", r);
      return 0;
    }

  l4_vm_tz_state *vmstate = (l4_vm_tz_state *)_vmstate;

  vcpu->task(vm);
  vcpu->state()->set(L4_VCPU_F_FPU_ENABLED);
  vcpu->saved_state()->set(L4_VCPU_F_USER_MODE | L4_VCPU_F_FPU_ENABLED);

  setup_linux(vmstate);

  L4::Cap<L4::Thread> me;
  L4Re::chksys(me->vcpu_control_ext((l4_addr_t)vcpu));

  while (1)
    {
      printf("VM run\n");
      int ret = l4_error(me->vcpu_resume_commit(me->vcpu_resume_start()));
      if (ret < 0)
	{
	  printf("VM run failed with %d\n", ret);
	  return -1;
	}
      else
	printf("VM exit\n");

      // do nothing but dump the state and sleep a second
      dump_vm_state(vmstate);
      l4_sleep(1000);
    }

  return 0;
}
