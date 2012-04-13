
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <l4/util/elf.h>
#include <l4/util/util.h>
#include <l4/log/log.h>
#include <l4/re/c/mem_alloc.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/util/cap_alloc.h>

#include <l4/sys/compiler.h>
#include <l4/sys/kdebug.h>
#ifdef CONFIG_L4_VCPU
#include <l4/sys/vcpu.h>
#endif

extern const char image_vmlinux_start[];

struct shared_data {
	unsigned long (*external_resolver)(void);
	L4_CV l4_utcb_t *(*l4lx_utcb)(void);
	void *l4re_global_env;
	void *kip;
};
struct shared_data exchg;

unsigned long __l4_external_resolver(void);

L4_CV l4_utcb_t *l4_utcb_wrap(void)
{
	if (exchg.l4lx_utcb)
		return exchg.l4lx_utcb();
	return l4_utcb_direct();
}

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset)
{
	LOG_printf("mmap called, should only happen for exceptions\n");
	return MAP_FAILED;
}

int munmap(void *addr, size_t length)
{
	LOG_printf("munmap called, should only happen for exceptions\n");
	return -1;
}


void l4x_external_exit(int code);
void l4x_external_exit(int code)
{
	_exit(code);
}

void do_resolve_error(const char *funcname);
void do_resolve_error(const char *funcname)
{
	LOG_printf("Symbol '%s' not found\n", funcname);
	enter_kdebug("Symbol not found!");
}

#ifdef ARCH_amd64
#define FMT "%016llx"
#else
#define FMT "%08x"
#endif

