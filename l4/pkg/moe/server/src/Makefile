PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= moe
DEFAULT_RELOC	= 0x0140000
RELOC_PHYS      = y
SRC_CC		= main.cc page_alloc.cc dataspace.cc dataspace_cont.cc \
                  dataspace_annon.cc globals.cc \
		  alloc.cc boot_fs.cc dataspace_static.cc dataspace_util.cc \
		  region.cc remote_mem.cc debug.cc \
		  loader.cc loader_elf.cc exception.cc \
		  app_task.cc dataspace_noncont.cc pages.cc \
		  name_space.cc mem.cc log.cc sched_proxy.cc \
		  delete.cc vesa_fb.cc gc_support.cc
SRC_S		:= ARCH-$(ARCH)/crt0.S
MODE		= sigma0

BOEHM_GC_SRCDIR    = $(L4DIR)/pkg/boehm_gc
PRIVATE_INCDIR    += $(SRC_DIR)/../libgc/include
include $(BOEHM_GC_SRCDIR)/mk/includes.inc

REQUIRES_LIBS  := libkproxy libloader l4re-util libsigma0 cxx_io \
                  cxx_libc_io libsupc++ libc_be_minimal_log_io
LIBS           += -L$(OBJ_DIR)/../libgc/OBJ-$(SYSTEM)
EXTRA_LIBS     := -ll4sys-direct -lmoe_gc

DEFINES        += -DL4_CXX_NO_EXCEPTION_BACKTRACE

ifeq ($(ARCH),ppc32)
LDSCRIPT	= $(SRC_DIR)/ARCH-$(ARCH)/main.ld
else
LDSCRIPT 	= $(SRC_DIR)/main.ld
endif

include $(L4DIR)/mk/prog.mk
