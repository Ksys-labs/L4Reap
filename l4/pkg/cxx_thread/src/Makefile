PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../..

vpath %.cc $(PKGDIR)/src/ARCH-$(ARCH)

TARGET		= libcxx_thread
SRC_CC		= thread.cc thread-arch.cc
PC_FILENAME     = cxx_thread

include $(L4DIR)/pkg/cxx/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(L4DIR)/pkg/cxx/lib/Makefile.inc
