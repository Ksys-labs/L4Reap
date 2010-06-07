PKGDIR ?=	../..
L4DIR ?=	$(PKGDIR)/../..

TARGET           = ex_libio
SRC_C            = main.c
MODE_USE_C       = y
L4_MULTITHREADED = y
DEPENDS_PKGS     = libio
REQUIRES_LIBS    = libio libirq

include $(L4DIR)/mk/prog.mk
