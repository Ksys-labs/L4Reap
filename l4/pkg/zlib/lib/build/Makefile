PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libl4z.a libl4z.so
LIBS            = $(GCCLIB)
SYSTEMS		= x86 arm amd64 ppc32
CONTRIB_INCDIR	= zlib

SRC_C		= adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c \
                  gzread.c gzwrite.c infback.c inffast.c inflate.c inftrees.c \
		  trees.c uncompr.c zutil.c
DEBUG 		= 0

vpath %.c $(SRC_DIR)/../dist

include $(L4DIR)/mk/lib.mk
