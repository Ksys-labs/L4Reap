PKGDIR	= ../..
L4DIR	?= $(PKGDIR)/../..

MODE           = host
INSTALL_TARGET = luac lua

PLAT-linux     = linux
PLAT-darwin    = macosx
PLAT-freebsd   = freebsd
PLAT           = $(if $(PLAT-$(HOST_SYSTEM)),$(PLAT-$(HOST_SYSTEM)),generic)

include $(L4DIR)/mk/prog.mk

lua luac:
	$(MAKE) -f "$(SRC_DIR)/Makefile.lua" PLAT=$(PLAT) \
	        CC="$(HOST_CC)" SRC_DIR="$(SRC_DIR)" $@

luac: lua
.PHONY: lua luac

clean::
	$(MAKE) -f "$(SRC_DIR)/Makefile.lua" PLAT=$(PLAT) \
	        CC="$(HOST_CC)" SRC_DIR="$(SRC_DIR)" clean
