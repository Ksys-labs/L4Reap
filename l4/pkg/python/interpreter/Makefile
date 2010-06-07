PKGDIR := ..
L4DIR ?= $(PKGDIR)/../..

TARGET = python
MODE = shared
REQUIRES_LIBS = libpython libdl libc_be_file_stdin \
				libc_be_sig libc_be_math \
				readline

L4_MULTITHREADED ?= y

PRIVATE_INCDIR = $(PKGDIR_ABS)/include $(PKGDIR_ABS)/include/ARCH-$(ARCH)

SRC_CC = python.cc 

#vpath %.cc $(PKGDIR_ABS)

PRIVATE_INCDIR += $(PKGDIR_ABS)/contrib/Include

LDFLAGS = --export-dynamic

include $(L4DIR)/mk/prog.mk
