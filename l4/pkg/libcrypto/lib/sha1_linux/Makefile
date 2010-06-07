PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_sha1_linux.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86 arm amd64
PC_FILENAME = $(PKGNAME)_sha1_linux

# list your .c files here
SRC_C	= sha1.c

include $(L4DIR)/mk/lib.mk
