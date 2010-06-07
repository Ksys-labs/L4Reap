PKGDIR           ?= ../..
L4DIR            ?= $(PKGDIR)/../..
DOPEDIR          ?= $(L4DIR_ABS)/pkg/dope

TARGET            = mag
PRIVATE_INCDIR    = $(SRC_DIR)/../../include/server
SRC_CC           := big_mouse.cc main.cc screen.cc view_stack.cc \
                    user_state.cc plugin.cc input_driver.cc object_gc.cc
#OBJS             += binary_default.tff.o

#MODE		 := shared

STATIC_PLUGINS := mag-input-lxdd mag-input-libinput mag-client_fb
STATIC_PLUGINS += mag-mag_client

REQUIRES_LIBS:= libsupc++ libdl mag-gfx
REQUIRES_LIBS += $(STATIC_PLUGINS)
#LDFLAGS += --export-dynamic

CPPFLAGS += -fvisibility=hidden

L4_MULTITHREADED = y

vpath %.ttf $(SRC_DIR)

include $(L4DIR)/mk/prog.mk

symbol_name = _binary_$(subst .,_,$(subst binary_,,$(subst .o,,$@)))

binary_%.o: $(SRC_DIR)/%
#	$(MSG_CONVERT)$@
	$(VERBOSE)hexdump -ve '/1 "0x%02x,"' $< |\
	  sed '1s/^/char $(symbol_name)_start[] __attribute__((aligned (4))) = {/;$$s/,$$/};/;$$a\' |\
	    $(CC) -x c -o $@ -c -

