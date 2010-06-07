PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..

TARGET           := libmag-mag_client.a libmag-mag_client.so
LINK_INCR        := libmag-mag_client.a
SRC_CC           := mag_client.cc
LDFLAGS_libmag-mag_client.so          += -lmag-plugin.o

PC_FILENAME      := mag-mag_client
REQUIRES_LIBS    := libstdc++

include $(L4DIR)/mk/lib.mk
