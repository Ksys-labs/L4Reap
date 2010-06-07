PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..

include $(L4DIR)/mk/Makeconf

ifneq ($(SYSTEM),)
  # check whether used gcc has the omp.h file available
  OMP_H_PATH := $(shell $(CC) -print-file-name=include/omp.h)

  ifeq ($(strip $(OMP_H_PATH)),include/omp.h)
    $(info [32m $(CC) does not have omp.h header file available, skipping. [0m)
    PC_FILENAME =
  else
   TARGET           = libgomp.a
 endif
endif
L4_MULTITHREADED = y

PRIVATE_INCDIR = $(PKGDIR)/lib/contrib/gcc-4.3/libgomp \
                 $(PKGDIR)/lib/contrib/gcc-4.3/libgomp/config/posix \
                 $(PKGDIR)/lib/build \
                 $(PKGDIR)/lib/build/ARCH-$(BUILD_ARCH)

vpath %.c $(PKGDIR)/lib/contrib/gcc-4.3/libgomp \
          $(PKGDIR)/lib/contrib/gcc-4.3/libgomp/config/posix

SRC_C = affinity.c alloc.c bar.c barrier.c critical.c env.c error.c \
        fortran.c iter.c lock.c loop.c mutex.c ordered.c parallel.c proc.c \
        sections.c sem.c single.c team.c time.c work.c

include $(L4DIR)/mk/lib.mk

CFLAGS := $(filter-out -Wextra,$(CFLAGS))
