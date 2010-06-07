PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..

TARGET       = libc_be_rtc.a libc_be_rtc.so
LINK_INCR    = libc_be_rtc.a
SRC_C        = gettime.c l4libc_init_time.c
PC_FILENAME  = rtc_libc_be
REQUIRES_LIBS= rtc

include $(L4DIR)/mk/lib.mk
