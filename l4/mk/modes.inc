# -*- Makefile -*-
# vim:et:ts=8:ft=make:
#
# Makefile-Include for defining modes

# Default is uClibc without reentrant functions. If, for example
# two independant threads of the same application call malloc at the same
# time, L4_MULTITHREADED _must_ be defined to y.
L4_MULTITHREADED      ?= n
LIBC_IMPLEMENTATION-y := libpthread libc
LIBC_IMPLEMENTATION-n := libc
LIBC_IMPLEMENTATION   = $(LIBC_IMPLEMENTATION-$(L4_MULTITHREADED))
ifeq ($(LIBC_IMPLEMENTATION),)
  $(error LIBC_IMPLEMENTATION not set, L4_MULTITHREADED must be defined to 'y' or 'n'.) 
endif

ifeq ($(MODE),l4linux)
  ifeq ($(L4_MULTITHREADED),y)
    $(error MODE=l4linux and L4_MULTITHREADED=y not possible)
  endif
endif

ifeq ($(ARCH),sparc)
CRTBEGIN_so       = $(GCCLIBDIR)/crtbegin.o
CRTBEGIN_stat_bin = $(GCCLIBDIR)/crtbegin.o
else
CRTBEGIN_so       = $(GCCLIBDIR)/crtbeginS.o
CRTBEGIN_stat_bin = $(GCCLIBDIR)/crtbeginT.o
endif

CRTBEGIN_dyn_bin  = $(GCCLIBDIR)/crtbegin.o

CRTEND_so         = $(GCCLIBDIR)/crtendS.o
CRTEND_dyn_bin    = $(GCCLIBDIR)/crtend.o
CRTEND_stat_bin   = $(GCCLIBDIR)/crtend.o


CRT1_so           =
CRT1_dyn_bin      = $(call findfile,crt1.o,$(L4LIBDIR))
CRT1_stat_bin     = $(call findfile,crt1.o,$(L4LIBDIR))

CRTI_so           = $(call findfile,crti.s.o,$(L4LIBDIR))
CRTI_dyn_bin      = $(call findfile,crti.o,$(L4LIBDIR))
CRTI_stat_bin     = $(call findfile,crti.o,$(L4LIBDIR))

ifeq ($(ARCH),sparc)
CRTN_so           = $(call findfile,crtn.s.o,$(L4LIBDIR))
else
CRTN_so           = $(CRTEND_so)       $(call findfile,crtn.s.o,$(L4LIBDIR))
endif
CRTN_dyn_bin      = $(CRTEND_dyn_bin)  $(call findfile,crtn.o,$(L4LIBDIR))
CRTN_stat_bin     = $(CRTEND_stat_bin) $(call findfile,crtn.o,$(L4LIBDIR))

LDS_so            = $(call findfile,main_rel.ld,$(L4LIBDIR))
LDS_dyn_bin       = $(call findfile,main_dyn.ld,$(L4LIBDIR))
LDS_stat_bin      = $(call findfile,main_stat.ld,$(L4LIBDIR))


LIBC_INCLUDE   = -nostdinc                                  \
                 -I$(OBJ_BASE)/include/$(ARCH)/uclibc       \
                 -I$(OBJ_BASE)/include/uclibc               \
                 -I$(OBJ_BASE)/include/contrib/libstdc++-v3        \
                 $(if $(CONFIG_USE_DROPS_STDDIR),           \
		  -I$(DROPS_STDDIR)/include/$(ARCH)/uclibc  \
                  -I$(DROPS_STDDIR)/include/uclibc          \
                  -I$(DROPS_STDDIR)/include/contrib/libstdc++-v3,)      \
                 $(I_GCCINCDIR)

#############################################################################

################################
# sigma0 mode
################################

BID_SUPPORTED_all_sigma0        = y
LIBCINCDIR_all_sigma0           = $(LIBC_INCLUDE)
REQUIRES_LIBS_all_sigma0        = l4util l4util l4sys                   \
                                  $(LIBC_IMPLEMENTATION)                \
                                  $(if $(MODE_USE_C),l4re_c-util)
