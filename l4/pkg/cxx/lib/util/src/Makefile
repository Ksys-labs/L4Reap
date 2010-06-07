PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libcxx_util
PC_FILENAME     = cxx_util
SRC_CC		= ipc_error_str.cc alloc_list.cc
SYSTEMS		= x86 arm amd64 ppc32

include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(L4DIR)/pkg/cxx/lib/Makefile.inc
