PKGDIR	?= ..
L4DIR	?= $(PKGDIR)/../..

TARGET = client-c analyzer netboot lwip libc_be_socket

include $(L4DIR)/mk/subdir.mk

lwip: client-c
libc_be_socket: lwip
