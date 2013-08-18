PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../..

CON_DIR := $(PKGDIR)/../uclibc/lib/contrib/uclibc/ldso
PTH_DIR := $(PKGDIR)/../uclibc/lib/libpthread
UCC_DIR := $(PKGDIR)/../uclibc/lib/uclibc

DIR_amd64 := x86_64
DIR_x86   := i386
DIR_arm   := arm
DIR_ppc32 := powerpc
DIR_sparc := sparc


vpath %.c $(CON_DIR)/ldso
vpath %.S $(CON_DIR)/ldso/$(DIR_$(ARCH))
vpath aeabi_read_tp-v6p.S     $(UCC_DIR)/ARCH-arm
vpath aeabi_read_tp_generic.c $(UCC_DIR)/ARCH-arm

PRIVATE_INCDIR-x86 := $(CON_DIR)/../libc
PRIVATE_INCDIR := $(PRIVATE_INCDIR-$(ARCH)) \
                  $(CON_DIR)/ldso/$(DIR_$(ARCH)) \
		  $(SRC_DIR)           \
		  $(CON_DIR)/include \
		  $(PTH_DIR)/src \
		  $(PTH_DIR)/src/sysdeps/$(DIR_$(ARCH))


TARGET := libld-l4.so
SRC_C  := ldso.c fixup.c string.c
SRC_CC := syscalls.cc vfs.cc
#SRC_S_arm-l4f  += aeabi_read_tp-v6p.S
SRC_C_arm-l4f   += aeabi_read_tp_generic.c
SRC_S  := resolve.S

CPPFLAGS := -DNOT_IN_libc -DIS_IN_rtld \
            -DLDSO_ELFINTERP=\"$(DIR_$(ARCH))/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\" \
	    -D__LDSO_SEARCH_INTERP_PATH__=1 \
            -DIS_IN_rtld -D__LIBDL_SHARED__ -DSHARED

DEFINES_x86-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_amd64-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_arm-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_sparc-l4f += -DUSE_TLS=1 -DUSE___THREAD=1

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
            -fno-threadsafe-statics $(CXXFLAGS_L4_GENERIC)

CPPFLAGS := $(filter-out -DL4SYS_USE_UTCB_WRAP=1, $(CPPFLAGS))

L4_LIBS :=
