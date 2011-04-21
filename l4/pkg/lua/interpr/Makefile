PKGDIR  ?= ..
L4DIR   ?= $(PKGDIR)/../..

TARGET        = lua
SRC_C         = interpr.c
REQUIRES_LIBS = lua libc_support_misc libc_be_fs_noop

PRIVATE_INCDIR = $(SRC_DIR)/../lib/contrib/src

include $(L4DIR)/mk/prog.mk