int main(int argc, char **argv)
{
	ElfW(Ehdr) *ehdr = (void *)image_vmlinux_start;
	int i;
	int (*entry)(int, char **);

	if (!l4util_elf_check_magic(ehdr)
	    || !l4util_elf_check_arch(ehdr)) {
		printf("lxldr: Invalid vmlinux binary (No ELF)\n");
		return 1;
	}

	for (i = 0; i < ehdr->e_phnum; ++i) {
		int r;
		l4_addr_t map_addr;
		unsigned long map_size;
		unsigned flags;
		l4re_ds_t ds, new_ds;
		l4_addr_t offset;

		ElfW(Phdr) *ph = (ElfW(Phdr)*)((l4_addr_t)l4util_elf_phdr(ehdr)
		                               + i * ehdr->e_phentsize);
		printf("PH %2d (t: %8d) offs="FMT" vaddr="FMT" vend="FMT"\n"
		       "                    phys="FMT" ephys="FMT"\n"
		       "                    f_sz="FMT" memsz="FMT" flgs=%c%c%c\n",
		       i, ph->p_type, ph->p_offset,
		       ph->p_vaddr, ph->p_vaddr + ph->p_memsz,
		       ph->p_paddr, ph->p_paddr + ph->p_memsz,
		       ph->p_filesz, ph->p_memsz,
		       ph->p_flags & PF_R ? 'r' : '-',
		       ph->p_flags & PF_W ? 'w' : '-',
		       ph->p_flags & PF_X ? 'x' : '-');
		if (ph->p_type != PT_LOAD)
			continue;

		if (ph->p_vaddr & ~L4_PAGEMASK) {
			printf("lxldr: unaligned section\n");
			continue;
		}

		if (ph->p_filesz < ph->p_memsz) {

			ds = l4re_util_cap_alloc();
			if (l4_is_invalid_cap(ds)) {
				printf("lxldr: out of caps\n");
				return 1;
			}

			if (l4re_ma_alloc(ph->p_memsz, ds,
			                  L4RE_MA_CONTINUOUS | L4RE_MA_PINNED)) {
				printf("lxldr: could not allocate memory\n");
				return 1;
			}

			map_addr = ph->p_vaddr;
			if (l4re_rm_attach((void **)&map_addr,
			                    ph->p_memsz, L4RE_RM_EAGER_MAP,
			                    ds, 0, 0)) {
				printf("lxldr: failed attaching memory:"
				       " "FMT" - "FMT"\n",
				       ph->p_vaddr,
				       ph->p_vaddr + ph->p_memsz - 1);
				return 1;
			}
#if 0
			printf("BSS copying %p -> %p (%lx), zero %p (%lx)\n",
			(void *)ph->p_vaddr,
			       (char *)image_vmlinux_start + ph->p_offset,
			       ph->p_filesz,
			       (void *)ph->p_vaddr + ph->p_filesz,
			       ph->p_memsz - ph->p_filesz);
#endif
			memcpy((void *)ph->p_vaddr,
			       (char *)image_vmlinux_start + ph->p_offset,
			       ph->p_filesz);
			memset((void *)ph->p_vaddr + ph->p_filesz, 0,
			       ph->p_memsz - ph->p_filesz);
			continue;
		}

		new_ds = l4re_util_cap_alloc();
		if (l4_is_invalid_cap(new_ds)) {
			printf("lxldr: out of caps\n");
			return 1;
		}

		if (l4re_ma_alloc(ph->p_memsz, new_ds,
		                  L4RE_MA_CONTINUOUS | L4RE_MA_PINNED)) {
			printf("lxldr: could not allocate memory\n");
			return 1;
		}

		/* We just need the dataspace and offset */
		map_addr = (l4_addr_t)image_vmlinux_start + ph->p_offset;
		map_size = ph->p_memsz;
		r = l4re_rm_find(&map_addr, &map_size, &offset, &flags, &ds);
		offset += ((l4_addr_t)image_vmlinux_start + ph->p_offset) - map_addr;
		if (r) {
			printf("lxldr: Failed lookup %p (%p + %lx): %d\n",
			       (char *)image_vmlinux_start + ph->p_offset,
			       (char *)image_vmlinux_start,
			       (unsigned long)ph->p_offset, r);
			return 1;
		}

		r = l4re_ds_copy_in(new_ds, 0, ds, offset, ph->p_memsz);
		if (r)
			printf("l4dm_mem_copyin failed\n");

		map_addr = ph->p_vaddr;
		if (l4re_rm_attach((void **)&map_addr,
		                    ph->p_memsz, L4RE_RM_EAGER_MAP,
		                    new_ds, 0, 0)) {
			printf("lxldr: failed to attach section\n");
			return 1;
		}
	}

	entry = (void *)ehdr->e_entry;
	printf("Starting binary at %p, argc=%d argv=%p *argv=%p argv0=%s\n",
	       entry, argc, argv, *argv, *argv);
	//printf("%x %x %x %x %x\n", *((char *)entry + 0), *((char *)entry + 1), *((char *)entry + 2), *((char *)entry + 3), *((char *)entry + 4));
	exchg.external_resolver = __l4_external_resolver;
	exchg.l4re_global_env  = l4re_global_env;
	exchg.kip              = l4re_kip();
	printf("External resolver is at %p\n", __l4_external_resolver);
#ifdef ARCH_arm
	{
		register unsigned long _argc  asm("r0") = argc;
		register unsigned long _argv  asm("r1") = (unsigned long)argv;
		register unsigned long _exchg asm("r2") = (unsigned long)&exchg;
		register unsigned long _entry asm("r3") = (unsigned long)entry;
		asm volatile("mov lr, pc   \n"
		             "mov pc, r3   \n"
			     "mov %0, r0   \n"
			     : "=r" (i)
			     : "r" (_argv),
			       "r" (_argc),
			       "r" (_exchg),
			       "r" (_entry)
			     : "memory");
	}
#elif defined(ARCH_x86)
	asm volatile("push %[argv]\n"
	             "push %[argc]\n"
	             "mov  %[exchg], %%esi\n"
	             "call  *%[entry]\n"
	             "pop %[argc]\n"
	             "pop %[argv]\n"
		     : "=a" (i)
		     : [argv] "r" (argv),
		       [argc] "r" (argc),
		       [exchg] "r" (&exchg),
		       [entry] "r" (entry)
		     : "memory", "esi");
#else
	asm volatile("movq  %[exchg], %%rcx\n"
	             "call  *%[entry]\n"
		     : "=a" (i)
		     : [argv] "S" (argv),
		       [argc] "D" ((unsigned long)argc),
		       [exchg] "r" (&exchg),
		       [entry] "r" (entry)
		     : "memory", "rcx");
#endif

	return i;
}
