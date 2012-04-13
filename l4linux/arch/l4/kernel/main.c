/*
 * Main & misc. file.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#ifdef CONFIG_BLK_DEV_INITRD
#include <linux/initrd.h>
#endif

#include <linux/kprobes.h>

#include <asm/page.h>
#include <asm/setup.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/unistd.h>
#include <asm/generic/l4lib.h>
#include <asm-generic/sections.h>

#include <l4/sys/err.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/thread.h>
#include <l4/sys/factory.h>
#include <l4/sys/scheduler.h>
#include <l4/sys/task.h>
#include <l4/sys/debugger.h>
#include <l4/sys/irq.h>
#include <l4/re/c/mem_alloc.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/debug.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/cap.h>
#include <l4/re/c/namespace.h>
#include <l4/re/env.h>
#include <l4/log/log.h>
#include <l4/util/cpu.h>
#include <l4/util/l4_macros.h>
#include <l4/util/kip.h>
#include <l4/util/atomic.h>
#include <l4/io/io.h>

#include <asm/api/config.h>
#include <asm/api/macros.h>

#include <asm/l4lxapi/thread.h>
#include <asm/l4lxapi/misc.h>
#include <asm/l4lxapi/task.h>
#include <asm/l4lxapi/irq.h>

#include <asm/generic/dispatch.h>
#include <asm/generic/ferret.h>
#include <asm/generic/kthreads.h>
#include <asm/generic/memory.h>
#include <asm/generic/setup.h>
#include <asm/generic/smp.h>
#include <asm/generic/stack_id.h>
#include <asm/generic/stats.h>
#include <asm/generic/tamed.h>
#include <asm/generic/task.h>
#include <asm/generic/upage.h>
#include <asm/generic/cap_alloc.h>
#include <asm/generic/log.h>
#include <asm/generic/util.h>
#include <asm/generic/vcpu.h>
#include <asm/server/server.h>

#include <asm/l4x/exception.h>
#include <asm/l4x/lx_syscalls.h>
#include <asm/l4x/utcb.h>
#include <asm/l4x/upage.h>

#ifdef CONFIG_X86
#include <asm/io.h>
#include <asm/i8259.h>
#ifdef CONFIG_L4_EXTERNAL_RTC
#include <l4/rtc/rtc.h>
#endif
#endif

L4_CV void l4x_external_exit(int);

// -- Configuration check
#ifdef CONFIG_L4_CONFIG_CHECKS
 #ifdef CONFIG_L4_FB_DRIVER
  #if defined(CONFIG_SERIO_I8042) || defined(CONFIG_SERIO_LIBPS2) \
      || defined(CONFIG_VGA_CONSOLE)
   #warning WARNING: L4FB enabled and also CONFIG_SERIO_I8042 or CONFIG_SERIO_LIBPS2 or CONFIG_VGA_CONSOLE
   //#error WARNING: This is usually not wanted.
  #endif

  #ifndef CONFIG_INPUT_EVDEV
   #warning WARNING: L4FB enabled but not CONFIG_INPUT_EVDEV, you probably want to enable this option.
  #endif
  #ifndef CONFIG_INPUT_MOUSEDEV
   #warning WARNING: L4FB enabled but not CONFIG_INPUT_MOUSEDEV, you probably want to enable this option.
  #endif
 #endif


 // this may be a bit harsh but well...
 #if defined(ARCH_x86) && !defined(CONFIG_L4_FB_DRIVER) && \
     !defined(CONFIG_VGA_CONSOLE) && \
     !defined(CONFIG_FRAMEBUFFER_CONSOLE) && \
     !defined(CONFIG_L4_SERIAL_CONSOLE)
  #error L4_FB_DRIVER nor VGA_CONSOLE nor FRAMEBUFFER_CONSOLE nor L4_SERIAL_CONSOLE enabled, choose one.
 #endif

#endif

// --

unsigned long __l4_external_resolver;

L4_EXTERNAL_FUNC(LOG_printf);
L4_EXTERNAL_FUNC(LOG_vprintf);
L4_EXTERNAL_FUNC(LOG_flush);
L4_EXTERNAL_FUNC(l4_sleep);

L4_EXTERNAL_FUNC(l4util_kip_kernel_has_feature);
L4_EXTERNAL_FUNC(l4util_kip_kernel_abi_version);
L4_EXTERNAL_FUNC(l4_kprintf);
L4_EXTERNAL_FUNC(l4x_external_exit);

L4_EXTERNAL_FUNC(l4sys_errtostr);
L4_EXTERNAL_FUNC(l4re_util_cap_alloc);
L4_EXTERNAL_FUNC(l4re_util_cap_free);
L4_EXTERNAL_FUNC(l4re_debug_obj_debug);

L4_EXTERNAL_FUNC(l4re_ds_size);
L4_EXTERNAL_FUNC(l4re_ds_phys);
L4_EXTERNAL_FUNC(l4re_ds_info);
L4_EXTERNAL_FUNC(l4re_ds_copy_in);
L4_EXTERNAL_FUNC(l4re_ds_map_region);

L4_EXTERNAL_FUNC(l4re_ns_query_to_srv);

L4_EXTERNAL_FUNC(l4re_rm_find_srv);
L4_EXTERNAL_FUNC(l4re_rm_show_lists_srv);
L4_EXTERNAL_FUNC(l4re_rm_attach_srv);
L4_EXTERNAL_FUNC(l4re_rm_detach_srv);
L4_EXTERNAL_FUNC(l4re_rm_reserve_area_srv);
L4_EXTERNAL_FUNC(l4re_rm_free_area_srv);

L4_EXTERNAL_FUNC(l4re_ma_alloc_srv);
L4_EXTERNAL_FUNC(l4re_ma_free_srv);

L4_EXTERNAL_FUNC(l4io_request_iomem);
L4_EXTERNAL_FUNC(l4io_request_iomem_region);
L4_EXTERNAL_FUNC(l4io_release_iomem);
L4_EXTERNAL_FUNC(l4io_request_ioport);
L4_EXTERNAL_FUNC(l4io_request_irq);
L4_EXTERNAL_FUNC(l4io_release_irq);
L4_EXTERNAL_FUNC(l4io_search_iomem_region);
L4_EXTERNAL_FUNC(l4io_lookup_device);
L4_EXTERNAL_FUNC(l4io_lookup_resource);
L4_EXTERNAL_FUNC(l4io_iterate_devices);
L4_EXTERNAL_FUNC(l4io_has_resource);

#ifdef CONFIG_L4_USE_L4SHMC
L4_EXTERNAL_FUNC(l4shmc_create);
L4_EXTERNAL_FUNC(l4shmc_attach_to);
L4_EXTERNAL_FUNC(l4shmc_get_chunk_to);
L4_EXTERNAL_FUNC(l4shmc_attach_signal_to);
L4_EXTERNAL_FUNC(l4shmc_connect_chunk_signal);
L4_EXTERNAL_FUNC(l4shmc_wait_chunk_to);
L4_EXTERNAL_FUNC(l4shmc_add_chunk);
L4_EXTERNAL_FUNC(l4shmc_add_signal);
L4_EXTERNAL_FUNC(l4shmc_get_signal_to);
#endif

#ifdef CONFIG_L4_SERVER
L4_EXTERNAL_FUNC(l4x_srv_init);
L4_EXTERNAL_FUNC(l4x_srv_setup_recv);
L4_EXTERNAL_FUNC(l4x_srv_register_c);
#endif

#if defined(CONFIG_X86) && defined(CONFIG_L4_EXTERNAL_RTC)
L4_EXTERNAL_FUNC(l4rtc_get_seconds_since_1970);
#endif

unsigned long upage_addr;
l4_vcpu_state_t *l4x_vcpu_states[NR_CPUS];
EXPORT_SYMBOL(l4x_vcpu_states);

#ifdef CONFIG_X86
struct desc_struct cpu_gdt_table[GDT_ENTRIES];
#ifdef CONFIG_X86_32
struct desc_ptr early_gdt_descr;
struct desc_ptr idt_descr = { .size = IDT_ENTRIES*8-1, .address = (unsigned long)idt_table };
#endif

unsigned l4x_fiasco_gdt_entry_offset;
struct desc_struct boot_gdt;

unsigned long l4x_fixmap_space_start;

void ia32_sysenter_target(void) {}

l4_utcb_t *l4x_utcb_pointer[L4X_UTCB_POINTERS];
#endif /* x86 */

#ifdef ARCH_arm
unsigned long cr_alignment;
unsigned long cr_no_alignment;
#endif

unsigned int  l4x_nr_cpus = 1;
struct l4x_cpu_physmap_struct
{
	unsigned int phys_id;
};
static struct l4x_cpu_physmap_struct l4x_cpu_physmap[NR_CPUS];

unsigned l4x_fiasco_nr_of_syscalls = 3;

#ifdef ARCH_arm
extern void start_kernel(void);
#endif

l4_cap_idx_t l4x_start_thread_id __nosavedata = L4_INVALID_CAP;
l4_cap_idx_t l4x_start_thread_pager_id __nosavedata = L4_INVALID_CAP;

static l4re_ds_t l4x_ds_mainmem __nosavedata;
static l4re_ds_t l4x_ds_isa_dma __nosavedata;
static void *l4x_main_memory_start;
static void *l4x_isa_dma_memory_start;
static l4_addr_t l4x_vmalloc_areaid __nosavedata;
unsigned long l4x_vmalloc_memory_start;
l4_kernel_info_t *l4lx_kinfo;
l4_cap_idx_t l4x_user_gate[NR_CPUS];

l4re_env_t *l4re_global_env;

DEFINE_SPINLOCK(l4x_cap_lock);


int l4x_debug_show_exceptions;
int l4x_debug_show_ghost_regions;
#ifdef CONFIG_L4_DEBUG_STATS
struct l4x_debug_stats l4x_debug_stats_data;
#endif

struct l4x_phys_virt_mem {
	l4_addr_t phys; /* physical address */
	void    * virt; /* virtual address */
	l4_size_t size; /* size of chunk in Bytes */
};

#define L4X_PHYS_VIRT_ADDRS_MAX_ITEMS 20

/* Default memory size */
unsigned long l4x_mainmem_size = CONFIG_L4_MEMSIZE << 20;
//unsigned long l4x_isa_dma_size   = 2 << 20;
unsigned long l4x_isa_dma_size   = 0;

static struct l4x_phys_virt_mem l4x_phys_virt_addrs[L4X_PHYS_VIRT_ADDRS_MAX_ITEMS] __nosavedata;
int l4x_phys_virt_addr_items;

#ifdef CONFIG_L4_CONFIG_CHECKS
static const unsigned long required_kernel_abi_version = 0;
static const char *required_kernel_features[] =
  {
#ifdef ARCH_x86
    "io_prot", "segments",
#endif
  };

static void l4x_configuration_sanity_check(const char *cmdline)
{
	int i;

	for (i = 0; i < sizeof(required_kernel_features)
		          / sizeof(required_kernel_features[0]); i++) {
		if (!l4util_kip_kernel_has_feature(l4lx_kinfo, required_kernel_features[i])) {
			LOG_printf("The running microkernel does not have the\n"
			           "      %s\n"
			           "feature enabled!\n",
			           required_kernel_features[i]);
			while (1)
				enter_kdebug("Microkernel feature missing!");
		}
	}

	if (l4util_kip_kernel_abi_version(l4lx_kinfo) < required_kernel_abi_version) {
		LOG_printf("The ABI version of the microkernel is too low: it has %ld, "
		           "I need %ld\n",
		           l4util_kip_kernel_abi_version(l4lx_kinfo),
		           required_kernel_abi_version);
		while (1)
			enter_kdebug("Stop!");
	}

#ifndef ARCH_arm
	{
		char *p;
		if ((p = strstr(cmdline, "mem="))
		    && (memparse(p + 4, &p) < (16 << 20))) {
			LOG_printf("Minimal 16MB recommended.\n");
			enter_kdebug("Increases value in mem= option.");
		}
	}
#endif

#ifndef CONFIG_L4_SERIAL
	if (strstr(cmdline, "console=ttyLv")) {
		LOG_printf("Console output set to ttyLvx but driver not compiled in.\n");
		enter_kdebug("L4 serial driver not enabled");
	}
#endif
}
#else
static void l4x_configuration_sanity_check(const char *cmdline)
{
	LOG_printf("Configuration checks disabled, you know what you're doing.\n");
}
#endif

static int l4x_check_setup(const char *cmdline)
{
#if defined(ARCH_arm) && defined(CONFIG_SMP)
	if (!l4util_kip_kernel_has_feature(l4lx_kinfo, "armv6plus")) {
		LOG_printf("Running Fiasco is not compiled for v6 "
		           "or better architecture.\nRequired when "
		           "L4Linux is compiled for v6 (e.g. for TLS).\n");
		/* Note, running on a armv5 fiasco is possible when using
		 * TLS trap-emulation (see dispatch.c).
		 */
		return 1;
	}
#endif
	l4x_configuration_sanity_check(cmdline);
	return 0;
}

static void l4x_server_loop(void);

void l4x_v2p_init(void)
{
	l4x_phys_virt_addr_items = 0;
}

void l4x_v2p_add_item(l4_addr_t phys, void *virt, l4_size_t size)
{
	if (l4x_phys_virt_addr_items == L4X_PHYS_VIRT_ADDRS_MAX_ITEMS)
		panic("v2p filled up!");

	l4x_phys_virt_addrs[l4x_phys_virt_addr_items++]
		= (struct l4x_phys_virt_mem){.phys = phys, .virt = virt, .size = size};
}

static void l4x_virt_to_phys_show(void)
{
	int i;
	for (i = 0; i < l4x_phys_virt_addr_items; i++) {
		printk("v = %08lx  p = %08lx   sz = %zx\n",
		       (unsigned long)l4x_phys_virt_addrs[i].virt,
		       l4x_phys_virt_addrs[i].phys,
		       l4x_phys_virt_addrs[i].size);
	}
}

unsigned long l4x_virt_to_phys(volatile void * address)
{
	int i;

	for (i = 0; i < l4x_phys_virt_addr_items; i++) {
		if (l4x_phys_virt_addrs[i].virt <= address &&
		    address < l4x_phys_virt_addrs[i].virt
		              + l4x_phys_virt_addrs[i].size)
			return (address - l4x_phys_virt_addrs[i].virt)
			       + l4x_phys_virt_addrs[i].phys;
	}

	l4x_virt_to_phys_show();
	/* Whitelist: */

	/* Debugging check: don't miss a translation, can give nasty
	 *                  DMA problems */
	l4x_printf("%s: Could not translate virt. address %p\n",
	           __func__, address);
	WARN_ON(1);

	return __pa(address);
}
EXPORT_SYMBOL(l4x_virt_to_phys);


void *l4x_phys_to_virt(unsigned long address)
{
	int i;

	for (i = 0; i < l4x_phys_virt_addr_items; i++) {
		if (l4x_phys_virt_addrs[i].phys <= address &&
		    address < l4x_phys_virt_addrs[i].phys
		              + l4x_phys_virt_addrs[i].size) {
			return (address - l4x_phys_virt_addrs[i].phys)
			       + l4x_phys_virt_addrs[i].virt;
		}
	}

	l4x_virt_to_phys_show();
	/* Whitelist: */
#ifdef ARCH_x86
	if (address < 0x1000 ||
	    address == 0xb8000 ||
	    address == 0xa0000 ||
	    address == 0xc0000)
		return __va(address);
#endif

	/* Debugging check: don't miss a translation, can give nasty
	 *                  DMA problems */
	l4x_printf("%s: Could not translate phys. address 0x%lx\n",
	           __func__, address);
	WARN_ON(1);

	return __va(address);
}
EXPORT_SYMBOL(l4x_phys_to_virt);

#ifdef CONFIG_VGA_CONSOLE
// we may want to make this function available for a broader usage
static int l4x_pagein(unsigned long addr, unsigned long size,
                      int rw)
{
	int err;
	l4_addr_t a;
	unsigned long sz;
	l4_addr_t off;
	unsigned fl;
	l4re_ds_t ds;
	unsigned long map_flags = rw ? L4RE_DS_MAP_FLAG_RW
		                     : L4RE_DS_MAP_FLAG_RO;

	if (size == 0)
		return 0;

	size += addr & ~L4_PAGEMASK;
	size  = l4_round_page(size);
	addr &= L4_PAGEMASK;

	do {
		a = addr;
		sz = 1;

		err = l4re_rm_find(&a, &sz, &off, &fl, &ds);
		if (err < 0)
			break;

		if (sz > size)
			sz = size;

		err = l4re_ds_map_region(ds, off + (addr - a), map_flags,
		                         addr, addr + sz);
		if (err < 0)
			break;

		size -= sz;
		addr += sz;
	} while (size);

	return err;
}
#endif

