PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_libpng
SRC_C		= main.c
MODE_USE_C      = y
L4_MULTITHREADED = y
DEPENDS_PKGS    = libpng zlib
REQUIRES_LIBS   = libl4png_wrap

include $(L4DIR)/mk/prog.mk
