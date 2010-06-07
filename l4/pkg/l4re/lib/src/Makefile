PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

include $(PKGDIR)/Makefile.inc

TARGET	      = lib4re.a lib4re.so
REQUIRES_LIBS = l4util l4sys

SRC_CC	= namespace.cc dataspace.cc rm.cc mem_alloc.cc parent.cc \
	  log.cc debug.cc event.cc video/goos.cc video/view.cc
SRC_C_lib4re.a   := env.c

include $(L4DIR)/mk/lib.mk

CFLAGS += -fvisibility=hidden
CXXFLAGS += -fvisibility=hidden