L4_LIBS_all_sigma0		= -static -nostdlib                     \
                                  $(GCCLIB) $(GCCLIB_EH) -lcrt0         \
				  $(EXTRA_LIBS)
CRT0_all_sigma0                 = $(CRTI_stat_bin) $(CRTBEGIN_stat_bin) $(CRT1_stat_bin)
CRTN_all_sigma0                 = $(CRTN_stat_bin)
LDSCRIPT_all_sigma0             = $(LDS_stat_bin)
LDFLAGS_all_sigma0              = -Bstatic


################################
# static mode
################################

BID_SUPPORTED_all_static        = y
LIBCINCDIR_all_static           = $(LIBC_INCLUDE)
REQUIRES_LIBS_all_static        = l4re-main l4re-util stdlibs      \
                                  $(LIBC_IMPLEMENTATION)           \
                                  $(if $(MODE_USE_C),l4re_c-util)
L4_LIBS_all_static		= -static -nostdlib  $(GCCLIB) $(GCCLIB_EH) \
                                  $(EXTRA_LIBS)
CRT0_all_static                 = $(CRTI_stat_bin) $(CRTBEGIN_stat_bin) $(CRT1_stat_bin)
CRTN_all_static                 = $(CRTN_stat_bin)
LDSCRIPT_all_static             = $(LDS_stat_bin)
LDFLAGS_all_static              = -Bstatic

#####################################
# shared mode -- programs are linked against shared libs
#####################################
BID_SUPPORTED_all_shared        = y
LIBCINCDIR_all_shared           = $(LIBC_INCLUDE)
CRT0_all_shared                 = $(CRTI_dyn_bin) $(CRTBEGIN_dyn_bin) $(CRT1_dyn_bin)
CRTN_all_shared                 = $(CRTN_dyn_bin)
LDSCRIPT_all_shared             = $(LDS_dyn_bin)
REQUIRES_LIBS_all_shared        = l4re-main l4re-util ldso stdlibs-sh  \
                                  $(LIBC_IMPLEMENTATION)  \
                                  $(if $(MODE_USE_C),l4re_c-util)
L4_LIBS_all_shared             += -nostdlib $(GCCLIB) $(GCCLIB_EH)    \
				  $(EXTRA_LIBS)
LDFLAGS_all_shared              = --eh-frame-hdr

################################
# l4linux mode
################################

BID_SUPPORTED_all_l4linux       = y
LIBCINCDIR_all_l4linux          =
REQUIRES_LIBS_all_l4linux       = 
L4_LIBS_all_l4linux		= -l4re_main                        \
				  $(L4_LIBS_all_l4linux_USE_C-y)    \
                                  -l4re-util                        \
                                  -l4lx-re.o                        \
				  -ll4sys-l4x -l4re                 \
				  $(EXTRA_LIBS)                     \
                                  -ll4util                          \
				  -lcxx_base -lcxx_io               \
				  -lcxx_libc_io
L4_LIBS_all_l4linux_USE_C-$(MODE_USE_C) = -l4re-c -l4re-c-util
CRT0_all_l4linux                 =
CRTN_all_l4linux                 =
LDSCRIPT_all_l4linux             =
LDFLAGS_all_l4linux              =


#####################################
# lib mode -- options when building libs
#####################################

BID_SUPPORTED_all_lib = y
LIBCINCDIR_all_lib    = $(LIBC_INCLUDE)
CRT0_all_lib          = $(CRTI_dyn_bin) $(CRTBEGIN_dyn_bin) $(CRT1_dyn_bin)
CRTN_all_lib          = $(CRTN_dyn_bin)
LDSCRIPT_all_lib      = $(LDS_so)
LDFLAGS_all_lib       = --eh-frame-hdr


#####################################
# host mode -- no L4 specifics
#####################################

BID_SUPPORTED_all_host = y
LIBCINCDIR_all_host    =
REQUIRES_LIBS_all_host =
L4_LIBS_all_host       =
CRT0_all_host          =
CRTN_all_host          =
LDSCRIPT_all_host      =
LDFLAGS_all_host       =
