PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_sha1_openssl.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86
PC_FILENAME = $(PKGNAME)_sha1_openssl

# some #defines
DEFINES	+= -DBUILD_FOR_L4_LIBCRYPTO=1

# list your .c files here
SRC_C	= glue.c sha1dgst.c
SRC_S	= sx86-elf.S

PRIVATE_INCDIR	= $(PKGDIR)/include

include $(L4DIR)/mk/lib.mk
