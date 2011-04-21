# generic plugin stuff

TARGET           := libmag-$(PLUGIN).a libmag-$(PLUGIN).so
LINK_INCR        := libmag-$(PLUGIN).a
LDFLAGS_libmag-$(PLUGIN).so          += -lmag-plugin.o

PC_FILENAME      := mag-$(PLUGIN)
REQUIRES_LIBS    := libstdc++

include $(L4DIR)/mk/lib.mk
