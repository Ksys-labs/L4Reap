PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../..

# the name of your library
TARGET	= $(PKGNAME)_aes_linux_586.o.a
BUILD_PIC = $(TARGET)
SYSTEMS = x86
PC_FILENAME = $(PKGNAME)_aes_linux_586

# list your .c files here
SRC_C	= aes.c
SRC_S	= aes-i586-asm.S

include $(L4DIR)/mk/lib.mk
