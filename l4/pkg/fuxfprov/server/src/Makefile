PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= fuxfprov
SYSTEMS		= x86-l4f
SRC_CC		= main.cc
DEPENDS_PKGS	= zlib l4util
REQUIRES_LIBS   = zlib l4util libsupc++ libl4revfs-fs-fuxfs

include $(L4DIR)/mk/prog.mk