#ifdef ARCH_x86
static unsigned l4x_x86_orig_utcb_segment;
static inline void l4x_x86_utcb_save_orig_segment(void)
{
	asm volatile ("mov %%fs, %0": "=r" (l4x_x86_orig_utcb_segment));
}
unsigned l4x_x86_utcb_get_orig_segment(void)
{
	return l4x_x86_orig_utcb_segment;
}
#else
static inline void l4x_x86_utcb_save_orig_segment(void) {}
#endif

L4_CV l4_utcb_t *l4_utcb_wrap(void)
{
#if defined(CONFIG_ARM) || defined(CONFIG_X86_64)
	return l4_utcb_direct();
#elif defined(CONFIG_X86_32)
	unsigned long v;
	asm volatile ("mov %%fs, %0": "=r" (v));
	if (v == 0x43 || v == 7) {
		asm volatile("mov %%fs:0, %0" : "=r" (v));
		return (l4_utcb_t *)v;
	}
	return l4x_stack_utcb_get();
#else
#error Add your arch
#endif
}

/*
 * Full fledged name resolving, including cap allocation.
 */
int l4x_re_resolve_name(const char *name, l4_cap_idx_t *cap)
{
	const char *n = name;
	int r;
	l4re_env_cap_entry_t const *entry;
	L4XV_V(f);

	for (; *n && *n != '/'; ++n)
		;

	L4XV_L(f);
	entry = l4re_get_env_cap_l(name, n - name, l4re_env());
	if (!entry) {
		L4XV_U(f);
		return -ENOENT;
	}

	if (!*n) {
		// full name resolved, no slashes, return
		*cap = entry->cap;
		L4XV_U(f);
		return 0;
	}

	if (l4_is_invalid_cap(*cap = l4x_cap_alloc())) {
		L4XV_U(f);
		return -ENOMEM;
	}

	r = l4re_ns_query_srv(entry->cap, n + 1, *cap);
	if (r) {
		l4x_printf("Failed to query name '%s': %s(%d)\n",
		           name, l4sys_errtostr(r), r);
		L4XV_U(f);
		return -ENOENT;
	}

	return 0;
}

int l4x_query_and_get_ds(const char *name, const char *logprefix,
                         l4_cap_idx_t *dscap, void **addr,
                         l4re_ds_stats_t *dsstat)
{
	int ret;
	L4XV_V(f);

	ret = l4x_re_resolve_name(name, dscap);
	if (ret)
		return ret;

	L4XV_L(f);
	if ((ret = l4re_ds_info(*dscap, dsstat)))
		goto out;

	*addr = NULL;
	if (l4re_rm_attach(addr, dsstat->size,
	                   L4RE_RM_SEARCH_ADDR
	                    | L4RE_RM_EAGER_MAP
			    | L4RE_RM_READ_ONLY,
	                   *dscap,
	                   0, L4_PAGESHIFT))
		goto out;

	L4XV_U(f);
	return 0;
out:
	l4x_cap_free(*dscap);
	L4XV_U(f);
	return ret;
}
EXPORT_SYMBOL(l4x_query_and_get_ds);

int l4x_query_and_get_cow_ds(const char *name, const char *logprefix,
                             l4_cap_idx_t *memcap,
                             l4_cap_idx_t *dscap, void **addr,
                             l4re_ds_stats_t *stat,
                             int pass_through_if_rw_src_ds)
{
	int ret;
	L4XV_V(f);

	*memcap = L4_INVALID_CAP;
	*addr = NULL;

	ret = l4x_re_resolve_name(name, dscap);
	if (ret)
		return ret;

	L4XV_L(f);
	if ((ret = l4re_ds_info(*dscap, stat)))
		goto out1;

	if ((stat->flags & L4RE_DS_MAP_FLAG_RW)
	    && pass_through_if_rw_src_ds) {
		// pass-through mode

		if ((ret = l4re_rm_attach(addr, stat->size,
		                          L4RE_RM_SEARCH_ADDR
		                            | L4RE_RM_EAGER_MAP,
		                          *dscap, 0, L4_PAGESHIFT)))
			goto out1;

	} else {
		// cow
		if (l4_is_invalid_cap(*memcap = l4x_cap_alloc()))
			goto out1;

		if ((ret = l4re_ma_alloc(stat->size, *memcap, 0)))
			goto out2;

		if ((ret = l4re_ds_copy_in(*memcap, 0, *dscap, 0, stat->size)))
			goto out3;

		if ((ret = l4re_rm_attach(addr, stat->size,
		                          L4RE_RM_SEARCH_ADDR
		                            | L4RE_RM_EAGER_MAP,
		                          *memcap,
		                          0, L4_PAGESHIFT)))
			goto out3;

		ret = -ENOMEM;
		if (l4re_ds_info(*memcap, stat))
			goto out4;
	}

	L4XV_U(f);
	return 0;

out4:
	if (*addr)
		l4re_rm_detach(*addr);

out3:
	if (l4_is_valid_cap(*memcap))
		l4re_ma_free(*memcap);

out2:
	if (l4_is_valid_cap(*memcap)) {
		l4re_ma_free(*memcap);
		l4re_util_cap_release(*memcap);
	}
out1:
	l4re_util_cap_release(*dscap);
	L4XV_U(f);
	l4x_cap_free(*dscap);
	return ret;
}
EXPORT_SYMBOL(l4x_query_and_get_cow_ds);

int l4x_detach_and_free_ds(l4_cap_idx_t dscap, void *addr)
{
	int r;
	L4XV_V(f);
	L4XV_L(f);
	if ((r = l4re_rm_detach(addr))) {
		LOG_printf("Failed to detach at %p (%d)\n", addr, r);
		return r;
	}

	if ((r = l4_error(l4re_util_cap_release(dscap)))) {
		LOG_printf("Failed to release/unmap cap: %d\n", r);
	}
	L4XV_U(f);

	l4x_cap_free(dscap);
	return 0;
}
EXPORT_SYMBOL(l4x_detach_and_free_ds);

int l4x_detach_and_free_cow_ds(l4_cap_idx_t memcap,
                               l4_cap_idx_t dscap, void *addr)
{
	int r;
	L4XV_V(f);
	L4XV_L(f);
	if ((r = l4re_rm_detach(addr))) {
		LOG_printf("Failed to detach at %p (%d)\n", addr, r);
		return r;
	}

	if ((r = l4re_ma_free(memcap)))
		LOG_printf("Failed to free mem: %d\n", r);

	if ((r = l4_error(l4re_util_cap_release(dscap))))
		LOG_printf("Failed to release/unmap cap: %d\n", r);

	if ((r = l4_error(l4re_util_cap_release(memcap))))
		LOG_printf("Failed to release/unmap cap: %d\n", r);

	L4XV_U(f);
	l4x_cap_free(dscap);
	l4x_cap_free(memcap);
	return 0;
}
EXPORT_SYMBOL(l4x_detach_and_free_cow_ds);

/* ---------------------------------------------------------------- */

typedef void (*at_exit_function_type)(void);

struct cxa_atexit_item {
	void (*f)(void *);
	void *arg;
	void *dso_handle;
};

static struct cxa_atexit_item at_exit_functions[10];
static const int at_exit_nr_of_functions
	= sizeof(at_exit_functions) / sizeof(at_exit_functions[0]);
static int __current_exititem;

static struct cxa_atexit_item *__next_atexit(void)
{
	if (__current_exititem >= at_exit_nr_of_functions) {
		LOG_printf("WARNING: atexit array overflow, increase!\n");
		return 0;
	}
	return &at_exit_functions[__current_exititem++];
}

int __cxa_atexit(void (*f)(void *), void *arg, void *dso_handle)
{
	struct cxa_atexit_item *h = __next_atexit();

	if (!h)
		return -1;

	h->f = f;
	h->arg = arg;
	h->dso_handle = dso_handle;

	return 0;
}

void __cxa_finalize(void *dso_handle)
{
	register int i = __current_exititem;
	while (i) {
		struct cxa_atexit_item *h = &at_exit_functions[--i];
		if (h->f && (dso_handle == 0 || h->dso_handle == dso_handle)) {
			//LOG_printf("Calling func %p\n", h->f);
			h->f(h->arg);
			//LOG_printf("done calling %p.\n", h->f);
			h->f = 0;
		}
	}
}


int atexit(void (*function)(void))
{
	return __cxa_atexit((void (*)(void*))function, 0, 0);
}

void
l4x_linux_main_exit(void)
{
	extern void exit(int);
	LOG_printf("Terminating L4Linux.\n");
	exit(0);
}

/* ---------------------------------------------------------------- */

void l4x_printf(const char *fmt, ...)
{
	va_list list;
	L4XV_V(f);
	va_start(list, fmt);
	L4XV_L(f);
	LOG_vprintf(fmt, list);
	L4XV_U(f);
	va_end(list);
}

/* ---------------------------------------------------------------- */

static inline int l4x_is_writable_area(unsigned long a)
{
	return    ((unsigned long)&_sdata <= a
	           && a < (unsigned long)&_edata)
	       || ((unsigned long)&__init_begin <= a
	           && a < (unsigned long)&__init_end)
	       || ((unsigned long)&__bss_start <= a
	           && a < (unsigned long)&__bss_stop);
}
static int l4x_forward_pf(l4_umword_t addr, l4_umword_t pc, int extra_write)
{
	l4_msgtag_t tag;
	l4_umword_t err;
	l4_utcb_t *u = l4_utcb();

	do {
		l4_msg_regs_t *mr = l4_utcb_mr_u(u);
		mr->mr[0] = addr | (extra_write ? 2 : 0);
		mr->mr[1] = pc;
		mr->mr[2] = L4_ITEM_MAP | addr;
		mr->mr[3] = l4_fpage(addr, L4_LOG2_PAGESIZE, L4_FPAGE_RWX).raw;
		tag = l4_msgtag(L4_PROTO_PAGE_FAULT, 2, 1, 0);
		tag = l4_ipc_call(l4x_start_thread_pager_id, u, tag,
		                  L4_IPC_NEVER);
		err = l4_ipc_error(tag, u);
	} while (err == L4_IPC_SECANCELED || err == L4_IPC_SEABORTED);

	if (unlikely(l4_msgtag_has_error(tag)))
		LOG_printf("Error forwarding page fault: %lx\n",
	                   l4_utcb_tcr_u(u)->error);

	if (unlikely(l4_msgtag_words(tag) > 0
	             && l4_utcb_mr_u(u)->mr[0] == ~0))
		// unresolvable page fault, we're supposed to trigger an
		// exception
		return 0;

	return 1;
}

/*
 * We need to get the lowest virtual address and we can find out in the
 * KIP's memory descriptors. Fiasco-UX on system like Ubuntu set the minimum
 * mappable address to 0x10000 so we cannot just plain begin at 0x1000.
 */
static unsigned long get_min_virt_address(void)
{
	l4_kernel_info_t *kip = l4re_kip();
	struct md_t {
		unsigned long _l, _h;
	};
	struct md_t *md
	  = (struct md_t *)((char *)kip +
			    (kip->mem_info >> ((sizeof(unsigned long) / 2) * 8)));
	unsigned long count
	  = kip->mem_info & ((1UL << ((sizeof(unsigned long)/2)*8)) - 1);
	unsigned long i;

	for (i = 0; i < count; ++i, md++) {
		/* Not a virtual descriptor? */
		if (!(md->_l & 0x200))
			continue;

		/* Return start address of descriptor, there should only be
		 * a single one, so just return the first */
		return md->_l & ~0x3ffUL;
	}

	return 0x1000;
}


/* To get mmap of /dev/mem working, map address space before
 * start of mainmem with a ro page,
 * lets try with this... */
static void l4x_mbm_request_ghost(l4re_ds_t *ghost_ds)
{
	unsigned int i;
	void *addr;

	if (l4_is_invalid_cap(*ghost_ds = l4re_util_cap_alloc())) {
		LOG_printf("%s: Out of caps\n", __func__);
		l4x_exit_l4linux();
	}

	/* Get a page from our dataspace manager */
	if (l4re_ma_alloc(L4_PAGESIZE, *ghost_ds, L4RE_MA_PINNED)) {
		LOG_printf("%s: Can't get ghost page!\n", __func__);
		l4x_exit_l4linux();
	}


	/* Poison the page, this is optional */

	/* Map page RW */
	addr = 0;
	if (l4re_rm_attach(&addr, L4_PAGESIZE, L4RE_RM_SEARCH_ADDR,
	                   *ghost_ds, 0, L4_PAGESHIFT)) {
		LOG_printf("%s: Can't map ghost page\n", __func__);
		l4x_exit_l4linux();
	}

	/* Write a certain value in to the page so that we can
	 * easily recognize it */
	for (i = 0; i < L4_PAGESIZE; i += sizeof(i))
		*(unsigned int *)((unsigned long)addr + i) = 0xcafeface;

	/* Detach it again */
	if (l4re_rm_detach(addr)) {
		LOG_printf("%s: Can't unmap ghost page\n", __func__);
		l4x_exit_l4linux();
	}

}

static void l4x_map_below_mainmem_print_region(l4_addr_t s, l4_addr_t e)
{
	if (!l4x_debug_show_ghost_regions)
		return;
	if (s == ~0UL)
		return;

	LOG_printf("Ghost region: %08lx - %08lx [%4ld]\n", s, e, (e - s) >> 12);
}

static void l4x_map_below_mainmem(void)
{
	unsigned long i;
	l4re_ds_t ds, ghost_ds = L4_INVALID_CAP;
	l4_addr_t off;
	l4_addr_t map_addr;
	unsigned long map_size;
	unsigned flags;
	long ret;
	unsigned long i_inc;
	int map_count = 0, map_count_all = 0;
	l4_addr_t reg_start = ~0UL;

	LOG_printf("Filling lower ptabs...\n");
	LOG_flush();

	/* Loop through free address space before mainmem */
	for (i = get_min_virt_address();
	     i < (unsigned long)l4x_main_memory_start; i += i_inc) {
		map_addr = i;
		map_size = L4_PAGESIZE;
		ret = l4re_rm_find(&map_addr, &map_size, &off, &flags, &ds);
		if (ret == 0) {
			// success, something there
			if (i != map_addr)
				enter_kdebug("shouldn't be, hmm?");
			i_inc = map_size;
			l4x_map_below_mainmem_print_region(reg_start, i);
			reg_start = ~0UL;
			continue;
		}

		if (reg_start == ~0UL)
			reg_start = i;

		i_inc = L4_PAGESIZE;

		if (ret != -L4_ENOENT) {
			LOG_printf("l4re_rm_find call failure: %s(%ld)\n",
			           l4sys_errtostr(ret), ret);
			l4x_exit_l4linux();
		}

		if (!map_count) {
			/* Get new ghost page every 1024 mappings
			 * to overcome a Fiasco mapping db
			 * limitation. */
			l4x_mbm_request_ghost(&ghost_ds);
			map_count = 1014;
		}
		map_count--;
		map_count_all++;
		map_addr = i;
		if (l4re_rm_attach((void **)&map_addr, L4_PAGESIZE,
		                   L4RE_RM_READ_ONLY | L4RE_RM_EAGER_MAP,
		                   ghost_ds, 0, L4_PAGESHIFT)) {
			LOG_printf("%s: Can't attach ghost page at %lx!\n",
			           __func__, i);
			l4x_exit_l4linux();
		}
	}
	l4x_map_below_mainmem_print_region(reg_start, i);
	LOG_printf("Done (%d entries).\n", map_count_all);
	LOG_flush();
}

static void l4x_setup_upage(void)
{
	l4re_ds_t ds;

	if (l4_is_invalid_cap(ds = l4re_util_cap_alloc())) {
		LOG_printf("%s: Cap alloc failed\n", __func__);
		l4x_linux_main_exit();
	}

	if (l4re_ma_alloc(L4_PAGESIZE, ds, L4RE_MA_PINNED)) {
		LOG_printf("Memory request for upage failed\n");
		l4x_linux_main_exit();
	}

	upage_addr = UPAGE_USER_ADDRESS;
	if (l4re_rm_attach((void **)&upage_addr, L4_PAGESIZE,
	                   L4RE_RM_SEARCH_ADDR,
	                   ds, 0, L4_PAGESHIFT)) {
		LOG_printf("Cannot attach upage properly\n");
		l4x_linux_main_exit();
	}

#ifdef ARCH_x86
	memcpy((void *)upage_addr,
	       &vdso32_int80_start,
	       &vdso32_int80_end - &vdso32_int80_start);
#endif
	/* ARM code is copied in traps.c */
}

