PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../..

SYSTEMS        = x86-l4f arm-l4f

REQUIRES_LIBS  = l4util l4re_c l4re_c-util shmc shmc_ringbuf \
				 ankh_netboot ankh ankh_lwip libc_be_socket_lwip

PRIVATE_INCDIR += $(PKGDIR)/lib/lwip/contrib/src/include \
				  $(PKGDIR)/lib/lwip/contrib/src/include/ipv4 \
				  $(PKGDIR)/lib/lwip

L4_MULTITHREADED = y

TARGET         = lwip_test

SRC_C          = main.c 

include $(L4DIR)/mk/prog.mk

