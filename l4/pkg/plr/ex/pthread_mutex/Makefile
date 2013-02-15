PKGDIR ?=	../../..
L4DIR ?=	$(PKGDIR)/..

SYSTEMS = x86-l4f

TARGET                = pthread_mutex
SRC_C = main.c
#REQUIRES_LIBS += libpthread

include $(L4DIR)/mk/prog.mk

LDFLAGS += -lpthread_rep -luc_c -levbuf_c
