PKGDIR          ?= ..
L4DIR           ?= $(PKGDIR)/../..

include $(L4DIR)/mk/Makeconf

INCSRC_DIR      := $(OBJ_DIR)
LIBPNG          := $(PKGDIR_ABS)/lib/dist
EXPORTED        := png.h pngconf.h
EXPORTED_O      := $(addprefix $(OBJ_DIR)/, $(EXPORTED))
LOCAL           := $(wildcard *.h)
LOCAL_O         := $(addprefix $(OBJ_DIR)/, $(LOCAL))
TARGET          := $(EXPORTED) $(LOCAL)

all:: $(EXPORTED_O) $(LOCAL_O)

$(EXPORTED_O) : $(OBJ_DIR)/% : $(LIBPNG)/%
	@$(INSTALL_LOCAL_MESSAGE)
	$(VERBOSE)$(LN) -fs $< $@

$(LOCAL_O): $(OBJ_DIR)/% : $(SRC_DIR)/%
	@$(INSTALL_LOCAL_MESSAGE)
	$(VERBOSE)$(LN) -fs $< $@
	

cleanall::
	$(VERBOSE)rm -f $(EXPORTED) 

include $(L4DIR)/mk/include.mk
