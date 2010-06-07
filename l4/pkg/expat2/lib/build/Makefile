PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TARGET  = libexpat2.a libexpat2.so
CONTRIB_INCDIR = expat2


vpath %.c $(PKGDIR)/lib/contrib/lib

DEFINES = -DHAVE_EXPAT_CONFIG_H

PRIVATE_INCDIR = $(SRC_DIR)

SRC_C   = xmlparse.c xmltok.c xmlrole.c

include $(L4DIR)/mk/lib.mk

CFLAGS := $(filter-out -Wextra,$(CFLAGS))
