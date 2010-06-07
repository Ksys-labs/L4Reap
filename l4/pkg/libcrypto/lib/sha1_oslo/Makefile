PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_sha1_oslo.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86
PC_FILENAME = $(PKGNAME)_sha1_oslo

# list your .c files here
SRC_C	= sha.c glue.c

PRIVATE_INCDIR	= $(PKGDIR)/include

include $(L4DIR)/mk/lib.mk
