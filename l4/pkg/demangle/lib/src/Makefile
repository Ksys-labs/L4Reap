PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libdemangle.a
SYSTEMS		= x86 arm amd64
BUILD_PIC	= libdemangle.a

SRC_C		= cplus-dem.c cp-demangle.c safe-ctype.c \
		  xmalloc.c xstrdup.c xexit.c memcmp.c
DEFINES		= -DHAVE_STDLIB_H -DHAVE_STRING_H \
		  -Dmalloc=demangle_malloc \
		  -Drealloc=demangle_realloc \
		  -Dcalloc=demangle_calloc \
		  -Dfree=demangle_free

include $(L4DIR)/mk/lib.mk

CFLAGS		+= -fomit-frame-pointer -Os
