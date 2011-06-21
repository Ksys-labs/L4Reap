PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../..

TARGET         = libc_be_socket_lwip.a libc_be_socket_lwip.so
SYSTEMS        = x86-l4f arm-l4f
PC_FILENAME    = libc_be_socket_lwip
SRC_C          = be.c 
SRC_CC         = sock_fs.cc
REQUIRES_LIBS  = ankh_lwip
PRIVATE_INCDIR += $(PKGDIR)/lib/lwip/contrib/src/include \
		  $(PKGDIR)/lib/lwip/contrib/src/include/ipv4 \
		  $(PKGDIR)/lib/lwip


include $(L4DIR)/mk/lib.mk
