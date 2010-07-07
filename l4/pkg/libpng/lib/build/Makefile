PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET          = libl4png.a libl4png.so
LIBS            = $(GCCLIB)
SYSTEMS		= x86 amd64 arm ppc32
REQUIRES_LIBS   = zlib
CONTRIB_INCDIR  = libpng

SRC_C           = pngerror.c pngget.c pngpread.c pngrio.c pngrutil.c \
		  pngwrite.c pngwutil.c png.c pngmem.c pngread.c \
		  pngrtran.c pngset.c pngtrans.c pngwio.c pngwtran.c

DEFINE		= PNG_USER_MEM_SUPPORTED

vpath %.c $(SRC_DIR)/../dist

include $(L4DIR)/mk/lib.mk