static void l4x_register_region(const l4re_ds_t ds, void *start,
                                int allow_noncontig, const char *tag)
{
	l4_size_t ds_size;
	l4_addr_t offset = 0;
	l4_addr_t phys_addr;
	l4_size_t phys_size;

	ds_size = l4re_ds_size(ds);

	LOG_printf("%15s: virt: %p to %p [%zu KiB]\n",
	           tag, start, start + ds_size - 1, ds_size >> 10);

	while (offset < ds_size) {
		if (l4re_ds_phys(ds, offset, &phys_addr, &phys_size)) {
			LOG_printf("error: failed to get physical address for %lx.\n",
			           (unsigned long)start + offset);
			break;
		}

		if (!allow_noncontig && phys_size != ds_size)
			LOG_printf("Noncontiguous region for %s\n", tag);

		l4x_v2p_add_item(phys_addr, start + offset, phys_size);

		LOG_printf("%15s: Phys: 0x%08lx to 0x%08lx, Size: %8zu\n",
		           tag, phys_addr,
		           phys_addr + phys_size, phys_size);

		offset += phys_size;
	}
}

/*
 * Register program section(s) for virt_to_phys, at least initdata is used
 * as normal storage later (including DMA usage).
 *
 * Note: we just register the dataspace region where __init_begin is in for
 *       now
 * Note2:
 */
static void l4x_register_pointer_section(void *p_in_addr,
                                         int allow_noncontig,
                                         const char *tag)
{
	l4re_ds_t ds;
	l4_addr_t off;
	l4_addr_t addr;
	unsigned long size;
	unsigned flags;

	addr = (l4_addr_t)p_in_addr;
	size = 1;
	if (l4re_rm_find(&addr, &size, &off, &flags, &ds)) {
		LOG_printf("Cannot anything at %p?!", p_in_addr);
		l4re_rm_show_lists();
		enter_kdebug("l4re_rm_find failed");
		return;
	}

	LOG_printf("%s: addr = %08lx size = %ld\n", __func__, addr, size);

	l4x_register_region(ds, (void *)addr, allow_noncontig, tag);
}

void __init l4x_setup_memory(char *cmdl,
                             unsigned long *main_mem_start,
                             unsigned long *main_mem_size,
                             unsigned long *isa_dma_mem_start,
                             unsigned long *isa_dma_mem_size)
{
	int res;
	char *memstr;
	char *memtypestr;
	unsigned long memory_area_size;
	l4_addr_t memory_area_id = 0;
	l4_addr_t memory_area_addr;
	//l4_size_t poolsize, poolfree;
	l4_uint32_t dm_flags = L4RE_MA_CONTINUOUS | L4RE_MA_PINNED;

	/* See if we find a mem=xxx option in the command line */
	if ((memstr = strstr(cmdl, "mem="))
	    && (res = memparse(memstr + 4, &memstr)))
		l4x_mainmem_size = res;

	if ((memtypestr = strstr(cmdl, "l4memtype="))) {
		memtypestr += 9;
		do {
			memtypestr++;
			if (!strncmp(memtypestr, "floating", 8)) {
				dm_flags &= ~L4RE_MA_PINNED;
				memtypestr += 8;
			} else if (!strncmp(memtypestr, "pinned", 6)) {
				dm_flags |= L4RE_MA_PINNED;
				memtypestr += 6;
			} else if (!strncmp(memtypestr, "distributed", 11)) {
				dm_flags &= ~L4RE_MA_CONTINUOUS;
				memtypestr += 11;
			} else if (!strncmp(memtypestr, "continuous", 10)) {
				dm_flags |= L4RE_MA_CONTINUOUS;
				memtypestr += 10;
			} else {
				LOG_printf("Unknown l4memtype option, use:\n"
				           "  floating, pinned, distributed, continuous\n"
				           "as a comma separated list\n");
				l4x_exit_l4linux();
			}
		} while (*memtypestr == ',');
	}

	if ((l4x_mainmem_size % L4_SUPERPAGESIZE) == 0) {
		LOG_printf("%s: Forcing superpages for main memory\n", __func__);
		/* force ds-mgr to allocate superpages */
		dm_flags |= L4RE_MA_SUPER_PAGES;
	}

	/* Allocate main memory */
	if (l4_is_invalid_cap(l4x_ds_mainmem = l4re_util_cap_alloc())) {
		LOG_printf("%s: Out of caps\n", __func__);
		l4x_exit_l4linux();
	}
	LOG_printf("l4re_global_env: %p\n", l4re_global_env);
	if (l4re_ma_alloc(l4x_mainmem_size, l4x_ds_mainmem, dm_flags)) {
		LOG_printf("%s: Can't get main memory of %ldMB!\n",
		           __func__, l4x_mainmem_size >> 20);
		l4re_debug_obj_debug(l4re_env()->mem_alloc, 0);
		l4x_exit_l4linux();
	}

#if 0
	/* if there's a '+' at the end of the mem=-option try to get
	 * more memory */
	if (memstr && *memstr == '+') {
		unsigned long chunksize = 64 << 20;

		while (chunksize > (4 << 20)) {
			if ((res = l4dm_mem_resize(&l4x_ds_mainmem,
						   l4x_mainmem_size
			                           + chunksize)))
				chunksize >>= 1; /* failed */
			else
				l4x_mainmem_size += chunksize;
		}
	}
#endif

	LOG_printf("Main memory size: %ldMB\n", l4x_mainmem_size >> 20);
	if (l4x_mainmem_size < (4 << 20)) {
		LOG_printf("Not enough main memory - aborting!\n");
		l4x_exit_l4linux();
	}

	memory_area_size = l4x_mainmem_size;

#ifdef CONFIG_ZONE_DMA
	/* Try to get ISA DMA memory */

	/* See if we find a memisadma=xxx option in the command line */
	if ((memstr = strstr(cmdl, "memisadma="))) {
		LOG_printf("'memisadma' currently not supported\n");
		//l4x_isa_dma_size = memparse(memstr + 10, &memstr);
	}

#if 0
	/** In the default config dm_phys prints out messages if
	 ** the allocation fails, so query the size separately to
	 ** not disturb users with error messages */
	if (l4x_isa_dma_size
	    && (res = l4dm_memphys_poolsize(L4DM_MEMPHYS_ISA_DMA9,
	                                    &poolsize, &poolfree))) {
		LOG_printf("Cannot query ISA DMA pool size: %s(%d)\n",
		           l4sys_errtostr(res), res);
		l4x_exit_l4linux();
	}
	if (l4x_isa_dma_size
	    && poolfree >= l4x_isa_dma_size
	    && !l4dm_memphys_open(L4DM_MEMPHYS_ISA_DMA9,
	                          L4DM_MEMPHYS_ANY_ADDR9,
	                          l4x_isa_dma_size, L4_PAGESIZE,
	                          L4DM_CONTIGUOUS9, "L4Linux ISA DMA memory",
	                          &l4x_ds_isa_dma)) {
		LOG_printf("Got %lukB of ISA DMA memory.\n",
		           l4x_isa_dma_size >> 10);
		memory_area_size += l4_round_size(l4x_isa_dma_size, L4_LOG2_SUPERPAGESIZE);
	} else
#endif
#endif
		l4x_ds_isa_dma = L4_INVALID_CAP;

	/* Get contiguous region in our virtual address space to put
	 * the dataspaces in */
	if (l4re_rm_reserve_area(&memory_area_id, memory_area_size,
	                         L4RE_RM_SEARCH_ADDR,
	                         L4_SUPERPAGESHIFT)) {
		LOG_printf("Error reserving memory area\n");
		l4x_exit_l4linux();
	}
	memory_area_addr = memory_area_id;

	/* Attach data spaces to local address space */
	/** First: the ISA DMA memory */
#ifdef CONFIG_ZONE_DMA
#if 0
	if (!l4_is_invalid_cap(l4x_ds_isa_dma)) {
		l4x_isa_dma_memory_start = (void *)memory_area_addr;
		l4x_main_memory_start =
			(void *)(l4x_isa_dma_memory_start
		                 + l4_round_size(l4x_isa_dma_size, L4_LOG2_SUPERPAGESIZE));
		if ((res = l4rm_area_attach_to_region(&l4x_ds_isa_dma,
	                                              memory_area_id,
	                                              l4x_isa_dma_memory_start,
	                                              l4x_isa_dma_size, 0,
	                                              L4DM_RW | L4RM_MAP))) {
			LOG_printf("Error attaching to ISA DMA memory: %s(%d)\n",
				   l4sys_errtostr(res), res);
			l4x_exit_l4linux();
		}
	} else
#endif
#endif
		l4x_main_memory_start = (void *)memory_area_addr;

	/** Second: the main memory */
	if (l4re_rm_attach(&l4x_main_memory_start, l4x_mainmem_size,
	                   L4RE_RM_IN_AREA | L4RE_RM_EAGER_MAP,
	                   l4x_ds_mainmem, 0,
	                   MAX_ORDER + PAGE_SHIFT)) {
		LOG_printf("Error attaching to L4Linux main memory\n");
		l4x_exit_l4linux();
	}

	/* Release area ... make possible hole available again */
	if (l4re_rm_free_area(memory_area_id)) {
		LOG_printf("Error releasing area\n");
		l4x_exit_l4linux();
	}

	*main_mem_start = (unsigned long)l4x_main_memory_start;
	*main_mem_size  = l4x_mainmem_size;

	if (l4_is_invalid_cap(l4x_ds_isa_dma))
		*isa_dma_mem_start = *isa_dma_mem_size = 0;
	else {
		*isa_dma_mem_start = (unsigned long)l4x_isa_dma_memory_start;
		*isa_dma_mem_size  = l4x_isa_dma_size;
	}

	if (!l4_is_invalid_cap(l4x_ds_isa_dma))
		l4x_register_region(l4x_ds_isa_dma, l4x_isa_dma_memory_start,
		                      0, "ISA DMA memory");
	l4x_register_region(l4x_ds_mainmem, l4x_main_memory_start,
	                    0, "Main memory");

	/* Reserve some part of the virtual address space for vmalloc */
	l4x_vmalloc_memory_start = (unsigned long)l4x_main_memory_start;
	if (l4re_rm_reserve_area(&l4x_vmalloc_memory_start,
#ifdef CONFIG_X86_32
	                          __VMALLOC_RESERVE,
#elif defined(CONFIG_X86_64)
				  VMALLOC_END - VMALLOC_START + 1,
#else
	                          VMALLOC_SIZE << 20,
#endif
	                          L4RE_RM_SEARCH_ADDR, PGDIR_SHIFT)) {
		LOG_printf("%s: Error reserving vmalloc memory!\n", __func__);
		l4x_exit_l4linux();
	}
	l4x_vmalloc_areaid = l4x_vmalloc_memory_start;
#ifdef ARCH_arm
	{
		extern void * /*__initdata*/ vmalloc_min;
		vmalloc_min = (void *)l4x_vmalloc_memory_start;
	}
#endif

#ifdef ARCH_x86
	// fixmap area
	l4x_fixmap_space_start = 0x100000; // not inside BIOS region
	if (l4re_rm_reserve_area(&l4x_fixmap_space_start,
	                         __end_of_permanent_fixed_addresses * PAGE_SIZE,
	                         L4RE_RM_SEARCH_ADDR, PAGE_SHIFT) < 0) {
		LOG_printf("%s: Failed reserving fixmap space!\n", __func__);
		l4x_exit_l4linux();
	}
	__FIXADDR_TOP = l4x_fixmap_space_start
	                 + __end_of_permanent_fixed_addresses * PAGE_SIZE;
#endif

#ifdef CONFIG_X86_64
	{
		unsigned long a = VMEMMAP_START;
		int npages;
		extern char vdso_start[], vdso_end[];

		if (l4re_rm_reserve_area(&a, 100 << 20, 0, 20)) {
			LOG_printf("Failed reserving vmemmap space!\n");
			l4x_exit_l4linux();
		}

		// see arch/x86/vdso/vma.c
		a = 4UL << 30;
		npages = (vdso_end - vdso_start + PAGE_SIZE - 1) / PAGE_SIZE;
		if (l4re_rm_reserve_area(&a, npages << PAGE_SHIFT,
		                         0, L4_SUPERPAGESHIFT)) {
			LOG_printf("VDSO area reservation failed\n");
			l4x_exit_l4linux();
		}
	}
#endif

	l4x_map_below_mainmem();

	// that happened with some version of ld...
	if ((unsigned long)&_end < 0x100000)
		LOG_printf("_end == %p, unreasonable small\n", &_end);

	l4x_register_pointer_section((void *)((unsigned long)&_end - 1), 0, "end");
}

#ifdef CONFIG_X86
#include <asm/l4lxapi/memory.h>

void l4x_update_mapping(unsigned long addr)
{
#ifdef CONFIG_X86_32
	pte_t *ptep;

	/* currently only used for fixmaps */
	if (l4x_fixmap_space_start > addr
	    || addr >= l4x_fixmap_space_start + __end_of_permanent_fixed_addresses * PAGE_SIZE)
		return;

	ptep = lookup_pte(swapper_pg_dir, addr);

	if (ptep && pte_val(*ptep))
		l4lx_memory_map_virtual_page(addr, pte_val(*ptep) & PAGE_MASK,
		                             1);
	else
		l4lx_memory_unmap_virtual_page(addr);
#else
	printk("%s %d\n", __func__, __LINE__); 
#endif
}
#endif

#ifdef ARCH_arm
static void setup_module_area(void)
{
	l4_addr_t start = MODULES_VADDR;
	if (l4re_rm_reserve_area(&start, MODULES_END - MODULES_VADDR,
	                         0, PGDIR_SHIFT))
		LOG_printf("Could not reserve module area, modules won't work!\n");
}

/* Sanity check for hard-coded values used in dispatch.c */
static int l4x_sanity_check_kuser_cmpxchg(void)
{
#if __LINUX_ARM_ARCH__ < 6
	extern char l4x_kuser_cmpxchg_critical_start[];
	extern char l4x_kuser_cmpxchg_critical_end[];

	if (   (unsigned long)l4x_kuser_cmpxchg_critical_start != L4X_UPAGE_KUSER_CMPXCHG_CRITICAL_START
	    || (unsigned long)l4x_kuser_cmpxchg_critical_end   != L4X_UPAGE_KUSER_CMPXCHG_CRITICAL_END) {
		LOG_printf("__kuser_cmpxchg value incorrect; Aborting.\n");
		return 1;
	}
#endif
	return 0;
}

#endif

unsigned long l4x_get_isa_dma_memory_end(void)
{
	if (l4x_isa_dma_memory_start)
		return (unsigned long)l4x_isa_dma_memory_start + l4x_isa_dma_size;
	return 0;
}

static void l4x_create_ugate(l4_cap_idx_t forthread, unsigned cpu)
{
	l4_msgtag_t r;

	l4x_user_gate[cpu] = l4x_cap_alloc();
	if (l4_is_invalid_cap(l4x_user_gate[cpu]))
		LOG_printf("Error getting cap\n");
	r = l4_factory_create_gate(l4re_env()->factory,
	                           l4x_user_gate[cpu],
	                           forthread, 0x10);
	if (l4_error(r))
		LOG_printf("Error creating user-gate %d, error=%lx\n",
		           cpu, l4_error(r));

	{
		char n[14];
		snprintf(n, sizeof(n), "l4x ugate-%d", cpu);
		n[sizeof(n) - 1] = 0;
#ifdef CONFIG_L4_DEBUG_REGISTER_NAMES
		l4_debugger_set_object_name(l4x_user_gate[cpu], n);
#endif
	}
}

#ifdef CONFIG_HOTPLUG_CPU
void l4x_destroy_ugate(unsigned cpu)
{
	l4_msgtag_t t;
	L4XV_V(f);
	L4XV_L(f);
	t = l4_task_unmap(L4_BASE_TASK_CAP,
	                  l4_obj_fpage(l4x_user_gate[cpu], 0, L4_FPAGE_RWX),
	                  L4_FP_ALL_SPACES);
	if (l4_error(t))
		LOG_printf("Error destroying user-gate%d\n", cpu);
	L4XV_U(f);
	l4x_cap_free(l4x_user_gate[cpu]);
	l4x_user_gate[cpu] = L4_INVALID_CAP;
}
#endif

unsigned l4x_cpu_physmap_get_id(unsigned lcpu)
{
	return l4x_cpu_physmap[lcpu].phys_id;
}

static l4lx_thread_t l4x_cpu_threads[NR_CPUS];

l4lx_thread_t l4x_cpu_thread_get(int cpu)
{
	BUG_ON(cpu >= NR_CPUS);
	return l4x_cpu_threads[cpu];
}

