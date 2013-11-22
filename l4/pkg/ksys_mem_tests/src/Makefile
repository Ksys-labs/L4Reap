PKGDIR ?= ../
L4DIR ?= $(PKGDIR)/../../

PKG_FILES = \
	map_rwx

TARGET=$(patsubst %,ksys_mem_%,$(PKG_FILES))

MODE = shared
WARNINGS = -Wno-unused
LDFLAGS = --no-undefined
DEFINES = -DRUNDIR=""

CCXX_FLAGS_arm += -DUSE_ARM=1

$(foreach target,$(PKG_FILES),$(eval SRC_CC_ksys_mem_$(target)=$(target).cc))

vpath %.c $(PKGDIR)/src/
include $(L4DIR)/mk/prog.mk
