PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..
DOPEDIR          ?= $(L4DIR_ABS)/pkg/dope

TARGET            = mag
PRIVATE_INCDIR    = $(SRC_DIR)/../../include/server
SRC_CC           := big_mouse.cc main.cc screen.cc view_stack.cc \
                    user_state.cc plugin.cc input_driver.cc object_gc.cc \
                    input_source.cc session.cc core_api.cc lua_glue.swg.cc
OBJS             += mag.lua.bin.o
OBJS             += default.tff.bin.o

#MODE		 := shared

STATIC_PLUGINS += mag-input-libinput
STATIC_PLUGINS += mag-input-event
STATIC_PLUGINS += mag-client_fb
STATIC_PLUGINS += mag-mag_client

REQUIRES_LIBS:= libsupc++ libdl mag-gfx lua++ cxx_libc_io cxx_io
REQUIRES_LIBS += $(STATIC_PLUGINS)
#LDFLAGS += --export-dynamic

CPPFLAGS += -fvisibility=hidden
ifneq ($(SRC_DIR),)
all:: $(SRC_DIR)/lua_glue.swg.h

endif

include $(L4DIR)/mk/prog.mk

%.bin.o: $(SRC_DIR)/% $(SRC_DIR)/Makefile $(ELF_PATCHER)
	@$(GEN_MESSAGE)
	$(VERBOSE)(TARGETDIR=$$PWD && cd $(SRC_DIR) && $(OBJCOPY) -I binary -B $(BFD_ARCH) -O $(OFORMAT) $(notdir $<) $$TARGETDIR/$@ )
	$(VERBOSE)$(ELF_PATCHER) $@ --min-section-align=4

#symbol_name = _binary_$(subst .,_,$(subst binary_,,$(subst .o,,$@)))
#
#binary_%.o: $(SRC_DIR)/%
##	$(MSG_CONVERT)$@
#	$(VERBOSE)hexdump -ve '/1 "0x%02x,"' $< |\
#	  sed '1s/^/char $(symbol_name)_start[] __attribute__((aligned (4))) = {/;$$s/,$$/};/;$$a\' |\
#	    $(CC) -x c -o $@ -c -

%.swg:

$(SRC_DIR)/%.swg.cc: $(SRC_DIR)/%.swg.c_cc $(SRC_DIR)/Makefile
	@$(GEN_MESSAGE)
	$(VERBOSE)$(PKGDIR)/swig/rm-extern-c.pl $< >$@

$(SRC_DIR)/%.swg.h: $(SRC_DIR)/%.swg.c_h
	@$(GEN_MESSAGE)
	$(VERBOSE)$(PKGDIR)/swig/rm-extern-c.pl $< >$@

SWIG_INCDIR:=-I$(SRC_DIR) -I$(SRC_DIR)/../../include -I$(L4DIR)/pkg

$(SRC_DIR)/%.swg.c_cc: $(SRC_DIR)/%.swg $(SRC_DIR)/Makefile
	@$(GEN_MESSAGE)
	$(VERBOSE)swig $(SWIG_INCDIR) -c++ -lua -small -o $@ $<

$(SRC_DIR)/%.swg.c_h:
	pwd
	@$(GEN_MESSAGE)
	$(VERBOSE)swig $(SWIG_INCDIR) -c++ -lua -small -external-runtime $@

