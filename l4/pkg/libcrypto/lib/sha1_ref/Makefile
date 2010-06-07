PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_sha1_ref.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86 arm amd64
PC_FILENAME = $(PKGNAME)_sha1_ref

# list your .c files here
SRC_C	= sha1.c glue.c

PRIVATE_INCDIR	= $(PKGDIR)/include

include $(L4DIR)/mk/lib.mk
