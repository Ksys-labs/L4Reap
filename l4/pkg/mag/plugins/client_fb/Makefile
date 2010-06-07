PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           := libmag-client_fb.a libmag-client_fb.so
LINK_INCR        := libmag-client_fb.a
SRC_CC           := client_fb.cc service.cc
LDFLAGS_libmag-client_fb.so          += -lmag-plugin.o #-Bsymbolic
#CPPFLAGS         += -fvisibility=hidden

PC_FILENAME      := mag-client_fb
REQUIRES_LIBS = libc
L4_MULTITHREADED=y

include $(L4DIR)/mk/lib.mk
