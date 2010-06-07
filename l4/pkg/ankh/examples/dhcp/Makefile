PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../..

SYSTEMS        = x86-l4f arm-l4f

REQUIRES_LIBS  = l4util l4re_c l4re_c-util shmc shmc_ringbuf ankh_netboot ankh

L4_MULTITHREADED = y

TARGET         = dhcp_clnt

PRIVATE_INCDIR += $(SRC_DIR)/../../lib/netboot

SRC_C          = main.c 

include $(L4DIR)/mk/prog.mk
