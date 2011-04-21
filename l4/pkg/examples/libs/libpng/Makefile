PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_libpng
SRC_C		= main.c
L4_MULTITHREADED = y
DEPENDS_PKGS    = libpng zlib
REQUIRES_LIBS   = libl4png_wrap l4re_c-util

include $(L4DIR)/mk/prog.mk
