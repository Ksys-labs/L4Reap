PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= rtc
SRC_CC_x86	= x86.cc
SRC_CC_amd64	= x86.cc
SRC_C_x86       = ux.c
SRC_CC		= main.cc $(SRC_CC_$(ARCH))
SRC_C		= $(SRC_C_$(ARCH))
REQUIRES_LIBS   = libio cxx_libc_io cxx_io
LIBS_x86        = -lclxfux.o
LIBS            = $(LIBS_$(ARCH))

include $(L4DIR)/mk/prog.mk
