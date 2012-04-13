PKGDIR		?= ../../
L4DIR		?= $(PKGDIR)/../..

TARGET		= libsigc++.a libsigc++.so
REQUIRES_LIBS	= libstdc++

PRIVATE_INCDIR   = $(PKGDIR)/lib/contrib/ $(PKGDIR)/include $(SRC_DIR) 

DEFINES		+= -DHAVE_CONFIG_H

vpath %.cc $(SRC_DIR)/../contrib/sigc++

LDFLAGS		=  --warn-common --no-undefined

SRC_CC = signal.cc signal_base.cc trackable.cc connection.cc \
	functors/slot.cc functors/slot_base.cc \
	adaptors/lambda/lambda.cc 

include $(L4DIR)/mk/lib.mk
