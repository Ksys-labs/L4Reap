PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../..

SYSTEMS        = x86-l4f arm-l4f

REQUIRES_LIBS  = l4util l4re_c l4re_c-util shmc shmc_ringbuf libstdc++ libboost-lite

L4_MULTITHREADED = y

TARGET         = morpork

SRC_CC          = main.cc arping.cc

include $(L4DIR)/mk/prog.mk
