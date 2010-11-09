PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET           = lwip_test
SRC_C            = main.c 
SYSTEMS          = x86-l4f arm-l4f
REQUIRES_LIBS    = ankh_lwip libc_be_socket_lwip
L4_MULTITHREADED = y

include $(L4DIR)/mk/prog.mk

