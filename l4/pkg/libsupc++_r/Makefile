PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../..

TARGET      = r/libsupc++.a r/libsupc++.so
PC_FILENAME =

STDCXX_PKG_DIR =  $(PKGDIR_ABS)/../libstdc++-v3
SUPCXX_PKG_DIR =  $(PKGDIR_ABS)/../libsupc++

L4_MULTITHREADED := y
REQUIRES_LIBS := libc

include $(PKGDIR)/../libsupc++/build/Makefile

vpath %.cc $(SUPCXX_PKG_DIR)/build
