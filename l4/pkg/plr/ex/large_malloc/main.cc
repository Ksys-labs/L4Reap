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
#include <l4/re/env.h>
#include <l4/sys/kdebug.h>

#include "log"

#define NUM_SIZES 5

/*
 */
static inline long long US(struct timeval& tv)
{
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(void)
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

	enter_kdebug("I am done.");
	return 0;
}
