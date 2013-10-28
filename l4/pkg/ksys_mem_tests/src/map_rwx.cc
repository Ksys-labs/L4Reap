#include <stdio.h>
#include <stdlib.h>

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/dataspace>

#include <l4/sigma0/sigma0.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/ktrace.h>

typedef void (*fptr)(void);

enum our_mem_flags {
	MAP_R = 1,
	MAP_W = 2,
	MAP_X = 4,
	MAP_RW = MAP_R | MAP_W,
	MAP_RX = MAP_R | MAP_X,
	MAP_WX = MAP_W | MAP_X,
	MAP_RWX = MAP_R | MAP_W | MAP_X,
};

typedef L4::Cap<L4Re::Dataspace> Dataspace;

static void* alloc_mem(size_t size, enum our_mem_flags flags) {
	void *ptr = 0;
	Dataspace ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	if (!ds.is_valid()) {
		puts("failed to get DS capability");
		exit(-1);
	}

	if (L4Re::Env::env()->mem_alloc()->alloc(size, ds,
		flags & MAP_X ? L4Re::Mem_alloc::Executable : 0))
	{
		puts("failed to allocate memory");
		exit(-1);
	}

	if (L4Re::Env::env()->rm()->attach(&ptr, size,
		L4Re::Rm::Search_addr | L4Re::Rm::Eager_map
		| ((flags & MAP_W) ? 0 : L4Re::Rm::Read_only)
		,ds))
	{
		puts("failed to attach memory");
		exit(-1);
	}

	return ptr;
}

int main() {
	volatile unsigned char buf_stack[4];
	volatile unsigned char *buf;
	volatile fptr func;

	puts("+ksys_mem_tests");

	buf = (volatile unsigned char*)alloc_mem(409600, MAP_RWX);
	printf("allocated memory @%016x\n", buf);
	fiasco_tbuf_log_3val("KSYS-MEM", (unsigned long)buf, 0, 0);

	#ifdef USE_ARM
	/* ARM BX LR instruction */
	((unsigned*)buf)[0] = 0xe12fff1e;
	#else
	/* X86 RETN opcode */
	buf[0] = '\xc3';
	#endif

	printf("before exec %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
	func = (fptr)buf;
	func();
	printf("buf %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

	printf("Dataspace test done, trying stack exec\n");

	#ifdef USE_ARM
	/* ARM BX LR instruction */
	((unsigned*)buf_stack)[0] = 0xe12fff1e;
	#else
	/* X86 RETN opcode */
	buf_stack[0] = '\xc3';
	#endif

	func = (fptr)buf_stack;
	func();
	printf("stack %x %x %x %x\n", buf_stack[0], buf_stack[1], buf_stack[2], buf_stack[3]);

	while (1) {};
}