l4_cap_idx_t l4x_cpu_thread_get_cap(int cpu)
{
	return l4lx_thread_get_cap(l4x_cpu_thread_get(cpu));
}

static void l4x_cpu_thread_set(int cpu, l4lx_thread_t tid)
{
	BUG_ON(cpu >= NR_CPUS);
	l4x_cpu_threads[cpu] = tid;
}

#ifdef CONFIG_SMP

#ifdef ARCH_x86
void l4x_load_percpu_gdt_descriptor(struct desc_struct *gdt)
{
	long r;
#ifdef CONFIG_L4_VCPU
	if ((r = fiasco_gdt_set(L4_INVALID_CAP,
	                        &gdt[GDT_ENTRY_PERCPU], 8, 2, l4_utcb())) < 0)
		LOG_printf("GDT setting failed: %ld\n", r);
	asm("mov %0, %%fs"
	    : : "r" ((l4x_fiasco_gdt_entry_offset + 2) * 8 + 3) : "memory");
#else
	if ((r = fiasco_gdt_set(l4x_stack_id_get(),
	                        &gdt[GDT_ENTRY_PERCPU], 8, 0, l4_utcb())) < 0)
		LOG_printf("GDT setting failed: %ld\n", r);
	asm("mov %0, %%fs"
	    : : "r" (l4x_fiasco_gdt_entry_offset * 8 + 3) : "memory");
#endif
}
#endif

static int l4x_cpu_check_pcpu(unsigned pcpu, l4_umword_t max_cpus)
{
	return pcpu < max_cpus
	       && l4_scheduler_is_online(l4re_env()->scheduler, pcpu);
}

static struct task_struct *l4x_cpu_idler[NR_CPUS] = { &init_task, 0, };

int l4x_cpu_cpu_get(void)
{
	int i = 0;
	l4_cap_idx_t id = l4x_stack_id_get();

	for (; i < NR_CPUS; i++)
		if (l4x_cpu_threads[i] &&
		    l4_capability_equal(id, l4lx_thread_get_cap(l4x_cpu_threads[i])))
			return i;

	BUG();
}

struct task_struct *l4x_cpu_idle_get(int cpu)
{
	BUG_ON(cpu >= NR_CPUS);
	return l4x_cpu_idler[cpu];
}

#ifndef CONFIG_L4_VCPU
static l4lx_thread_t l4x_cpu_ipi_threads[NR_CPUS];
#endif
static l4_cap_idx_t  l4x_cpu_ipi_irqs[NR_CPUS];
static l4_umword_t   l4x_cpu_ipi_vector_mask[NR_CPUS];

#ifndef CONFIG_L4_VCPU
#include <asm/generic/sched.h>
#include <asm/generic/do_irq.h>
#include <l4/sys/irq.h>
static __noreturn L4_CV void l4x_cpu_ipi_thread(void *x)
{
	unsigned cpu = *(unsigned *)x;
	l4_msgtag_t tag;
	l4_cap_idx_t cap;
	int err;
	l4_utcb_t *utcb = l4_utcb();

	l4x_prepare_irq_thread(current_thread_info(), cpu);

	// wait until parent has setup our cap and attached us
	while (l4_is_invalid_cap(l4x_cpu_ipi_irqs[cpu])) {
		mb();
		l4_sleep(2);
	}

	cap = l4x_cpu_ipi_irqs[cpu];

	while (1) {
		tag = l4_irq_receive(cap, L4_IPC_NEVER);
		if (unlikely(err = l4_ipc_error(tag, utcb)))
			LOG_printf("ipi%d: ipc error %d (cap: %lx)\n",
			           cpu, err, cap);

		while (1) {
			l4_umword_t ret, mask;
			unsigned vector;
			do {
				mask = l4x_cpu_ipi_vector_mask[cpu];
				ret = l4util_cmpxchg(&l4x_cpu_ipi_vector_mask[cpu],
				                     mask, 0);
			} while (unlikely(!ret));

			if (!mask)
				break;

			while (mask) {
				vector = ffs(mask) - 1;
				l4x_do_IPI(vector, current_thread_info());
				mask &= ~(1 << vector);
			}
		}
	}
}
#else /* vcpu */
void l4x_vcpu_handle_ipi(struct pt_regs *regs)
{
	unsigned cpu = smp_processor_id();

	while (1) {
		l4_umword_t ret, mask;
		unsigned vector;
		do {
			mask = l4x_cpu_ipi_vector_mask[cpu];
			ret = l4util_cmpxchg(&l4x_cpu_ipi_vector_mask[cpu],
			                     mask, 0);
		} while (unlikely(!ret));

		if (!mask)
			break;

		while (mask) {
			vector = ffs(mask) - 1;
			l4x_smp_process_IPI(vector, regs);
			mask &= ~(1 << vector);
		}
	}
}
#endif


void l4x_cpu_ipi_setup(unsigned cpu)
{
	l4_msgtag_t t;
	l4_cap_idx_t c;
	char s[14];
	L4XV_V(f);

	snprintf(s, sizeof(s), "l4lx.ipi%d", cpu);
	s[sizeof(s)-1] = 0;

	l4x_cpu_ipi_irqs[cpu] = L4_INVALID_CAP;

#ifndef CONFIG_L4_VCPU
	l4x_cpu_ipi_threads[cpu]
		= l4lx_thread_create(l4x_cpu_ipi_thread,
	                             cpu,
	                             NULL, &cpu, sizeof(cpu),
	                             l4lx_irq_prio_get(0), 0, s);

	if (!l4lx_thread_is_valid(l4x_cpu_ipi_threads[cpu])) {
		LOG_printf("Failed to create thread %s\n", s);
		l4x_exit_l4linux();
	}
#endif

	L4XV_L(f);

	c = l4x_cap_alloc();
	if (l4_is_invalid_cap(c)) {
		LOG_printf("Failed to alloc cap\n");
		l4x_exit_l4linux();
	}

	t = l4_factory_create_irq(l4re_env()->factory, c);
	if (l4_error(t)) {
		LOG_printf("Failed to create IRQ\n");
		l4x_exit_l4linux();
	}

#ifdef CONFIG_L4_DEBUG_REGISTER_NAMES
	l4_debugger_set_object_name(c, s);
#endif

#ifdef CONFIG_L4_VCPU
	t = l4_irq_attach(c, L4X_VCPU_IRQ_IPI << 2,
	                  l4lx_thread_get_cap(l4x_cpu_threads[cpu]));
#else
	t = l4_irq_attach(c, 0,
	                  l4lx_thread_get_cap(l4x_cpu_ipi_threads[cpu]));
#endif
	if (l4_error(t)) {
		LOG_printf("Failed to attach IPI IRQ%d\n", cpu);
		l4x_exit_l4linux();
	}
	L4XV_U(f);

	// now commit and make cap visible to child, it can run now
	mb();
	l4x_cpu_ipi_irqs[cpu] = c;
}

void l4x_cpu_ipi_stop(unsigned cpu)
{
	l4_msgtag_t t;
	L4XV_V(f);

	L4XV_L(f);
	t = l4_irq_detach(l4x_cpu_ipi_irqs[cpu]);
	if (l4_error(t)) {
		LOG_printf("Failed to detach for IPI IRQ%d\n", cpu);
		L4XV_U(f);
		return;
	}

	t = l4_task_unmap(L4_BASE_TASK_CAP,
	                  l4_obj_fpage(l4x_cpu_ipi_irqs[cpu], 0, L4_FPAGE_RWX),
	                  L4_FP_ALL_SPACES);
	if (l4_error(t)) {
		LOG_printf("Failed to unmap IPI IRQ%d\n", cpu);
		L4XV_U(f);
		return;
	}
	L4XV_U(f);

	l4x_cap_free(l4x_cpu_ipi_irqs[cpu]);
	l4x_cpu_ipi_irqs[cpu] = L4_INVALID_CAP;

#ifndef CONFIG_L4_VCPU
	l4lx_thread_shutdown(l4x_cpu_ipi_threads[cpu], 0);
#endif
}

static L4_CV void __cpu_starter(void *x)
{
	l4_umword_t cpu;
	l4_msgtag_t tag;
	l4_utcb_t *u = l4_utcb();

	l4x_stack_setup(current_thread_info(), u, 0);

	tag = l4_ipc_receive(l4x_cpu_thread_get_cap(0), u, L4_IPC_NEVER);
	cpu = l4_utcb_mr_u(u)->mr[0];
	BUG_ON(l4_ipc_error(tag, u));

	l4x_create_ugate(l4x_stack_id_get(), cpu);

	l4lx_thread_pager_change(l4x_stack_id_get(), l4x_start_thread_id);

#ifdef CONFIG_L4_VCPU
	l4x_vcpu_init(l4x_vcpu_states[cpu]);
#endif

#ifdef ARCH_x86
	l4x_load_percpu_gdt_descriptor(get_cpu_gdt_table(cpu));
	l4x_stack_setup((struct thread_info *)(stack_start & ~(THREAD_SIZE - 1)),
	                u, cpu);
	asm volatile ("movl (stack_start), %esp; jmp *(initial_code)");
#endif
#ifdef ARCH_arm
	asm volatile ("b l4x_secondary_start_kernel");
#endif
	panic("CPU startup failed");
}

//static char smp_init_stack[L4LX_THREAD_STACK_SIZE];

void l4x_cpu_spawn(int cpu, struct task_struct *idle)
{
	char s[8];
	L4XV_V(f);

	BUG_ON(cpu >= NR_CPUS);

#ifndef CONFIG_L4_VCPU
	l4x_tamed_set_mapping(cpu, l4x_cpu_physmap_get_id(cpu));
	l4x_tamed_start(cpu);
#endif

	snprintf(s, sizeof(s), "cpu%d", cpu);
	s[sizeof(s)-1] = 0;

	l4x_cpu_idler[cpu] = idle;
	mb();

	L4XV_L(f);
	LOG_printf("Launching %s on pcpu %d at %p\n",
	          s, l4x_cpu_physmap_get_id(cpu), __cpu_starter);
	l4x_cpu_threads[cpu]
	   = l4lx_thread_create(__cpu_starter, cpu,
	                        NULL, NULL, 0, CONFIG_L4_PRIO_SERVER_PROC,
	                        &l4x_vcpu_states[cpu],
	                        s);
	L4XV_U(f);
}

void l4x_cpu_release(int cpu)
{
	int error;
	l4_msgtag_t tag;
	L4XV_V(f);

	L4XV_L(f);
	l4_utcb_mr()->mr[0] = cpu;
	tag = l4_ipc_send(l4x_cpu_thread_get_cap(cpu), l4_utcb(),
	                  l4_msgtag(0, 1, 0, 0), L4_IPC_NEVER);
	if ((error = l4_ipc_error(tag, l4_utcb())))
		LOG_printf("Failed to release CPU%d; ipc error %d\n",
		           cpu, error);
	L4XV_U(f);
}

void l4x_cpu_ipi_trigger(unsigned cpu)
{
	l4_msgtag_t tag;
	L4XV_V(f);
	L4XV_L(f);
	tag = l4_irq_trigger(l4x_cpu_ipi_irqs[cpu]);
	if (unlikely(l4_error(tag) != L4_PROTO_IRQ))
		LOG_printf("Trigger of IPI%d failed\n", cpu);
	L4XV_U(f);
}

void l4x_cpu_ipi_enqueue_vector(unsigned cpu, unsigned vector)
{
	l4_umword_t old, ret;
	do {
		old = l4x_cpu_ipi_vector_mask[cpu];
		ret = l4util_cmpxchg(&l4x_cpu_ipi_vector_mask[cpu],
		                     old, old | (1 << vector));
	} while (!ret);
}

#ifdef ARCH_x86
void l4x_send_IPI_mask_bitmask(unsigned long mask, int vector)
{
	int cpu;

	if (unlikely(vector > 31))
		LOG_printf("BIG vector %d (caller %lx)\n", vector, _RET_IP_);

	BUILD_BUG_ON(NR_CPUS > sizeof(l4x_cpu_ipi_vector_mask[cpu]) * 8);
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (!(mask & (1 << cpu)))
			continue;

		l4x_cpu_ipi_enqueue_vector(cpu, vector);
		l4x_cpu_ipi_trigger(cpu);
	}
}

void l4x_smp_process_IPI(int vector, struct pt_regs *regs)
{
	if (vector == RESCHEDULE_VECTOR) {
		extern void smp_reschedule_interrupt(struct pt_regs *regs);
		// actually this function is empty
		smp_reschedule_interrupt(regs);
		return;
	}

	if (vector == CALL_FUNCTION_VECTOR) {
		extern void smp_call_function_interrupt(struct pt_regs *);
		smp_call_function_interrupt(regs);
		return;
	}

	if (vector == CALL_FUNCTION_SINGLE_VECTOR) {
		extern void smp_call_function_single_interrupt(struct pt_regs *);
		smp_call_function_single_interrupt(regs);
		return;
	}

	if (vector == L4X_TIMER_VECTOR) {
		extern void l4x_smp_timer_interrupt(struct pt_regs *regs);
		l4x_smp_timer_interrupt(regs);
		return;
	}

	LOG_printf("Spurious IPI on CPU%d: %x\n", smp_processor_id(), vector);
	return;
}
#endif

void __cpuinit check_tsc_sync_source(int cpu)
{
	LOG_printf("%s(%d)\n", __func__, cpu);
}

void l4x_migrate_thread(l4_cap_idx_t thread, unsigned from_cpu, unsigned to_cpu)
{
	l4_msgtag_t t;
	l4_sched_param_t sp = l4_sched_param(CONFIG_L4_PRIO_SERVER_PROC, 0);
	sp.affinity = l4_sched_cpu_set(l4x_cpu_physmap_get_id(to_cpu), 0, 1);

	if (l4x_cpu_physmap_get_id(to_cpu) == l4x_cpu_physmap_get_id(from_cpu))
		return;

	t = l4_scheduler_run_thread(l4re_env()->scheduler, thread, &sp);
	if (l4_error(t))
		printk("Failed to migrate thread %lx\n", thread);
}

//----------------------
// repnop start
#ifndef CONFIG_L4_VCPU
l4lx_thread_t l4x_repnop_id;

static char l4x_repnop_stack[L4LX_THREAD_STACK_SIZE];

static L4_CV void l4x_repnop_thread(void *d)
{
	l4_cap_idx_t label;
	l4_msgtag_t tag;
	int error;
	l4_utcb_t *u = l4_utcb();

	tag = l4_ipc_wait(u, &label, L4_IPC_SEND_TIMEOUT_0);
	while (1) {
		if ((error = l4_ipc_error(tag, u)))
			LOG_printf("%s: IPC error = %x\n", __func__, error);

		tag = l4_ipc_reply_and_wait(u,
		                            l4_msgtag(0, 0, 0, L4_MSGTAG_SCHEDULE),
		                            &label, L4_IPC_SEND_TIMEOUT_0);
	}
}

void l4x_rep_nop(void)
{
	l4_msgtag_t t;
	LOG_printf("%s %d\n", __func__, __LINE__); 
	t = l4_ipc_call(l4lx_thread_get_cap(l4x_repnop_id), l4_utcb(),
	                l4_msgtag(0, 0, 0, L4_MSGTAG_SCHEDULE), L4_IPC_NEVER);
	BUG_ON(l4_ipc_error(t, l4_utcb()));
}

static void l4x_repnop_init(void)
{
	l4x_repnop_id = l4lx_thread_create(l4x_repnop_thread, 0,
	                                   l4x_repnop_stack
	                                     + sizeof(l4x_repnop_stack),
	                                   NULL, 0,
	                                   CONFIG_L4_PRIO_SERVER_PROC - 1,
	                                   0, "nop");
}
// repnop end
// ---------------
#endif

#else
#ifndef CONFIG_L4_VCPU
static inline void l4x_repnop_init(void) {}
#endif
#endif

static int l4x_cpu_virt_phys_map_init(const char *boot_command_line)
{
	l4_umword_t max_cpus = 1;
	l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0, 0);
	unsigned i;

