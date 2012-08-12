PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..


INSTALL_TARGET = liblua.a
PC_LIBS       := -llua
CONTRIB_INCDIR = lua
REQUIRES_LIBS  = l4re_c-util
OPTS :=

all::

include $(L4DIR)/mk/lib.mk

DEFS = -DARCH_$(ARCH) $(OPTS)

liblua.a: $(SRC_DIR)/Makefile $(SRC_DIR)/Makefile.lua FORCE
	$(MAKE) -f $(SRC_DIR)/Makefile.lua SRC_DIR=$(SRC_DIR) \
	  L4_DEFINES="$(DEFS)" \
	  L4_INCLUDES="$(LIBCINCDIR) $(addprefix -I, $(L4INCDIR))" \
	  PLAT=l4re CFLAGS_L4_GENERIC="$(CFLAGS_L4_GENERIC)" \
	  CC="$(CC)" AR="$(AR) rcu" RANLIB="$(RANLIB)" liblua.a

clean::
	$(MAKE) -f $(SRC_DIR)/Makefile.lua SRC_DIR=$(SRC_DIR) clean


all:: $(PC_FILES)
