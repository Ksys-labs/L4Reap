PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           := libmag-input-lxdd.a #libmag-input-lxdd.so
LINK_INCR        := libmag-input-lxdd.a
SRC_CC           := input_lxdd.cc
LDFLAGS_libmag-input-lxdd.so := -lmag-plugin.o -Bsymbolic
CPPFLAGS         += -fvisibility=hidden
PC_FILENAME      := mag-input-lxdd

include $(L4DIR)/mk/lib.mk