#ifdef CONFIG_SMP
	char overbooking = 0;
	char *p;

	if ((p = strstr(boot_command_line, "l4x_cpus="))) {
		l4x_nr_cpus = simple_strtoul(p + 9, NULL, 0);
		if (l4x_nr_cpus > NR_CPUS) {
			LOG_printf("Linux only configured for max. %d CPUs. Limited to %d.\n",
			           NR_CPUS, NR_CPUS);
			l4x_nr_cpus = NR_CPUS;
		}
	}


	if (l4_error(l4_scheduler_info(l4re_env()->scheduler,
	                               &max_cpus, &cs)) == L4_EOK) {
		if ((p = strstr(boot_command_line, "l4x_cpus_map="))) {
			// l4x_cpus_map=0,1,2,3,4,...
			// the list specifies the physical CPU for each
			// logical, the number of logical CPUs is limited to
			// the size of the given list
			unsigned pcpu;
			char *q;
			p += 13;
			l4x_nr_cpus = 0;
			while (*p && *p != ' ') {

				if (l4x_nr_cpus >= NR_CPUS) {
					LOG_printf("ERROR: vCPU%d out of bounds\n", pcpu);
					return 1;
				}

				pcpu = simple_strtoul(p, &q, 0);
				if (p == q) {
					LOG_printf("ERROR: Error parsing l4x_cpus_map option\n");
					return 1;
				}
				if (!l4x_cpu_check_pcpu(pcpu, max_cpus)) {
					LOG_printf("ERROR: pCPU%d not found\n", pcpu);
					return 1;
				}
				l4x_cpu_physmap[l4x_nr_cpus].phys_id = pcpu;
				for (i = 0; i < l4x_nr_cpus; ++i)
					overbooking |=
					   l4x_cpu_physmap[i].phys_id == pcpu;
				l4x_nr_cpus++;
				if (*q != ',')
					break;
				p = q + 1;
			}
		} else {
			unsigned p = 0;
			unsigned v;
			for (v = 0; v < NR_CPUS && v < l4x_nr_cpus; ++v) {
				while (p < max_cpus
				       && !l4x_cpu_check_pcpu(p, max_cpus))
					p++;
				if (p == max_cpus)
					break;
				l4x_cpu_physmap[v].phys_id = p++;
			}
			l4x_nr_cpus = v;

		}
	}
#ifndef CONFIG_L4_VCPU
	l4x_tamed_set_mapping(0, l4x_cpu_physmap_get_id(0));
#endif

#else /* UP */
	if (l4_error(l4_scheduler_info(l4re_env()->scheduler,
	                               &max_cpus, &cs)) == L4_EOK) {

		int p = 0;
		if (cs.map)
			p = find_first_bit(&cs.map, sizeof(cs.map) * 8);
		l4x_cpu_physmap[0].phys_id = p;
	}
#endif

	LOG_printf("CPU mapping (l:p)[%d]: ", l4x_nr_cpus);
	for (i = 0; i < l4x_nr_cpus; i++)
		LOG_printf("%u:%u%s", i, l4x_cpu_physmap[i].phys_id,
		           i == l4x_nr_cpus - 1 ? "\n" : ", ");

#if defined(CONFIG_SMP) && defined(ARCH_arm) && defined(CONFIG_L4_VCPU)
	if (overbooking) {
		LOG_printf("  On ARM it is not possible to place multiple\n"
		           "  logical CPUs on a single physical CPU as the TLS\n"
			   "  register handling cannot be virtualized.\n");
		// On a longer thought we'd need to find out on which vCPU
		// we're running. The utcb address should be available via
		// tls-reg-3 and then we'd need to find out the current tls
		// value for that vCPU, maybe by putting a page at the same
		// place as the utcb is in the linux server address space.
		// Or: We just do a system call.
		// But: Userspace also likes to do NOT use the kernel
		// provided tls code (e.g. Android) so this would not work
		// at all.
		enter_kdebug("Please read the message");
	}
#endif

	return 0;
}



#ifdef CONFIG_PCI
#ifdef CONFIG_X86_IO_APIC
void __init check_acpi_pci(void)
{}
#endif
#endif

/*
 * This is the panic blinking function, we misuse it to sleep forever.
 */
static long l4x_blink(int state)
{
	L4XV_V(f);
	printk("panic: going to sleep forever, bye\n");
	L4XV_L(f);
	LOG_printf("panic: going to sleep forever, bye\n");
	l4_sleep_forever();
	return 0;
}

static L4_CV void __init l4x_linux_startup(void *data)
{
	l4_cap_idx_t caller_id = *(l4_cap_idx_t *)data;
	struct thread_info *ti = current_thread_info();

	l4x_stack_setup(ti, l4_utcb(), 0);

#ifdef CONFIG_L4_VCPU
	l4x_vcpu_init(l4x_vcpu_states[0]);
#endif

	LOG_printf("%s: thread "l4util_idfmt".\n",
	           __func__, l4util_idstr(l4x_stack_id_get()));

	/* Wait for start signal */
	l4_ipc_receive(caller_id, l4_utcb(), L4_IPC_NEVER);

	LOG_printf("main thread: received startup message.\n");

	l4x_create_ugate(l4x_stack_id_get(), 0);
	l4lx_thread_pager_change(l4x_stack_id_get(), caller_id);

#ifdef ARCH_x86
	asm volatile("movl %%ds,  %%eax \n"
	             "movl %%eax, %%fs  \n"  // fs == ds for percpu
		     : : : "eax", "memory");
#endif
	*ti = (struct thread_info) INIT_THREAD_INFO(init_task);
	ti->task->stack = ti;

#ifdef CONFIG_L4_VCPU
#ifdef CONFIG_X86_32
	ti->task->thread.regsp = task_pt_regs_v(ti->task);
#else
	ti->task->thread.regsp = task_pt_regs(ti->task);
#endif
#endif

	panic_blink = l4x_blink;

#ifdef ARCH_x86
	// todo: also do more ..._early_setup stuff here
	legacy_pic = &null_legacy_pic;
#endif

	local_irq_disable();
#ifdef CONFIG_X86_32
	i386_start_kernel();
#elif defined(CONFIG_X86_64)
	l4x_stack_setup((struct thread_info *)per_cpu(irq_stack_union.irq_stack, 0), l4_utcb(), 0);
	x86_64_start_kernel(NULL);
#else
	start_kernel();
#endif
}

#ifdef CONFIG_BLK_DEV_INITRD
static l4re_ds_t l4x_initrd_ds;
static unsigned long l4x_initrd_mem_start;

/**
 * Get the RAM disk from the file provider!
 *
 * \param filename    File name in the form {(nd)}/path/to/file
 * \param *rd_start   Start of ramdisk
 * \param *rd_end     End of ramdisk
 *
 * \return 0 on succes, != 0 otherwise
 */
static int fprov_load_initrd(const char *filename,
                             unsigned long *rd_start,
                             unsigned long *rd_end)
{
	int ret;
	l4re_ds_stats_t dsstat;

	if ((ret = l4x_query_and_get_ds(filename, "initrd", &l4x_initrd_ds,
	                                (void **)rd_start, &dsstat))) {
		LOG_printf("Failed to get initrd: %s(%d)\n",
		           l4sys_errtostr(ret), ret);
		*rd_start = *rd_end = 0;
		return -1;
	}

	LOG_printf("INITRD: Size of RAMdisk is %ldKiB\n", dsstat.size >> 10);

	l4x_initrd_mem_start = *rd_start;
	*rd_end = *rd_start + dsstat.size;
	printk("INITRD: %08lx - %08lx\n", *rd_start, *rd_end);

	if (dsstat.size * 2 > l4x_mainmem_size) {
		LOG_printf("WARNING: RAMdisk size of %ldMB probably too big\n"
		           "for %ldMB main memory. Sleeping a bit...\n",
		           dsstat.size >> 20, l4x_mainmem_size >> 20);
		l4_sleep(20000);
	}

	LOG_flush();
	return 0;
}

/**
 * Free InitRD memory.
 */
void l4x_free_initrd_mem(void)
{
	L4XV_V(f);
	printk("INITRD: Freeing memory.\n");
	/* detach memory */
	L4XV_L(f);
	if (l4re_rm_detach((void *)l4x_initrd_mem_start))
		enter_kdebug("Error detaching from initrd mem!");
	L4XV_U(f);
}


#define L4X_MAX_RD_PATH 200
static char l4x_rd_path[L4X_MAX_RD_PATH];

void l4x_load_initrd(char *command_line)
{
	char *sa, *se;
	char param_str[] = "l4x_rd=";
	int i, b;

	sa = command_line;
	while (*sa) {
		for (i = 0, b = 1; param_str[i] && b; i++)
			b = sa[i] == param_str[i];
		if (b)
			break;
		sa++;
	}
	if (*sa) {
		sa += strlen(param_str);
		se = sa;

		while (*se && *se != ' ')
			se++;
		if (se - sa > L4X_MAX_RD_PATH) {
			enter_kdebug("l4x_rd_path > L4X_MAX_RD_PATH");
			return;
		}
		strncpy(l4x_rd_path, sa, se - sa);
		LOG_printf("l4x_rd_path: %s\n", l4x_rd_path);
	}

	if (*l4x_rd_path) {
		LOG_printf("Loading: %s\n", l4x_rd_path);

		if (fprov_load_initrd(l4x_rd_path,
		                      &initrd_start,
		                      &initrd_end)) {
			LOG_flush();
			LOG_printf("Couldn't load ramdisk :(\n");
			return;
		}

		initrd_below_start_ok = 1;

		//reserve_bootmem(initrd_start, initrd_end - initrd_start);

		LOG_printf("RAMdisk from %08lx to %08lx [%ldKiB]\n",
		           initrd_start, initrd_end,
		           (initrd_end - initrd_start) >> 10);
	}
}
#endif /* CONFIG_BLK_DEV_INITRD */

static void get_initial_cpu_capabilities(void)
{
#ifdef CONFIG_X86_32
	/* new_cpu_data is filled into boot_cpu_data in setup_arch */
	extern struct cpuinfo_x86 new_cpu_data;
	new_cpu_data.x86_capability[0] = l4util_cpu_capabilities();
#endif
}

#ifdef CONFIG_X86

#include <asm/l4x/ioport.h>

static u32 l4x_x86_kernel_ioports[65536 / sizeof(u32)];

int l4x_x86_handle_user_port_request(struct task_struct *task,
                                     unsigned nr, unsigned len)
{
	l4_msgtag_t tag;
	unsigned char o;
	unsigned i;

	if (nr + len > 65536)
		return 0;

	for (i = 0; i < len; ++i)
		if (!test_bit(nr + i,
		              (volatile unsigned long *)l4x_x86_kernel_ioports))
			return 0;

	i = nr;
	do {
		L4XV_V(f);
		o = l4_fpage_max_order(0, i, nr, nr + len, 0);
		L4XV_L(f);
		tag = l4_task_map(task->mm->context.task,
		                  L4RE_THIS_TASK_CAP,
		                  l4_iofpage(nr, o),
		                  l4_map_control(nr, 0, L4_MAP_ITEM_MAP));
		L4XV_U(f);
		if (l4_error(tag))
			return 0;

		i += 1 << o;
	} while (i < nr + len);

	return 1;
}

static void l4x_x86_register_ports(l4io_resource_t *res)
{
	unsigned i;

	if (res->type != L4IO_RESOURCE_PORT)
		return;

	if (res->start > res->end)
		return;

	for (i = res->start; i <= res->end; ++i)
		if (i < 65536)
			set_bit(i,
			        (volatile unsigned long *)l4x_x86_kernel_ioports);

}
#endif

static void l4x_scan_hw_resources(void)
{
	l4io_device_handle_t dh = l4io_get_root_device();
	l4io_device_t dev;
	l4io_resource_handle_t reshandle;

	LOG_printf("Device scan:\n");
	while (1) {
		l4io_resource_t res;

		if (l4io_iterate_devices(&dh, &dev, &reshandle))
			break;

		if (dev.num_resources == 0)
			continue;

		LOG_printf("  Device: %s\n", dev.name);

		while (!l4io_lookup_resource(dh, L4IO_RESOURCE_ANY,
					     &reshandle, &res)) {
			char *t = "undef";

			switch (res.type) {
				case L4IO_RESOURCE_IRQ:  t = "IRQ";  break;
				case L4IO_RESOURCE_MEM:  t = "MEM";  break;
				case L4IO_RESOURCE_PORT: t = "PORT"; break;
			};

			LOG_printf("    %s: %08lx - %08lx\n",
			           t, res.start, res.end);

#ifdef CONFIG_X86
			l4x_x86_register_ports(&res);
			l4io_request_ioport(res.start,
			                    res.end - res.start + 1);
#endif
		}
	}
}

int __init_refok L4_CV main(int argc, char **argv)
{
	l4lx_thread_t main_id;
	extern char _end[];
	extern char boot_command_line[];
	unsigned i;
	char *p;

#ifdef CONFIG_L4_SERVER
	l4x_srv_init();
#endif

	LOG_printf("\033[34;1m======> L4Linux 2.6 starting... <========\033[0m\n");
	LOG_printf("%s", linux_banner);
	LOG_printf("Binary name: %s\n", (*argv)?*argv:"NULL??");
#ifdef ARCH_arm
#ifdef CONFIG_AEABI
	LOG_printf("   This is an AEABI build.\n");
#else
	LOG_printf("   This is an OABI build.\n");
#endif
#endif

	argv++;
	p = boot_command_line;
	while (*argv) {
		i = strlen(*argv);
		if (p - boot_command_line + i >= COMMAND_LINE_SIZE) {
			LOG_printf("Command line too long!");
			enter_kdebug("Command line too long!");
		}
		strcpy(p, *argv);
		p += i;
		if (*++argv)
			*p++ = ' ';
	}
	LOG_printf("Linux kernel command line (%d args): %s\n",
	           argc - 1, boot_command_line);

	/* See if we find a showpfexc=1 or showghost=1 in the command line */
	if ((p = strstr(boot_command_line, "showpfexc=")))
		l4x_debug_show_exceptions = simple_strtoul(p+10, NULL, 0);
	if ((p = strstr(boot_command_line, "showghost=")))
		l4x_debug_show_ghost_regions = simple_strtoul(p+10, NULL, 0);

	if (l4x_check_setup(boot_command_line))
		return 1;

	if (l4x_cpu_virt_phys_map_init(boot_command_line))
		return 1;

	l4x_x86_utcb_save_orig_segment();

	LOG_printf("Image: %08lx - %08lx [%lu KiB].\n",
	           (unsigned long)_stext, (unsigned long)_end,
	           (unsigned long)(_end - _stext + 1) >> 10);

	LOG_printf("Areas: Text:     %08lx - %08lx [%ldkB] (a bit longer)\n",
	           (unsigned long)&_stext, (unsigned long)&_sdata,
	           ((unsigned long)&_sdata - (unsigned long)&_stext) >> 10);
	LOG_printf("       Data:     %08lx - %08lx [%ldkB]\n",
	           (unsigned long)&_sdata, (unsigned long)&_edata,
	           ((unsigned long)&_edata - (unsigned long)&_sdata) >> 10);
	LOG_printf("       Initdata: %08lx - %08lx [%ldkB]\n",
	           (unsigned long)&__init_begin, (unsigned long)&__init_end,
	           ((unsigned long)&__init_end - (unsigned long)&__init_begin) >> 10);
	LOG_printf("       BSS:      %08lx - %08lx [%ldkB]\n",
	           (unsigned long)&__bss_start, (unsigned long)&__bss_stop,
	           ((unsigned long)&__bss_stop - (unsigned long)&__bss_start) >> 10);

	/* Touch areas to avoid pagefaults
	 * Data needs to be touched rw, code ro
	 */
	l4_touch_ro(&_stext, (unsigned long)&_sdata - (unsigned long)&_stext);
	l4_touch_rw(&_sdata, (unsigned long)&_edata - (unsigned long)&_sdata);
	l4_touch_ro(&__init_begin,
	            (unsigned long)&__init_end - (unsigned long)&__init_begin);
	l4_touch_rw(&__bss_start,
	            (unsigned long)&__bss_stop - (unsigned long)&__bss_start);

	/* some things from head.S */
	get_initial_cpu_capabilities();

#ifdef ARCH_x86
#ifdef CONFIG_VGA_CONSOLE
	/* map VGA range */
	if (l4io_request_iomem_region(0xa0000, 0xa0000, 0xc0000 - 0xa0000, 0)) {
		LOG_printf("Failed to map VGA area. Ux?\n");
		return 0;
	}
	if (l4x_pagein(0xa0000, 0xc0000 - 0xa0000, 1))
		LOG_printf("Page-in of VGA failed.\n");
#endif
#endif

#ifdef ARCH_x86
	{
		unsigned long gs, fs;
		asm volatile("mov %%gs, %0" : "=r"(gs));
		asm volatile("mov %%fs, %0" : "=r"(fs));
		LOG_printf("gs=%lx   fs=%lx\n", gs, fs);
	}
#endif

	/* Init v2p list */
	l4x_v2p_init();

	l4lx_task_init();
	l4lx_thread_init();

	l4x_start_thread_id = l4re_env()->main_thread;

	l4_thread_control_start();
	l4_thread_control_commit(l4x_start_thread_id);
	l4x_start_thread_pager_id
		= l4_utcb_mr()->mr[L4_THREAD_CONTROL_MR_IDX_PAGER];

#ifndef CONFIG_L4_VCPU
#ifdef CONFIG_L4_TAMED
	l4x_tamed_init();
	l4x_tamed_start(0);
#endif
	l4x_repnop_init();
#endif

	l4x_scan_hw_resources();

#ifdef CONFIG_X86
	if (l4util_kip_kernel_has_feature(l4lx_kinfo, "io_prot")) {
#ifndef CONFIG_L4_TAMED
		enter_kdebug("ioprot-microkernel -> enable TAMED mode!");
#endif
	}

	/* Initialize GDT entry offset */
#ifdef CONFIG_X86_32
	l4x_fiasco_gdt_entry_offset = fiasco_gdt_get_entry_offset(l4re_env()->main_thread, l4_utcb());
	LOG_printf("l4x_fiasco_gdt_entry_offset = %x\n", l4x_fiasco_gdt_entry_offset);
#endif

	l4x_v2p_add_item(0xa0000, (void *)0xa0000, 0xfffff - 0xa0000);

#ifdef CONFIG_L4_FERRET
	l4x_ferret_init();
#endif

#ifdef CONFIG_L4_EXTERNAL_RTC
	/* Make sure RTC is ready */
	{
		l4_uint32_t seconds;
		if (l4rtc_get_seconds_since_1970(&seconds))
			LOG_printf("WARNING: RTC server does not seem there!\n");
	}
#endif
#endif /* CONFIG_X86 */

	l4x_setup_upage();

#ifdef ARCH_arm
	setup_module_area();

	if (l4x_sanity_check_kuser_cmpxchg())
		return 1;
#endif

	/* Set name of startup thread */
	l4lx_thread_name_set(l4x_start_thread_id, "l4x-start");

	/* fire up Linux server, will wait until start message */
	p = (char *)init_stack + sizeof(init_stack);
#ifdef CONFIG_X86_64
	p = (char *)((struct pt_regs *)p - 1);
#endif
	main_id = l4lx_thread_create(l4x_linux_startup, 0, p,
	                             &l4x_start_thread_id,
	                             sizeof(l4x_start_thread_id),
	                             CONFIG_L4_PRIO_SERVER_PROC,
	                             &l4x_vcpu_states[0],
	                             "cpu0");

	if (!l4lx_thread_is_valid(main_id))
		return 1;


	l4x_cpu_thread_set(0, main_id);

	LOG_printf("main thread will be " PRINTF_L4TASK_FORM "\n",
	           PRINTF_L4TASK_ARG(l4lx_thread_get_cap(main_id)));

	l4x_register_pointer_section(&__init_begin, 0, "section-with-init");

#ifdef ARCH_x86
	/* Needed for smp alternatives -- nobody will ever use this for a
	 * real phys-addr (hopefully), we cannot use real phys addresses as
	 * we would need to use pinned memory for the text-segment...
	 * Give one page more for text_poke */
	l4x_v2p_add_item((unsigned long)&_text, &_text,
	                 (unsigned long)&_etext - (unsigned long)&_text + PAGE_SIZE);
#endif
	/* The next is not necessary as no-one has done virt_to_phys
	 * for these areas so far */
	//l4x_register_pointer_section(&_edata, 0, "data");

	/* Send start message to main thread. */
	l4_ipc_send(l4lx_thread_get_cap(main_id),
	            l4_utcb(), l4_msgtag(0, 0, 0, 0), L4_IPC_NEVER);

	LOG_printf("Main thread running, waiting...\n");

	l4x_server_loop();

	return 0;
}

