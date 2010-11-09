PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET           = morping morpong
SYSTEMS          = x86-l4f arm-l4f
REQUIRES_LIBS    = l4util l4re-util shmc_ringbuf libstdc++ libboost-lite
L4_MULTITHREADED = y
SRC_CC_morping   = ping.cc
SRC_CC_morpong   = ping.cc

include $(L4DIR)/mk/prog.mk
