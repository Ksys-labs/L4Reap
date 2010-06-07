PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_pad.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86 arm amd64
PC_FILENAME = $(PKGNAME)_pad

# list your .c files here
SRC_C	= oaep.c

include $(L4DIR)/mk/lib.mk