#ifdef CONFIG_X86

#if 0
/*
 * System calls that need a pt_regs pointer.
 */
#define PTREGSCALL(name)					\
long ptregs_##name(void);					\
long sys_##name(struct pt_regs *regs);				\
long ptregs_##name(void)					\
{								\
	return sys_##name(L4X_THREAD_REGSP(&current->thread));	\
}

PTREGSCALL(iopl)
PTREGSCALL(fork)
PTREGSCALL(vfork)
PTREGSCALL(execve)
PTREGSCALL(sigaltstack)
PTREGSCALL(sigreturn)
PTREGSCALL(rt_sigreturn)
PTREGSCALL(vm86)
PTREGSCALL(vm86old)
#endif

#include <asm/syscalls.h>

#define PTREGSCALL0(name)                                       \
long ptregs_##name(void)                                        \
{                                                               \
	struct pt_regs *r = L4X_THREAD_REGSP(&current->thread); \
	return sys_##name(r);                                   \
}

#define PTREGSCALL1(name, t1)                                   \
long ptregs_##name(void)                                        \
{                                                               \
	struct pt_regs *r = L4X_THREAD_REGSP(&current->thread); \
	return sys_##name((t1)r->bx, r);                        \
}

#define PTREGSCALL2(name, t1, t2)                               \
long ptregs_##name(void)                                        \
{                                                               \
	struct pt_regs *r = L4X_THREAD_REGSP(&current->thread); \
	return sys_##name((t1)r->bx, (t2)r->cx, r);             \
}

#define PTREGSCALL3(name, t1, t2, t3)                           \
long ptregs_##name(void)                                        \
{                                                               \
	struct pt_regs *r = L4X_THREAD_REGSP(&current->thread); \
	return sys_##name((t1)r->bx, (t2)r->cx, (t3)r->dx, r);  \
}


#ifdef CONFIG_X86_32
PTREGSCALL1(iopl, unsigned int)
PTREGSCALL0(fork)
PTREGSCALL0(vfork)
PTREGSCALL3(execve, const char __user *, const char __user *const __user *, const char __user *const __user *)
PTREGSCALL2(sigaltstack, const stack_t __user *, stack_t __user *)
PTREGSCALL0(sigreturn)
PTREGSCALL0(rt_sigreturn)
PTREGSCALL2(vm86, unsigned long, unsigned long)
PTREGSCALL1(vm86old, struct vm86_struct __user *)

long ptregs_clone(void)
{
	struct pt_regs *r = L4X_THREAD_REGSP(&current->thread);
	return sys_clone(r->bx, r->cx, (void __user *)r->dx,
	                 (void __user *)r->di, r);
}
#endif

#ifdef CONFIG_X86_32
#define RN(n) e##n
#else
#define RN(n) r##n
#endif

static void l4x_setup_die_utcb(l4_exc_regs_t *exc)
{
	struct pt_regs regs;
	unsigned long regs_addr;
	extern void die(const char *msg, struct pt_regs *regs, int err);
	static char message[40];

	snprintf(message, sizeof(message), "Trap: %ld", exc->trapno);
	message[sizeof(message) - 1] = 0;

	memset(&regs, 0, sizeof(regs));
	utcb_exc_to_ptregs(exc, &regs);

	/* XXX: check stack boundaries, i.e. exc->esp & (THREAD_SIZE-1)
	 * >= THREAD_SIZE - sizeof(thread_struct) - sizeof(struct pt_regs)
	 * - ...)
	 */
	/* Copy pt_regs on the stack */
	exc->sp -= sizeof(struct pt_regs);
	*(struct pt_regs *)exc->sp = regs;
	regs_addr = exc->sp;

	/* Fill arguments in regs for die params */
#ifdef CONFIG_X86_32
	exc->ecx = exc->err;
	exc->edx = regs_addr;
	exc->eax = (unsigned long)message;
#else
	exc->rdx = exc->err;
	exc->rsi = regs_addr;
	exc->rdi = (unsigned long)message;
#endif

	exc->sp -= sizeof(unsigned long);
	*(unsigned long *)exc->sp = 0;
	/* Set PC to die function */
	exc->ip = (unsigned long)die;

	outstring("Die message: ");
	outstring(message);
	outstring("\n");
}

asmlinkage static void l4x_do_intra_iret(struct pt_regs regs)
{
#ifdef CONFIG_X86_32
	asm volatile ("mov %%cs, %0" : "=r" (regs.cs));
	asm volatile
	("movl %0, %%esp	\t\n"
	 "popl %%ebx		\t\n"
	 "popl %%ecx		\t\n"
	 "popl %%edx		\t\n"
	 "popl %%esi		\t\n"
	 "popl %%edi		\t\n"
	 "popl %%ebp		\t\n"
	 "popl %%eax		\t\n"
	 "addl $8, %%esp	\t\n" /* keep ds, es */
	 "popl %%fs		\t\n"
	 "popl %%gs		\t\n"
	 "addl $4, %%esp	\t\n" /* keep orig_eax */
	 "iret			\t\n"
	 : : "r" (&regs));
#endif

	panic("Intra game zombie walking!");
}

static void l4x_setup_stack_for_traps(l4_exc_regs_t *exc, struct pt_regs *regs,
                                      void (*trap_func)(struct pt_regs *regs, long err))
{
	const int l4x_intra_regs_size
		= sizeof(struct pt_regs) - 2 * sizeof(unsigned long);

	/* Copy pt_regs on the stack but omit last to two dwords,
	 * an intra-priv exception/iret doesn't have those, and native
	 * seems to do it the same, with the hope that nobody touches
	 * there in the pt_regs */
	exc->sp -= l4x_intra_regs_size;
	memcpy((void *)exc->sp, regs, l4x_intra_regs_size);

	/* do_<exception> functions are regparm(3), arguments go in regs */
#ifdef CONFIG_X86_32
	exc->eax = exc->sp;
	exc->edx = exc->err;
#else
	exc->rdi = exc->sp;
	exc->rsi = exc->err;
#endif

	/* clear TF */
	exc->flags &= ~256;

	exc->sp -= sizeof(unsigned long);
	*(unsigned long *)exc->sp = 0; /* Return of l4x_do_intra_iret */
	exc->sp -= sizeof(unsigned long);
	*(unsigned long *)exc->sp = (unsigned long)l4x_do_intra_iret;

	/* Set PC to trap function */
	exc->ip = (unsigned long)trap_func;
}

static int l4x_handle_hlt_for_bugs_test(l4_exc_regs_t *exc)
{
	void *pc = (void *)l4_utcb_exc_pc(exc);

	if (*(unsigned int *)pc == 0xf4f4f4f4) {
		// check_bugs does 4 times hlt
		LOG_printf("Jumping over 4x 'hlt' at 0x%lx\n",
		           (unsigned long)pc);
		exc->ip += 4;
		return 0; // handled
	}

	return 1; // not for us
}

#ifdef CONFIG_KPROBES
static int l4x_handle_kprobes(l4_exc_regs_t *exc)
{
	extern void do_int3(struct pt_regs *regs, long err);
	struct pt_regs regs;

	// XXX need to check other thread!!
	//if (kprobe_running())
	//	return 1; /* Not handled */

	if (l4_utcb_exc_pc(exc) < PAGE_SIZE)
		return 1; /* Can not handle */

	/* check for kprobes break instruction */
	if (*(unsigned char *)l4_utcb_exc_pc(exc) != BREAKPOINT_INSTRUCTION)
		return 1; /* Not handled */

	utcb_exc_to_ptregs(exc, &regs);
	l4x_set_kernel_mode(&regs);

	/* Set after breakpoint instruction as for HLT pc is on the
	 * instruction and for INT3 after the instruction */
	regs.ip++;

	l4x_setup_stack_for_traps(exc, &regs, do_int3);
	return 0;
}
#endif

static int l4x_handle_int1(l4_exc_regs_t *exc)
{
	struct pt_regs regs;
	extern void do_debug(struct pt_regs *regs, long err);

	utcb_exc_to_ptregs(exc, &regs);
	l4x_setup_stack_for_traps(exc, &regs, do_debug);
	return 0;
}

static int l4x_handle_clisti(l4_exc_regs_t *exc)
{
	unsigned char opcode = *(unsigned char *)l4_utcb_exc_pc(exc);
	extern void exit(int);

	/* check for cli or sti instruction */
	if (opcode != 0xfa && opcode != 0xfb)
		return 1; /* not handled if not those instructions */

	/* If we trap those instructions it's most likely a configuration
	 * error and quite early in the boot-up phase, so just quit. */
	LOG_printf("Aborting L4Linux due to unexpected CLI/STI instructions"
	           " at %lx.\n", l4_utcb_exc_pc(exc));
	enter_kdebug("abort");
	exit(0);

	return 0;
}

static unsigned long l4x_handle_ioport_pending_pc;

static int l4x_handle_ioport(l4_exc_regs_t *exc)
{
	if (l4_utcb_exc_pc(exc) == l4x_handle_ioport_pending_pc) {
		l4x_setup_die_utcb(exc);
		return 0;
	}
	return 1;
}

void in_kernel_int80_set_kernel(void)
{ set_fs(KERNEL_DS); }

void in_kernel_int80_set_user(void)
{ set_fs(USER_DS); }

asm(
"in_kernel_int80_helper: \n\t"
#ifdef CONFIG_X86_32
"	pushl %eax	\n\t"	/* store eax */
"	call in_kernel_int80_set_kernel \n\t"
"	popl  %eax	\n\t"	/* restore eax */
"	call *%eax	\n\t"	/* eax has the sys_foo function we are calling */
"	movl %eax,24(%esp) \n\t"	/* store return value from sys_foo */
#else
"	push %rax	\n\t"	/* store eax */
"	call in_kernel_int80_set_kernel \n\t"
"	pop  %rax	\n\t"	/* restore eax */
"	call *%rax	\n\t"	/* eax has the sys_foo function we are calling */
"	mov %rax,24(%rsp) \n\t"	/* store return value from sys_foo */
#endif
"	call in_kernel_int80_set_user\n\t"
"	call l4x_do_intra_iret\n\t"		/* return */
);

static int l4x_handle_lxsyscall(l4_exc_regs_t *exc)
{
	void *pc = (void *)l4_utcb_exc_pc(exc);
	extern char in_kernel_int80_helper[];
	unsigned long syscall;
	struct pt_regs *regsp;
	struct thread_info *ti;
	const int l4x_intra_regs_size
		= sizeof(struct pt_regs) - 2 * sizeof(unsigned long);

#ifdef CONFIG_X86_64
	LOG_printf("possibly FIRST SYSCALL: %lx, %p...\n", exc->err, pc);
#endif

	if (pc < (void *)_stext || pc > (void *)_etext)
		return 1; /* Not for us */

	if (exc->err != 0x402)
		return 1; /* No int80 error code */

	if (*(unsigned short *)pc != 0x80cd)
		return 1; /* No int80 instructions */

	syscall = exc->RN(ax);

	if (!is_lx_syscall(syscall))
		return 1; /* Not a valid system call number */

	ti = (struct thread_info *)(exc->sp & ~(THREAD_SIZE - 1));
	regsp = L4X_THREAD_REGSP(&ti->task->thread);

	utcb_exc_to_ptregs(exc, regsp);
	l4x_set_kernel_mode(regsp);

	/* Set pc after int80 */
	regsp->ip += 2;

	exc->sp -= l4x_intra_regs_size;
	memcpy((void *)exc->sp, regsp, l4x_intra_regs_size);

#ifdef CONFIG_L4_VCPU
	if (syscall == __NR_execve)
		LOG_printf("internal __NR_execve called\n");
#else
	/* eax has the function, see in_kernel_int80_helper */
	if (syscall == __NR_execve)
		exc->RN(ax) = (unsigned long)l4_kernelinternal_execve;
	else
#endif
		exc->RN(ax) = (unsigned long)sys_call_table[syscall];

	/* Set PC to helper */
	exc->ip = (unsigned long)in_kernel_int80_helper;

	return 0;
}

static int l4x_handle_msr(l4_exc_regs_t *exc)
{
	void *pc = (void *)l4_utcb_exc_pc(exc);
	unsigned long reg = exc->RN(cx);

	/* wrmsr */
	if (*(unsigned short *)pc == 0x300f) {
		if (reg != MSR_IA32_SYSENTER_CS
		    && reg != MSR_IA32_SYSENTER_ESP
		    && reg != MSR_IA32_SYSENTER_EIP)
			LOG_printf("WARNING: Unknown wrmsr: %08lx at %p\n", reg, pc);

		exc->ip += 2;
		return 0; // handled
	}

	/* rdmsr */
	if (*(unsigned short *)pc == 0x320f) {

		if (reg == MSR_IA32_MISC_ENABLE) {
			exc->RN(ax) = exc->RN(dx) = 0;
		} else if (reg == MSR_K7_CLK_CTL) {
			exc->RN(ax) = 0x20000000;
		} else if (reg == MSR_K8_TSEG_ADDR) {
			exc->RN(ax) = exc->RN(dx) = 0;
		} else
			LOG_printf("WARNING: Unknown rdmsr: %08lx at %p\n", reg, pc);

		exc->ip += 2;
		return 0; // handled
	}

	return 1; // not for us
}

