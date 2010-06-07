PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../..

CON_DIR := $(PKGDIR)/../uclibc/lib/contrib/uclibc/ldso

DIR_amd64 := x86_64
DIR_x86   := i386
DIR_arm   := arm
DIR_ppc32 := powerpc


vpath %.c $(CON_DIR)/ldso
vpath %.S $(CON_DIR)/ldso/$(DIR_$(ARCH))

PRIVATE_INCDIR-x86 := $(CON_DIR)/../libc
PRIVATE_INCDIR := $(PRIVATE_INCDIR-$(ARCH)) \
                  $(CON_DIR)/ldso/$(DIR_$(ARCH)) \
		  $(SRC_DIR)           \
		  $(CON_DIR)/include


TARGET := libld-l4.so
SRC_C  := ldso.c fixup.c
SRC_CC := syscalls.cc vfs.cc
SRC_S  := resolve.S

CPPFLAGS := -DNOT_IN_libc -DIS_IN_rtld \
            -DLDSO_ELFINTERP=\"$(DIR_$(ARCH))/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\" \
	    -D__LDSO_SEARCH_INTERP_PATH__=1 \
            -DIS_IN_rtld -D__LIBDL_SHARED__ -DSHARED

LDFLAGS  =  -Bsymbolic -Bstatic --warn-common --sort-common \
            --no-undefined -e _start -z defs -z now \
            --discard-locals --discard-all   \
            --gc-sections -ll4sys-direct $(GCCLIB)

CRT0 :=
CRTN :=


include $(L4DIR)/mk/lib.mk

ldso.1.o: ldso.o
	$(OBJCOPY) --weaken $< $@

CXXFLAGS := -DL4_NO_RTTI -fno-rtti -fno-exceptions -O99 -fvisibility=hidden \
            -fno-threadsafe-statics $(GCCNOSTACKPROTOPT)

CPPFLAGS := $(filter-out -DL4SYS_USE_UTCB_WRAP=1, $(CPPFLAGS))

L4_LIBS :=
