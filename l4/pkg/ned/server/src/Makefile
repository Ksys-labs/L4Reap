PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		 = ned

SRC_CC          := remote_mem.cc app_model.cc app_task.cc main.cc \
                   lua.cc lua_env.cc lua_ns.cc lua_cap.cc \
	           lua_exec.cc lua_factory.cc lua_info.cc server.cc
OBJS            += ned.lua.bin.o

REQUIRES_LIBS   := libloader l4re-util l4re lua++ libpthread cxx_libc_io cxx_io

ifeq (1,0)
DEFINES         += -DUSE_READLINE
REQUIRES_LIBS   += readline libc_be_file_stdin
endif

include $(L4DIR)/mk/prog.mk

%.bin.o: $(SRC_DIR)/% $(SRC_DIR)/Makefile $(ELF_PATCHER)
	@$(GEN_MESSAGE)
	$(VERBOSE)(TARGETDIR=$$PWD && cd $(SRC_DIR) && $(OBJCOPY) -I binary -B $(BFD_ARCH) -O $(OFORMAT) $(notdir $<) $$TARGETDIR/$@ )
	$(VERBOSE)$(ELF_PATCHER) $@ --min-section-align=4