static inline void l4x_print_exception(l4_cap_idx_t t, l4_exc_regs_t *exc)
{
	LOG_printf("EX: "l4util_idfmt": "
	           "pc = "l4_addr_fmt" "
	           "sp = "l4_addr_fmt" "
	           "trapno = 0x%lx err/pfa = 0x%lx%s\n",
	           l4util_idstr(t), exc->ip, exc->sp,
	           exc->trapno,
	           exc->trapno == 14 ? exc->pfa : exc->err,
	           exc->trapno == 14 ? (exc->err & 2) ? " w" : " r" : "");

	if (l4x_debug_show_exceptions >= 2
	    && !l4_utcb_exc_is_pf(exc)) {
		/* Lets assume we can do the following... */
		unsigned len = 72, i;
		unsigned long ip = exc->ip - 43;

		LOG_printf("Dump: ");
		for (i = 0; i < len; i++, ip++)
			if (ip == exc->ip)
				LOG_printf("<%02x> ", *(unsigned char *)ip);
			else
				LOG_printf("%02x ", *(unsigned char *)ip);

		LOG_printf(".\n");
	}
}

#ifdef CONFIG_L4_VCPU
static inline l4_exc_regs_t *cast_to_utcb_exc(l4_vcpu_regs_t *vcpu_regs)
{
#ifdef CONFIG_X86_32
	enum { OFF = offsetof(l4_vcpu_regs_t, gs), };
#else
	enum { OFF = offsetof(l4_vcpu_regs_t, r15), };
#endif
#ifdef CONFIG_X86_32
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, gs)     - OFF != offsetof(l4_exc_regs_t, gs));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, fs)     - OFF != offsetof(l4_exc_regs_t, fs));
#else
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r15)    - OFF != offsetof(l4_exc_regs_t, r15));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r14)    - OFF != offsetof(l4_exc_regs_t, r14));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r13)    - OFF != offsetof(l4_exc_regs_t, r13));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r12)    - OFF != offsetof(l4_exc_regs_t, r12));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r11)    - OFF != offsetof(l4_exc_regs_t, r11));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r10)    - OFF != offsetof(l4_exc_regs_t, r10));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r9)     - OFF != offsetof(l4_exc_regs_t, r9));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r8)     - OFF != offsetof(l4_exc_regs_t, r8));
#endif
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, di)     - OFF != offsetof(l4_exc_regs_t, RN(di)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, si)     - OFF != offsetof(l4_exc_regs_t, RN(si)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, bp)     - OFF != offsetof(l4_exc_regs_t, RN(bp)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, pfa)    - OFF != offsetof(l4_exc_regs_t, pfa));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, bx)     - OFF != offsetof(l4_exc_regs_t, RN(bx)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, dx)     - OFF != offsetof(l4_exc_regs_t, RN(dx)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, cx)     - OFF != offsetof(l4_exc_regs_t, RN(cx)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, ax)     - OFF != offsetof(l4_exc_regs_t, RN(ax)));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, trapno) - OFF != offsetof(l4_exc_regs_t, trapno));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, err)    - OFF != offsetof(l4_exc_regs_t, err));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, ip)     - OFF != offsetof(l4_exc_regs_t, ip));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, flags)  - OFF != offsetof(l4_exc_regs_t, flags));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, sp)     - OFF != offsetof(l4_exc_regs_t, sp));

	return (l4_exc_regs_t *)((char *)vcpu_regs + OFF);
}
#endif

#endif /* ARCH_x86 */

#ifdef ARCH_arm
static void l4x_setup_die_utcb(l4_exc_regs_t *exc)
{
	struct pt_regs regs;
	extern void die(const char *msg, struct pt_regs *regs, int err);
	static char message[40];

	snprintf(message, sizeof(message), "Boom!");
	message[sizeof(message) - 1] = 0;

	utcb_exc_to_ptregs(exc, &regs);
	regs.ARM_ORIG_r0 = 0;
	l4x_set_kernel_mode(&regs);

	/* Copy pt_regs on the stack */
	exc->sp -= sizeof(struct pt_regs);
	*(struct pt_regs *)exc->sp = regs;

	/* Put arguments for die into registers */
	exc->r[0] = (unsigned long)message;
	exc->r[1] = exc->sp;
	exc->r[2] = exc->err;

	/* Set PC to die function */
	exc->pc  = (unsigned long)die;
	exc->ulr = 0;
}

static void l4x_arm_set_reg(l4_exc_regs_t *exc, int num, unsigned long val)
{
	if (num > 15) {
		LOG_printf("Invalid register: %d\n", num);
		return;
	}

	switch (num) {
		case 15: exc->pc = val;  break;
		case 14: exc->ulr = val; break;
		default: exc->r[num] = val;
	}
}

#ifdef CONFIG_VFP
#include <asm/generic/fpu.h>
asm(
".global l4x_return_from_do_vfp    \t\n"
"l4x_return_from_do_vfp:           \t\n"
"	ldmia sp!, {r0-r12,lr,pc}  \t\n"
);

#include <asm/vfp.h>
#define  __L4X_ARM_VFP_H__USE_REAL_FUNCS
#include "../../arm/vfp/vfpinstr.h"

#include <asm/l4x/fpu.h>

unsigned l4x_fmrx(unsigned long reg)
{
	switch (reg) {
		case FPEXC:   return l4x_fpu_get(smp_processor_id())->fpexc;
		case FPSCR:   return fmrx(cr1);
		case FPSID:   return fmrx(cr0);
		case MVFR0:   return fmrx(cr7);
		case MVFR1:   return fmrx(cr6);
		case FPINST:  return l4x_fpu_get(smp_processor_id())->fpinst;
		case FPINST2: return l4x_fpu_get(smp_processor_id())->fpinst2;
		default: printk("Invalid fmrx-reg: %ld\n", reg);
	}
	return 0;
}

void l4x_fmxr(unsigned long reg, unsigned long val)
{
	switch (reg) {
		case FPEXC:
			l4x_fpu_get(smp_processor_id())->fpexc = val;
			return;
		case FPSCR:
			fmxr(cr1, val);
			return;
		default: printk("Invalid fmxr-reg: %ld\n", reg);
	}
	return;
}
#endif

static int l4x_arm_instruction_emu(l4_exc_regs_t *exc)
{
	unsigned long pc = exc->pc - 4;
	unsigned long op;

	if (pc < (unsigned long)&_stext || pc > (unsigned long)&_etext)
		return 1; // not for us

	op = *(unsigned long *)pc;

	if ((op & 0xff000000) == 0xee000000) {
		// always, mrc
		unsigned int reg;

		op &= 0x00ffffff;
		reg = (op >> 12) & 0xf;
		// see also cputype.h!
		if ((op & 0x00ff0fff) == 0x00100f30) {
			LOG_printf("Read Cache Type Register, to r%d\n", reg);
			// 32kb i/d cache
			l4x_arm_set_reg(exc, reg, 0x1c192992);
			return 0;
		} else if ((op & 0x00ff0fff) == 0x100f10) {
			// mrc     15, 0, xx, cr0, cr0, {0}
			LOG_printf("Read ID code register, to r%d\n", reg);
			l4x_arm_set_reg(exc, reg, 0x860f0001);
			return 0;
		} else if ((op & 0x00ff0fff) == 0x00100f91) {
			// mrc     15, 0, xx, cr0, cr1, {4}
			LOG_printf("Read memory model feature reg0 to r%d\n", reg);
			l4x_arm_set_reg(exc, reg, 0);
			return 0;
		} else if (   (op & 0x00000f00) == 0xa00
		           || (op & 0x00000f00) == 0xb00) {
			LOG_printf("Copro10/11 access (FPU), invalid, will oops\n");
			return 1; // not for use, will oops
		}
		LOG_printf("Unknown MRC: %lx at %lx\n", op, exc->pc);
	}

	return 1; // not for us
}

static inline void l4x_print_exception(l4_cap_idx_t t, l4_exc_regs_t *exc)
{
	LOG_printf("EX: "l4util_idfmt": "
	           "pc="l4_addr_fmt" "
	           "sp="l4_addr_fmt" "
	           "err=0x%lx lr=%lx\n",
	           l4util_idstr(t), exc->pc, exc->sp, exc->err, exc->ulr);

	if (l4x_debug_show_exceptions >= 2
	    && !l4_utcb_exc_is_pf(exc)
	    && (exc->pc & 3) == 0) {
		/* Lets assume we can do the following... */
		unsigned len = 72 >> 2, i;
		unsigned long pc = exc->pc - 44;

		LOG_printf("Dump: ");
		for (i = 0; i < len; i++, pc += sizeof(unsigned long))
			if (pc == exc->pc)
				LOG_printf("<%08lx> ", *(unsigned long *)pc);
			else
				LOG_printf("%08lx ", *(unsigned long *)pc);

		LOG_printf(".\n");
	}
}

#ifdef CONFIG_L4_VCPU
static inline l4_exc_regs_t *cast_to_utcb_exc(l4_vcpu_regs_t *vcpu_regs)
{
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, pfa)   != offsetof(l4_exc_regs_t, pfa));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, err)   != offsetof(l4_exc_regs_t, err));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, r)     != offsetof(l4_exc_regs_t, r));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, sp)    != offsetof(l4_exc_regs_t, sp));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, lr)    != offsetof(l4_exc_regs_t, ulr));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, ip)    != offsetof(l4_exc_regs_t, pc));
	BUILD_BUG_ON(offsetof(l4_vcpu_regs_t, flags) != offsetof(l4_exc_regs_t, cpsr));

	return (l4_exc_regs_t *)(vcpu_regs);
}
#endif

#endif /* ARCH_arm */

struct l4x_exception_func_struct {
	int           (*f)(l4_exc_regs_t *exc);
	unsigned long trap_mask;
	int           for_vcpu;
};
static struct l4x_exception_func_struct l4x_exception_func_list[] = {
#ifdef CONFIG_X86
	{ .trap_mask = 0x2000, .for_vcpu = 1, .f = l4x_handle_hlt_for_bugs_test }, // before kprobes!
#ifdef CONFIG_KPROBES
	{ .trap_mask = 0x2000, .for_vcpu = 1, .f = l4x_handle_kprobes },
#endif
	{ .trap_mask = 0x2000, .for_vcpu = 0, .f = l4x_handle_lxsyscall },
	{ .trap_mask = 0x0002, .for_vcpu = 0, .f = l4x_handle_int1 },
	{ .trap_mask = 0x2000, .for_vcpu = 1, .f = l4x_handle_msr },
	{ .trap_mask = 0x2000, .for_vcpu = 1, .f = l4x_handle_clisti },
	{ .trap_mask = 0x4000, .for_vcpu = 0, .f = l4x_handle_ioport },
#endif
#ifdef ARCH_arm
	{ .trap_mask = ~0UL,   .for_vcpu = 1, .f = l4x_arm_instruction_emu },
#endif
};
static const int l4x_exception_funcs
	= sizeof(l4x_exception_func_list) / sizeof(l4x_exception_func_list[0]);

static inline int l4x_handle_pagefault(unsigned long pfa, unsigned long ip,
                                       int wr)
{
	l4_addr_t addr;
	unsigned long size;
	l4re_ds_t ds;
	l4_addr_t offset;
	unsigned flags;
	int r;

	/* Check if the page-fault is a resolvable one */
	addr = pfa;
	if (addr < L4_PAGESIZE)
		return 0; // will trigger an Ooops
	size = 1;
	r = l4re_rm_find(&addr, &size, &offset, &flags, &ds);
	if (r == -L4_ENOENT) {
		/* Not resolvable: Ooops */
		LOG_printf("Non-resolvable page fault at %lx, ip %lx.\n", pfa, ip);
		// will trigger an oops in caller
		return 0;
	}

	/* Forward PF to our pager */
	return l4x_forward_pf(pfa, ip, wr);
}

#ifdef CONFIG_L4_VCPU
int l4x_vcpu_handle_kernel_pf(unsigned long pfa, unsigned long ip, int wr)
{
	return l4x_handle_pagefault(pfa, ip, wr);
}

// return true when exception handled successfully, false if not
int l4x_vcpu_handle_kernel_exc(l4_vcpu_regs_t *vr)
{
	int i;
	l4_exc_regs_t *exc = cast_to_utcb_exc(vr);
	// check handlers for this exception
	for (i = 0; i < l4x_exception_funcs; i++) {
		struct l4x_exception_func_struct *f
		  = &l4x_exception_func_list[i];
		if (f->for_vcpu
	            && ((1 << l4_utcb_exc_typeval(exc)) & f->trap_mask)
		    && !f->f(exc))
			break;
	}
	return i != l4x_exception_funcs;
}
#endif

static int l4x_default(l4_cap_idx_t *src_id, l4_msgtag_t *tag)
{
	l4_exc_regs_t exc;
	l4_umword_t pfa;
	l4_umword_t pc;

	pfa = l4_utcb_mr()->mr[0];
	pc  = l4_utcb_mr()->mr[1];

	memcpy(&exc, l4_utcb_exc(), sizeof(exc));

#if 0
	if (!l4_msgtag_is_exception(*tag)
	    && !l4_msgtag_is_io_page_fault(*tag)) {
		static unsigned long old_pf_addr = ~0UL, old_pf_pc = ~0UL;
		if (unlikely(old_pf_addr == (pfa & ~1) && old_pf_pc == pc)) {
			LOG_printf("Double page fault pfa=%08lx pc=%08lx\n",
			           pfa, pc);
			enter_kdebug("Double pagefault");
		}
		old_pf_addr = pfa & ~1;
		old_pf_pc   = pc;
	}
#endif

	if (l4_msgtag_is_exception(*tag)) {
		int i;

		if (l4x_debug_show_exceptions)
			l4x_print_exception(*src_id, &exc);

		// check handlers for this exception
		for (i = 0; i < l4x_exception_funcs; i++)
			if (((1 << l4_utcb_exc_typeval(&exc)) & l4x_exception_func_list[i].trap_mask)
			    && !l4x_exception_func_list[i].f(&exc))
				break;

		// no handler wanted to handle this exception
		if (i == l4x_exception_funcs) {
			if (l4_utcb_exc_is_pf(&exc)
			    && l4x_handle_pagefault(l4_utcb_exc_pfa(&exc),
			                            l4_utcb_exc_pc(&exc), 0)) {
				*tag = l4_msgtag(0, 0, 0, 0);
				pfa = pc = 0;
				return 0; // reply
			}

			l4x_setup_die_utcb(&exc);
		}

		*tag = l4_msgtag(0, L4_UTCB_EXCEPTION_REGS_SIZE, 0, 0);
		memcpy(l4_utcb_exc(), &exc, sizeof(exc));
		return 0; // reply

#ifdef CONFIG_X86
	} else if (l4_msgtag_is_io_page_fault(*tag)) {
		LOG_printf("Invalid IO-Port access at pc = "l4_addr_fmt
		           " port=0x%lx\n", pc, pfa >> 12);

		l4x_handle_ioport_pending_pc = pc;

		/* Make exception out of PF */
		*tag = l4_msgtag(-1, 1, 0, 0);
		l4_utcb_mr()->mr[0] = -1;
		return 0; // reply
#endif
	} else if (l4_msgtag_is_page_fault(*tag)) {
		if (l4x_debug_show_exceptions)
			LOG_printf("Page fault: addr = " l4_addr_fmt
			           " pc = " l4_addr_fmt " (%s%s)\n",
			           pfa, pc,
			           pfa & 2 ? "rw" : "ro",
			           pfa & 1 ? ", T" : "");

		if (l4x_handle_pagefault(pfa, pc, !!(pfa & 2))) {
			*tag = l4_msgtag(0, 0, 0, 0);
			return 0;
		}

		LOG_printf("Page fault (non-resolved): pfa=%lx pc=%lx\n", pfa, pc);
		/* Make exception out of PF */
		*tag = l4_msgtag(-1, 1, 0, 0);
		l4_utcb_mr()->mr[0] = -1;
		return 0; // reply
	}

	l4x_print_exception(*src_id, &exc);

	return 1; // no reply
}

enum {
	L4X_SERVER_EXIT = 1,
};

static void __noreturn l4x_server_loop(void)
{
	int do_wait = 1;
	l4_msgtag_t tag = (l4_msgtag_t){0};
	l4_umword_t src = 0;

	while (1) {

		while (do_wait)
			do_wait = l4_msgtag_has_error(tag = l4_ipc_wait(l4_utcb(), &src, L4_IPC_NEVER));

		switch (l4_msgtag_label(tag)) {
			case L4X_SERVER_EXIT:
				l4x_linux_main_exit(); // will not return anyway
				do_wait = 1;
				break;
			default:
				if (l4x_default(&src, &tag)) {
					do_wait = 1;
					continue; // do not reply
				}
		}

		tag = l4_ipc_reply_and_wait(l4_utcb(), tag,
		                            &src, L4_IPC_SEND_TIMEOUT_0);
		do_wait = l4_msgtag_has_error(tag);
	}
}


