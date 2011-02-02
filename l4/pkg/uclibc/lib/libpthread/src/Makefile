PKGDIR     ?= ../../..
L4DIR      ?= $(PKGDIR)/../..

TARGET      = r/libpthread.a r/libpthread.so

PC_FILENAME = libpthread
PC_LIBS_libpthread := -lpthread

SRC_CC      = manager.cc l4.cc
SRC_C      += spinlock.c mutex.c condvar.c rwlock.c errno.c specific.c \
              semaphore.c attr.c barrier.c join.c pthread.c \
              errno-loc.c cancel.c ptcleanup.c libc_pthread_init.c
SRC_S       = tramp-$(ARCH).S
SO_INIT     = so_init-$(ARCH)


INCDIR-x86   = $(SRC_DIR)/sysdeps/i386/i686 $(SRC_DIR)/sysdeps/i386 \
               $(PKGDIR)/lib/contrib/uclibc/libc
INCDIR-amd64 = $(SRC_DIR)/sysdeps/x86_64
INCDIR-arm   = $(SRC_DIR)/sysdeps/arm
INCDIR-ppc32 = $(SRC_DIR)/sysdeps/powerpc
INCDIR-sparc = $(SRC_DIR)/sysdeps/sparc

PRIVATE_INCDIR = $(SRC_DIR) $(INCDIR-$(ARCH))

DEFINES   := -DL4_THREAD_SAFE -D__USE_XOPEN2K -D_POSIX_C_SOURCE=200112L \
             -D_GNU_SOURCE=1 -D__NO_WEAK_PTHREAD_ALIASES=1 \
	     -D_LIBC=1 -DIS_IN_libpthread=1

CFLAGS    += -include $(PKGDIR)/lib/contrib/uclibc/include/libc-symbols.h
CXXFLAGS  += -include $(PKGDIR)/lib/contrib/uclibc/include/libc-symbols.h
CXXFLAGS  += -fno-exceptions

include $(L4DIR)/mk/lib.mk
CRT0 = $(CRTI_so) $(SO_INIT).s.o $(CRTBEGIN_so) $(CRT1_so)

CFLAGS := $(filter-out -Wextra,$(CFLAGS))
CXXFLAGS := $(filter-out -Wextra,$(CXXFLAGS))

r/libpthread.so: $(SO_INIT).s.o
