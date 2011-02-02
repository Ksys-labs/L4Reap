PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           := libmag-input-event.a #libmag-input-event.so
LINK_INCR        := libmag-input-event.a
SRC_CC           := input_event.cc
LDFLAGS_libmag-input-event.so := -lmag-plugin.o -Bsymbolic
CPPFLAGS         += -fvisibility=hidden
PC_FILENAME      := mag-input-event

include $(L4DIR)/mk/lib.mk