void __noreturn l4x_exit_l4linux(void)
{
	l4_msgtag_t tag = l4_msgtag(L4X_SERVER_EXIT, 0, 0, 0);

	__cxa_finalize(0);

	if (l4_msgtag_has_error(l4_ipc_send(l4x_start_thread_id, l4_utcb(), tag, L4_IPC_NEVER)))
		outstring("IPC ERROR l4x_exit_l4linux\n");
	l4_sleep_forever();
}

/* ---------------------------------------------------------------- */
/* swsusp stuff */

#ifdef CONFIG_PM
int arch_prepare_suspend(void)
{
	LOG_printf("%s\n", __func__);
	return 0;
}

void l4x_swsusp_before_resume(void)
{
	// make our AS readonly so that we see all PFs
#if 0
	l4_fpage_unmap(l4_fpage(0, 31, 0, 0),
	               L4_FP_REMAP_PAGE | L4_FP_ALL_SPACES);
#endif
}

void l4x_swsusp_after_resume(void)
{
	LOG_printf("%s\n", __func__);
}


/* ---- */
/* we need to remember virtual mappings to restore them after resume */

#include <asm/generic/vmalloc.h>
#include <asm/generic/suspres.h>
#include <asm/l4lxapi/memory.h>

struct l4x_virtual_mem_struct {
	struct list_head list;
	unsigned long address, page;
};

static LIST_HEAD(virtual_pages);

enum l4x_virtual_mem_type {
	L4X_VIRTUAL_MEM_TYPE_MAP,
	L4X_VIRTUAL_MEM_TYPE_UNMAP,
};

void l4x_virtual_mem_register(unsigned long address, unsigned long page)
{
	struct l4x_virtual_mem_struct *e;
	if (!(e = kmalloc(sizeof(*e), GFP_KERNEL)))
		BUG();
	e->address = address;
	e->page    = page;
	list_add_tail(&e->list, &virtual_pages);
}

void l4x_virtual_mem_unregister(unsigned long address)
{
	struct list_head *p, *tmp;
	list_for_each_safe(p, tmp, &virtual_pages) {
		struct l4x_virtual_mem_struct *e
		 = list_entry(p, struct l4x_virtual_mem_struct, list);
		if (e->address == address) {
			list_del(p);
			kfree(e);
		}
	}
}

static void l4x_virtual_mem_handle_pages(enum l4x_virtual_mem_type t)
{
	struct list_head *p;
	list_for_each(p, &virtual_pages) {
		struct l4x_virtual_mem_struct *e
		 = list_entry(p, struct l4x_virtual_mem_struct, list);

		if (t == L4X_VIRTUAL_MEM_TYPE_MAP) {
			LOG_printf("map virtual %lx -> %lx\n", e->address, e->page);
			l4lx_memory_map_virtual_page(e->address, e->page, 1);
		} else {
			LOG_printf("unmap virtual %lx\n", e->address);
			l4lx_memory_unmap_virtual_page(e->address);
		}
	}
}

static LIST_HEAD(suspres_func_list);

void l4x_suspend_resume_register(void (*func)(enum l4x_suspend_resume_state),
                                 struct l4x_suspend_resume_struct *e)
{
	if (!e)
		return;

	e->func = func;
	list_add(&e->list, &suspres_func_list);
}

static void l4x_suspend_resume_call_funcs(enum l4x_suspend_resume_state state)
{
	struct list_head *p;
	list_for_each(p, &suspres_func_list) {
		struct l4x_suspend_resume_struct *e
		 = list_entry(p, struct l4x_suspend_resume_struct, list);
		e->func(state);
	}
}


#include <linux/pm.h>
#include <linux/platform_device.h>

static int l4x_power_mgmt_suspend(struct platform_device *dev, pm_message_t state)
{
	// needs fixing for vCPU mode if someone wants that
#ifndef CONFIG_L4_VCPU
	struct task_struct *p;

	LOG_printf("%s (state = %d)\n", __func__, state.event);

	for_each_process(p) {
		if (l4_is_invalid_cap(p->thread.user_thread_id))
			continue;

		// FIXME: destroy better, threads and task
		if (!l4lx_task_delete_task(p->thread.user_thread_id, 1))
			LOG_printf("Error deleting %s(%d)\n", p->comm, p->pid);
		if (l4lx_task_number_free(p->thread.user_thread_id))
			LOG_printf("Error freeing %s(%d)\n", p->comm, p->pid);
		p->thread.user_thread_id = L4_INVALID_CAP;
		LOG_printf("kicked %s(%d)\n", p->comm, p->pid);
	}
#endif

	l4x_suspend_resume_call_funcs(L4X_SUSPEND);
	l4x_virtual_mem_handle_pages(L4X_VIRTUAL_MEM_TYPE_UNMAP);
	return 0;
}

static int l4x_power_mgmt_resume(struct platform_device *dev)
{
#ifndef CONFIG_L4_VCPU
	struct task_struct *p;
#endif

	LOG_printf("%s\n", __func__);

	l4x_virtual_mem_handle_pages(L4X_VIRTUAL_MEM_TYPE_MAP);
	l4x_suspend_resume_call_funcs(L4X_RESUME);

	// needs fixing for vCPU mode if someone wants that
#ifndef CONFIG_L4_VCPU
	for_each_process(p) {
		l4_msgtag_t tag;
		l4_umword_t src_id;

		if (l4_is_invalid_cap(p->thread.user_thread_id))
			continue;

		if (l4lx_task_get_new_task(L4_INVALID_CAP,
		                           &p->thread.user_thread_id))
			LOG_printf("l4lx_task_get_new_task failed\n");
		if (l4lx_task_create(p->thread.user_thread_id))
			LOG_printf("l4lx_task_create for %s(%d) failed\n",
			           p->comm, p->pid);

		do {
			tag = l4_ipc_wait(l4_utcb(), &src_id, L4_IPC_SEND_TIMEOUT_0);
			if (l4_ipc_error(tag, l4_utcb()))
				LOG_printf("ipc error %lx\n", l4_ipc_error(tag, l4_utcb()));
		} while ( 1 ); //FIXME //!l4_thread_equal(src_id, p->thread.user_thread_id));

		LOG_printf("contacted %s(%d)\n", p->comm, p->pid);
	}
#endif

	return 0;
}

static struct platform_driver l4x_power_mgmt_drv = {
	.suspend = l4x_power_mgmt_suspend,
	.resume  = l4x_power_mgmt_resume,
	.driver  = {
		.name = "l4x_power_mgmt",
	},
};

static void l4x_power_mgmt_platform_release(struct device *device)
{}

static struct platform_device l4x_power_mgmt_dev = {
	.name = "l4x_power_mgmt",
	.dev = {
		.release = l4x_power_mgmt_platform_release,
	}
};

static int l4x_power_mgmt_init(void)
{
	int ret;
	if (!(ret = platform_driver_register(&l4x_power_mgmt_drv))) {
		ret = platform_device_register(&l4x_power_mgmt_dev);
		if (ret)
			platform_driver_unregister(&l4x_power_mgmt_drv);
	}
	return ret;
}

module_init(l4x_power_mgmt_init);
#endif /* CONFIG_PM */


void exit(int code)
{
	__cxa_finalize(0);
	l4x_external_exit(code);
	LOG_printf("Still alive, going zombie???\n");
	l4_sleep_forever();
}

/* -------------------------------------------------- */
/*   Maybe for l4util                                 */

/**
 * Set the PC of a thread, leaving the SP where it is.
 */
void l4x_thread_set_pc(l4_cap_idx_t thread, void *pc)
{
	l4_thread_ex_regs(thread, (l4_umword_t)pc, ~0UL, 0);
}


/* -------------------------------------------------- */

int l4x_peek_upage(unsigned long addr,
                   unsigned long __user *datap,
                   int *ret)
{
	unsigned long tmp;

	if (addr >= UPAGE_USER_ADDRESS
	    && addr < UPAGE_USER_ADDRESS + PAGE_SIZE) {
		addr -= UPAGE_USER_ADDRESS;
		tmp = *(unsigned long *)(addr + upage_addr);
		*ret = put_user(tmp, datap);
		return 1;
	}
	return 0;
}

/* ----------------------------------------------------- */

void l4x_printk_func(char *buf, int len)
{
	outnstring(buf, len);
}

#ifndef CONFIG_L4_VCPU
#include <linux/hardirq.h>

/* ----------------------------------------------------- */
/* Prepare a thread for use as an IRQ thread. */
void l4x_prepare_irq_thread(struct thread_info *ti, unsigned _cpu)
{
	/* Stack setup */
	*ti = (struct thread_info) INIT_THREAD_INFO(init_task);

	l4x_stack_setup(ti, l4_utcb(), _cpu);
	barrier();

	ti->task          = l4x_idle_task(_cpu);
	ti->exec_domain   = NULL;
	ti->cpu           = _cpu;
	ti->preempt_count = HARDIRQ_OFFSET;
#ifdef ARCH_x86
	ti->addr_limit    = MAKE_MM_SEG(0);
#endif

	/* Set pager */
	l4lx_thread_set_kernel_pager(l4x_stack_id_get());

#if defined(CONFIG_SMP) && defined(ARCH_x86)
	l4x_load_percpu_gdt_descriptor(get_cpu_gdt_table(_cpu));
#endif
}
#endif

/* ----------------------------------------------------- */
void l4x_show_process(struct task_struct *t)
{
#ifdef CONFIG_L4_VCPU
#ifdef CONFIG_X86
	printk("%2d: %s tsk st: %lx thrd flgs: %x esp: %08lx\n",
	       t->pid, t->comm, t->state, task_thread_info(t)->flags,
	       t->thread.sp);
#endif
#ifdef ARCH_arm
	printk("%2d: %s tsk st: %lx thrd flgs: %lx esp: %08x\n",
	       t->pid, t->comm, t->state, task_thread_info(t)->flags,
	       task_thread_info(t)->cpu_context.sp);
#endif
#else
#ifdef CONFIG_X86
	printk("%2d: %s tsk st: %lx thrd flgs: %x " PRINTF_L4TASK_FORM " esp: %08lx\n",
	       t->pid, t->comm, t->state, task_thread_info(t)->flags,
	       PRINTF_L4TASK_ARG(t->thread.user_thread_id),
	       t->thread.sp);
#endif
#ifdef ARCH_arm
	printk("%2d: %s tsk st: %lx thrd flgs: %lx " PRINTF_L4TASK_FORM " esp: %08x\n",
	       t->pid, t->comm, t->state, task_thread_info(t)->flags,
	       PRINTF_L4TASK_ARG(t->thread.user_thread_id),
	       task_thread_info(t)->cpu_context.sp);
#endif
#endif
}

void l4x_show_processes(void)
{
	struct task_struct *t;
	for_each_process(t) {
		if (t->pid >= 10)
			l4x_show_process(t);
	}
	printk("c");
	l4x_show_process(current);
}

void l4x_show_sigpending_processes(void)
{
	struct task_struct *t;
	printk("Processes with pending signals:\n");
	for_each_process(t) {
		if (signal_pending(t))
			l4x_show_process(t);
	}
	printk("Signal list done.\n");

}

/* Just a function we can call without defining the header files */
void kdb_ke(void)
{
	enter_kdebug("kdb_ke");
}

#ifdef CONFIG_L4_DEBUG_SEGFAULTS
#include <linux/fs.h>
void l4x_print_vm_area_maps(struct task_struct *p, unsigned long highlight)
{
	struct vm_area_struct *vma = p->mm->mmap;
	L4XV_V(f);


	L4XV_L(f);
	while (vma) {
		struct file *file = vma->vm_file;
		int flags = vma->vm_flags;

		printk("%08lx - %08lx %c%c%c%c",
		       vma->vm_start, vma->vm_end,
		       flags & VM_READ ? 'r' : '-',
		       flags & VM_WRITE ?  'w' : '-',
		       flags & VM_EXEC ?  'x' : '-',
		       flags & VM_MAYSHARE ?  's' : 'p');

		if (file) {
			char buf[40];
			int count = 0;
			char *s = buf;
			char *p = d_path(&file->f_path, s, sizeof(buf));

			printk(" %05lx", vma->vm_pgoff);

			if (!IS_ERR(p)) {
				while (s <= p) {
					char c = *p++;
					if (!c) {
						p = buf + count;
						count = s - buf;
						buf[count] = 0;
						break;
					} else if (!strchr("", c)) {
						*s++ = c;
					} else if (s + 4 > p) {
						break;
					} else {
						*s++ = '\\';
						*s++ = '0' + ((c & 0300) >> 6);
						*s++ = '0' + ((c & 070) >> 3);
						*s++ = '0' + (c & 07);
					}
				}
			}
			printk(" %s", count ? buf : "Unknown");
		}

		if (   highlight >= vma->vm_start
		    && highlight < vma->vm_end)
			printk(" <====");
		printk("\n");
		vma = vma->vm_next;
	}
	L4XV_U(f);
}
#endif

#ifdef CONFIG_X86

#include <linux/clockchips.h>

struct clock_event_device *global_clock_event;

static void clock_init_l4_timer(enum clock_event_mode mode,
                                struct clock_event_device *evt)
{
}

static int clock_l4_next_event(unsigned long delta, struct clock_event_device *evt)
{
	return 0;
}

struct clock_event_device l4_clockevent = {
	.name		= "l4",
	.features	= CLOCK_EVT_FEAT_PERIODIC,
	.set_mode	= clock_init_l4_timer,
	.set_next_event	= clock_l4_next_event,
	.shift		= 32,
	.irq		= 0,
	.mult		= 1,
};

DEFINE_RAW_SPINLOCK(i8253_lock);
EXPORT_SYMBOL(i8253_lock);

void setup_pit_timer(void)
{
	l4_clockevent.cpumask = cpumask_of(0);
	clockevents_register_device(&l4_clockevent);
	global_clock_event = &l4_clockevent;
}
#endif

/* ----------------------------------------------------------------------- */
/* Export list, we could also put these in a separate file (like l4_ksyms.c) */

#ifdef CONFIG_X86
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>

#include <asm/i387.h>
#include <asm/checksum.h>
#include <asm/delay.h>
#include <asm/dma-mapping.h>
#include <asm/io.h>

#ifdef CONFIG_X86_32
EXPORT_SYMBOL(__VMALLOC_RESERVE);
#endif

EXPORT_SYMBOL(swapper_pg_dir);
EXPORT_SYMBOL(init_thread_union);

#ifdef CONFIG_X86_32
EXPORT_SYMBOL(strstr);
#endif

EXPORT_SYMBOL(csum_partial);
#ifdef CONFIG_X86_32
EXPORT_SYMBOL(csum_partial_copy);
#endif
EXPORT_SYMBOL(empty_zero_page);

#ifdef CONFIG_X86_64
EXPORT_SYMBOL(memcpy);
#endif

/*
 * Note, this is a prototype to get at the symbol for
 * the export, but dont use it from C code, it is used
 * by assembly code and is not using C calling convention!
 */
#ifndef CONFIG_X86_CMPXCHG64
extern void cmpxchg8b_emu(void);
EXPORT_SYMBOL(cmpxchg8b_emu);
#endif

EXPORT_SYMBOL(l4x_fpu_set);

#endif /* ARCH_x86 */

/* Some exports from L4 libraries etc. */
EXPORT_SYMBOL(LOG_printf);

EXPORT_SYMBOL(l4x_vmalloc_memory_start);

/* Exports for L4 specific modules */
EXPORT_SYMBOL(l4_sleep);

char l4x_ipc_errstrings[0];
EXPORT_SYMBOL(l4x_ipc_errstrings);

#ifndef CONFIG_L4_VCPU
EXPORT_SYMBOL(l4x_prepare_irq_thread);
#endif

EXPORT_SYMBOL(l4lx_task_create);
EXPORT_SYMBOL(l4lx_task_get_new_task);
EXPORT_SYMBOL(l4lx_task_number_free);
EXPORT_SYMBOL(l4lx_task_delete_task);
EXPORT_SYMBOL(l4re_global_env);
#ifdef ARCH_x86
EXPORT_SYMBOL(l4_utcb_wrap);
extern char __l4sys_invoke_direct[];
EXPORT_SYMBOL(__l4sys_invoke_direct);

#ifdef CONFIG_FUNCTION_TRACER
/* mcount is defined in assembly */
EXPORT_SYMBOL(mcount);
#endif
#endif
