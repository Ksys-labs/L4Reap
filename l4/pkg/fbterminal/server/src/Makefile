PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           = fbterminal
SRC_CC           = main.cc
REQUIRES_LIBS    = libgfxbitmap lib_vt100 libc_support_misc libc_be_fs_noop \
                   libgfxbitmap-support libevent
L4_MULTITHREADED = y

include $(L4DIR)/mk/prog.mk
