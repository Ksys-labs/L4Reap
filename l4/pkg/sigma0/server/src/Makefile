PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= sigma0

DEPENDS_PKGS	= cxx

MODE		= sigma0
L4_LIBS		= -static -nostdlib $(LIBCLIBS_X86) $(GCCLIB)
L4_LIBS		+= -lcxx_util -lcxx_io -lcxx_base -ll4sys
L4_LIBS		+= -lcxx_io -lcxx_io_kdebug -lcxx_base 
L4_LIBS		+= -luc_c -ll4sys-direct
DEFAULT_RELOC_x86   := 0x00100000
DEFAULT_RELOC_amd64 := 0x00100000
DEFAULT_RELOC_arm   := 0x00080000
DEFAULT_RELOC_ppc32 := 0x00100000
RELOC_PHYS          := y


PRIVATE_INCDIR	= $(SRC_DIR)

SRC_CC := init.cc memmap.cc page_alloc.cc mem_man.cc io_backend.cc \
          init_mem.cc
SRC_CC_x86-l4f   := ioports_x86.cc
SRC_CC_amd64-l4f := ioports_x86.cc
SRC_CC_arm-l4f   := ioports_none.cc
SRC_CC_ppc32-l4f := ioports_none.cc
SRC_C  := support.c
SRC_S  := $(addprefix ARCH-$(ARCH)/, crt0.S)

include $(L4DIR)/mk/prog.mk
CRT1_stat_bin   =

CXXFLAGS	+= -DL4_NO_RTTI -fno-rtti -fno-exceptions -Wall
