PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

SRC_CC   := of.cc
SYSTEMS  := x86 arm amd64 ppc32
TARGET    = libdrivers_of.a

PRIVATE_INCDIR += $(SRC_DIR)/../include 

include $(L4DIR)/mk/lib.mk

CXXFLAGS += -DL4_NO_RTTI -fno-exceptions -fno-rtti
