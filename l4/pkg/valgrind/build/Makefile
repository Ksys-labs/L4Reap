PKGDIR ?= ..
L4DIR ?= $(PKGDIR)/../..

include $(L4DIR)/mk/Makeconf

# path- and version-specific variables
VALGRIND_VERSION = 3.6.0-svn
VALGRIND_SRC = $(shell find $(PKGDIR)/src/ -name valgrind-$(VALGRIND_VERSION))
VALGRIND_BUILD = $(OBJ_DIR)/OBJ-$(SYSTEM)/vg

# basic BID settings
MODE_USE_C = y
L4_MULTITHREADED = y

# variables used for passing to Valgrind's build system
# gcc need a "," after defsym and a "=" after dynamic-linker
L4_LDFLAGS_WITH_LIBC=`echo "$(LDFLAGS)" | sed 's/'--dynamic-linker\ '/'-Wl,--dynamic-linker\='/g' | \
            sed 's/'--defsym\ '/'-Wl,--defsym\,'/g' | \
            sed 's/'--start-group\ '/'-Wl,--start-group\ '/g' | \
            sed 's/'--end-group\ '/'-Wl,--end-group\ '/g' | \
            sed 's$$'-T$(LDS_stat_bin)'$$''$$g'`
L4_CFLAGS  = $(CFLAGS)

L4_CRT0 = $(CRTI_stat_bin) $(CRTBEGIN_stat_bin)

SYSTEMS = x86-l4f

include $(L4DIR)/mk/prog.mk

all :: valgrind

ifneq ($(SYSTEM), )
ifeq ($(BUILD_ARCH), x86)

TMP_LDFLAGS = $(addprefix -L, $(L4LIBDIR_PROG_NOEXC-$(BID_INT_NOEXC)))
TMP_LDFLAGS += -Wl,--start-group
TMP_LDFLAGS += -Wl,--defsym,__L4_KIP_ADDR__=$(L4_KIP_ADDR) \
               -Wl,--defsym,__L4_STACK_ADDR__=$(L4_STACK_ADDR)
TMP_LDFLAGS += $(REQUIRES_LIBS_LIST)
TMP_LDFLAGS += -Wl,--end-group
TMP_LDFLAGS += --warn-common -gc-sections
L4_LDFLAGS = $(TMP_LDFLAGS)

valgrind : check_rediff valgrind_conf
	@echo -e "\033[35;1m___...---=== COMPILING ===---...___\033[0m"
	L4_CFLAGS="$(L4_CFLAGS)" \
	L4_CPPFLAGS="$(CPPFLAGS)" \
	L4_CXXFLAGS="$(CXXFLAGS)" \
	L4_CRT0="$(L4_CRT0)" \
	L4_CRT1="$(CRT1_stat_bin)" \
	L4_CRTN="$(CRTN)" \
	L4_CFLAGS="$(L4_CFLAGS)" \
	L4_CPPFLAGS="$(CPPFLAGS)" \
	L4_LDFLAGS="$(L4_LDFLAGS)" \
	L4_LDFLAGS_WITH_LIBC="$(L4_LDFLAGS_WITH_LIBC)" \
	L4_OBJ_BASE="$(OBJ_BASE)" \
	L4_LDS_stat_bin="$(LDS_stat_bin)" \
	VG_LDFLAGS="$(VG_LDFLAGS)" \
	LD="$(LINK_PROGRAM)" \
	AR="$(AR)" \
		make -C $(VALGRIND_BUILD)

valgrind_conf :
	if [ ! -e $(VALGRIND_BUILD)/config.ready ]; then \
		echo -e "\033[35;1m___...---=== CONFIG ===---...___\033[0m" && \
		echo OBJ: $(VALGRIND_BUILD) && \
		mkdir -p $(VALGRIND_BUILD) && \
		cd $(VALGRIND_BUILD) &&  \
		AR="$(AR)" CC="$(CC)" CXX="$(CXX)" STTRIP="$(STRIP)" RANLIB="$(RANLIB)"\
		$(VALGRIND_SRC)/configure \
						  --enable-maintainer-mode \
						  --host=i386-l4re \
						  --build=i386-linux; \
		touch $(VALGRIND_BUILD)/config.ready; \
	fi

# this rule checks whether the modified file copies from other l4 packages
# are still up-to-date
check_rediff:
	@cd $(PKGDIR)/.. && sha1sum -c $(SRC_DIR)/ext-checksums

.PHONY = valgrind_conf valgrind

endif # ARCH_x86
else
valgrind :
endif # SYSTEM != ""

clean cleanall::
	@echo -e "\033[35;1m___...---=== CLEAN ===---...___\033[0m"
	$(RM) -r $(OBJ_DIR)/*

