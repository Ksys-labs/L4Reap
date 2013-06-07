/*
 * large_malloc/main.cc --
 * 
 *    Try out a couple of malloc/l4_touch_rw/free sequences
 *    to figure out the overhead for Romain's memory management.
 *
 * (c) 2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>

#include <l4/util/util.h>
#include <l4/re/env>
#include <l4/re/mem_alloc>
#include <l4/re/rm>
#include <l4/re/util/cap_alloc>
#include <l4/sys/kdebug.h>

#include "log"

#define NUM_SIZES 5

/*
 */
static inline long long US(struct timeval& tv)
{
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void
__attribute__((used))
malloc_test()
{
	l4_umword_t sizes[NUM_SIZES] = {
		      1024 * 1024, // 1 MB
		 64 * 1024 * 1024, // 64 MB
		100 * 1024 * 1024, // 100 MB
		300 * 1024 * 1024, // 300 MB
		500 * 1024 * 1024, // 500 MB
	};

	for (unsigned i = 0; i < NUM_SIZES; ++i) {
		struct timeval tstart, tmalloc, ttouch, tfree;

		printf("%s==== %d -> %lx ====%s\n", BOLD_CYAN, i, sizes[i], NOCOLOR);

		gettimeofday(&tstart, 0);
		char *p = reinterpret_cast<char*>(malloc(sizes[i]));
		printf("malloc(%lx) = %p (%d)\n",
		       sizes[i], p, errno);
		if (!p)
			enter_kdebug("malloc");
		gettimeofday(&tmalloc, 0);
		l4_touch_rw(p, sizes[i]);
		gettimeofday(&ttouch, 0);
		printf("... touched.\n");
		free(p);
		gettimeofday(&tfree, 0);
		printf("%sfreed.%s\n", BOLD_GREEN, NOCOLOR);

		printf("malloc %10lld µs, touch %10lld µs, free %10lld µs\n",
		       US(tmalloc) - US(tstart),
		       US(ttouch)  - US(tmalloc),
		       US(tfree)   - US(ttouch));
	}
}


static void rm_test()
{
	unsigned size = 400 * 1024 * 1024; // 800 MB
	unsigned flags = 0; //L4Re::Mem_alloc::Super_pages | L4Re::Mem_alloc::Continuous;

	printf("Start RM test. %lx\n", flags);

	L4::Cap<L4Re::Dataspace> dscap = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	L4::Cap<L4Re::Dataspace> dscap2;
	printf("DS: %lx\n", dscap.cap());
	int error = L4Re::Env::env()->mem_alloc()->alloc(size, dscap, flags);
	if (error) {
		enter_kdebug("alloc fail");
	}

	l4_addr_t a = 0;
	error = L4Re::Env::env()->rm()->attach(&a, size, L4Re::Rm::Search_addr, dscap, 0);
	printf("Attached to %p\n", (void*)a);

	l4_touch_rw((void*)a, size);

	//L4Re::Env::env()->rm()->detach(a, &dscap2);
	//L4Re::Env::env()->mem_alloc()->free(dscap);

	printf("End RM test.\n");
}


int main(int argc, char **argv)
{
	int max_rounds = 2;
	struct timeval start, stop;
	if (argc > 1) {
		max_rounds = strtol(argv[1], 0, 0);
	}

	l4_sleep(2000);

	gettimeofday(&start, 0);
	for (unsigned i = 0; i < max_rounds; ++i) {
		//malloc_test();
		rm_test();
	}
	gettimeofday(&stop, 0);
	printf("Difference: %lld µs\n", US(stop) - US(start));


	return 0;
}
