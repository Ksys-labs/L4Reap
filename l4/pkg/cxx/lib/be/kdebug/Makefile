PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libcxx_io_kdebug
PC_FILENAME     = cxx_io_kdebug
SYSTEMS		= x86 arm amd64 ppc32
SRC_CC		= iostream.cc

include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(L4DIR)/pkg/cxx/lib/Makefile.inc
