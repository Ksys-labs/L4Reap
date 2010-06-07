PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           := libmag-input-libinput.a #libmag-input-libinput.so
LINK_INCR        := libmag-input-libinput.a

SRC_CC           := input_libinput.cc
LDFLAGS_libmag-input-libinput.so += -lmag-plugin.o -Bsymbolic
CPPFLAGS         += -fvisibility=hidden
REQUIRES_LIBS    := input
PC_FILENAME      := mag-input-libinput

include $(L4DIR)/mk/lib.mk
