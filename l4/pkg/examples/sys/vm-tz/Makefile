PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= vm-tz
SYSTEMS		= arm-l4f

MODE_USE_C    = y
SRC_CC		= main.cc

LIBS		+= -lsigma0

include $(L4DIR)/mk/prog.mk
