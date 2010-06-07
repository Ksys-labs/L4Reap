PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../..

TARGET       = r/libstdc++.a r/libstdc++.so
PC_FILENAME  =
REQUIRES_LIBS= libsupc++

STDCXX_PKG_DIR = $(PKGDIR_ABS)/../libstdc++-v3
L4_MULTITHREADED := y

include $(PKGDIR)/../libstdc++-v3/build/src/Makefile

$(GENERAL_D_LOC): $(PKGDIR)/../libstdc++-v3/build/src/Makefile
