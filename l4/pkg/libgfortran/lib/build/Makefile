PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET          = libgfortran.a libgfortran.so libgfortranbegin.a
PC_FILENAMES	= libgfortran

# The -fallow-leading-underscore is only needed for some Fortran
# files, but it does not hurt for all the others.
CFLAGS         += -fcx-fortran-rules -ftree-vectorize -funroll-loops
FFLAGS         += -fno-underscoring -fimplicit-none -fno-repack-arrays \
                  -fallow-leading-underscore \
                  -ftree-vectorize -funroll-loops

include $(L4DIR)/mk/Makeconf

REQUIRES_LIBS = libc_be_sig libc_support_misc libstdc++ libc

include $(SRC_DIR)/Makefile.dist-c
include $(SRC_DIR)/Makefile.dist-f

SRC_C_libgfortran.a      = $(C_FILES)
SRC_F_libgfortran.a      = $(F_FILES)
SRC_C_libgfortran.so     = $(C_FILES)
SRC_F_libgfortran.so     = $(F_FILES)
SRC_C_libgfortranbegin.a = fmain.c

CONTRIB_DIR     = $(SRC_DIR)/../contrib
PRIVATE_INCDIR  = $(SRC_DIR) $(SRC_DIR)/include $(CONTRIB_DIR)
PRIVATE_INCDIR += $(SRC_DIR)/ARCH-$(ARCH)
PRIVATE_INCDIR += $(OBJ_BASE)/include/contrib/libstdc++-v3/bits

vpath %.c   $(CONTRIB_DIR)
vpath %.f90 $(CONTRIB_DIR)
vpath %.F90 $(CONTRIB_DIR)

include $(L4DIR)/mk/lib.mk
