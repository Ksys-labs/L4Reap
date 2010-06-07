include $(BOEHM_GC_SRCDIR)/mk/includes.inc

DEFINES        += -D__L4

vpath %.c $(BOEHM_GC_CONTRIBDIR)

SRC_PTHREAD_C := \
	specific.c \
	pthread_stop_world.c \
	thread_local_alloc.c \
	pthread_support.c \
	libatomic_ops/src/atomic_ops.c \
	libatomic_ops/src/atomic_ops_stack.c \
	libatomic_ops/src/atomic_ops_malloc.c \

SRC_C := \
	stubborn.c \
	ptr_chck.c \
	new_hblk.c \
	pcr_interface.c \
	dyn_load.c \
	finalize.c \
	allchblk.c \
	malloc.c \
	mach_dep.c \
	mark_rts.c \
	blacklst.c \
	obj_map.c \
	real_malloc.c \
	mark.c \
	typd_mlc.c \
	headers.c \
	misc.c \
	backgraph.c \
	reclaim.c \
	checksums.c \
	alloc.c \
	mallocx.c

#	dbg_mlc.c \


include $(L4DIR)/mk/lib.mk

CFLAGS := $(filter-out -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations, $(CFLAGS))

CFLAGS += -Wno-unused-parameter


