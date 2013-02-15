PKGDIR ?=	../../..
L4DIR ?=	$(PKGDIR)/..

TARGET                = pthread_hello
SRC_C = main.c

SYSTEMS = x86-l4f

include $(L4DIR)/mk/prog.mk

LDFLAGS+=-lpthread_rep -luc_c -levbuf_c
