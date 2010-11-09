PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET           = morpork
SYSTEMS          = x86-l4f arm-l4f
L4_MULTITHREADED = y
SRC_CC           = main.cc arping.cc
REQUIRES_LIBS    = l4util l4re l4re-util shmc_ringbuf libstdc++ libboost-lite

include $(L4DIR)/mk/prog.mk
