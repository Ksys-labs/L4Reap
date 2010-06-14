# -*- Makefile -*-
#
# DROPS (Dresden Realtime OPerating System) Component
#
# Makefile-Template for library directories
#
# install.inc is used, see there for further documentation
# binary.inc is used, see there for further documentation


ifeq ($(origin _L4DIR_MK_LIB_MK),undefined)
_L4DIR_MK_LIB_MK=y

ROLE = lib.mk

# define INSTALLDIRs prior to including install.inc, where the install-
# rules are defined. Same for INSTALLDIR.
ifeq ($(MODE),host)
INSTALLDIR_LIB		?= $(DROPS_STDDIR)/lib/host
INSTALLDIR_LIB_LOCAL	?= $(OBJ_BASE)/lib/host
else
INSTALLDIR_LIB		?= $(DROPS_STDDIR)/lib/$(subst -,/,$(SYSTEM))
INSTALLDIR_LIB_LOCAL	?= $(OBJ_BASE)/lib/$(subst -,/,$(SYSTEM))
endif
INSTALLFILE_LIB		?= $(INSTALL) -m 644 $(1) $(2)
INSTALLFILE_LIB_LOCAL	?= $(LN) -sf $(call absfilename,$(1)) $(2)

INSTALLFILE		= $(INSTALLFILE_LIB)
INSTALLDIR		= $(INSTALLDIR_LIB)
INSTALLFILE_LOCAL	= $(INSTALLFILE_LIB_LOCAL)
INSTALLDIR_LOCAL	= $(INSTALLDIR_LIB_LOCAL)

# our mode
MODE			?= lib

# sanity check for proper mode
ifneq ($(filter-out lib host,$(MODE)),)
$(error MODE=$(MODE) not possible when building libraries)
endif

# all libraries are built using the wraped utcb-getter
CPPFLAGS          += -DL4SYS_USE_UTCB_WRAP=1

# include all Makeconf.locals, define common rules/variables
include $(L4DIR)/mk/Makeconf
include $(L4DIR)/mk/binary.inc
$(GENERAL_D_LOC): $(L4DIR)/mk/lib.mk

ifneq ($(SYSTEM),) # if we a system, really build

TARGET_LIB        := $(TARGET) $(TARGET_$(OSYSTEM))
TARGET_SHARED     := $(filter     %.so,$(TARGET_LIB))
TARGET_STANDARD   := $(filter-out %.so,$(TARGET_LIB))

TARGET_LIB_NE     := $(TARGET_NE) $(TARGET_NE_$(OSYSTEM))
TARGET_SHARED_NE  := $(addprefix noexc/,$(filter     %.so,$(TARGET_LIB_NE)))
TARGET_STANDARD_NE:= $(addprefix noexc/,$(filter-out %.so,$(TARGET_LIB_NE)))

TARGET_PROFILE  := $(patsubst %.a,%.pr.a,\
			$(filter $(BUILD_PROFILE),$(TARGET_STANDARD)))
TARGET_PROFILE_SHARED := $(filter %.so,$(TARGET_PROFILE))
TARGET_PIC      := $(patsubst %.a,%.p.a,\
			$(filter $(BUILD_PIC),$(TARGET_STANDARD)))
TARGET_PIC_NE   := $(addprefix noexc/,$(TARGET_PIC))
TARGET_PROFILE_PIC := $(patsubst %.a,%.p.a,\
			$(filter $(BUILD_PIC),$(TARGET_PROFILE)))
TARGET	+= $(TARGET_$(OSYSTEM)) $(TARGET_PIC)
TARGET  += $(TARGET_PIC_NE) $(TARGET_SHARED_NE) $(TARGET_STANDARD_NE)
TARGET	+= $(TARGET_PROFILE) $(TARGET_PROFILE_SHARED) $(TARGET_PROFILE_PIC)

# define some variables different for lib.mk and prog.mk
LDFLAGS += $(addprefix -L, $(PRIVATE_LIBDIR) $(PRIVATE_LIBDIR_$(OSYSTEM)) $(PRIVATE_LIBDIR_$@) $(PRIVATE_LIBDIR_$@_$(OSYSTEM)))
LDFLAGS += $(addprefix -L, $(if $(LINK_WITH_NOEXC_LIBS_$@),$(L4LIBDIR_NOEXC),$(L4LIBDIR_R-$(L4_MULTITHREADED))))
LDFLAGS += $(LIBCLIBDIR)
LDFLAGS_SO ?= -shared -nostdlib

LDFLAGS += --defsym __L4_KIP_ADDR__=$(L4_KIP_ADDR) \
           --defsym __L4_STACK_ADDR__=$(L4_STACK_ADDR)

BID_LDFLAGS_FOR_LINKING_DYN_LD  = $(LDFLAGS)
BID_LDFLAGS_FOR_GCC_DYN         = $(filter     -static -shared -nostdlib -Wl$(BID_COMMA)% -L% -l%,$(LDFLAGS))
BID_LDFLAGS_FOR_LD_DYN          = $(filter-out -static -shared -nostdlib -Wl$(BID_COMMA)% -L% -l%,$(LDFLAGS))
BID_LDFLAGS_FOR_LINKING_DYN_GCC = $(addprefix -Wl$(BID_COMMA),$(BID_LDFLAGS_FOR_LD_DYN)) $(BID_LDFLAGS_FOR_GCC_DYN)


LDSCRIPT       = $(LDS_so)
LDSCRIPT_INCR ?= /dev/null
CRT0           = $(CRTI_so) $(CRTBEGIN_so) $(CRT1_so)
CRTN           = $(CRTN_so)

# install.inc eventually defines rules for every target
include $(L4DIR)/mk/install.inc

ifeq ($(NOTARGETSTOINSTALL),)
PC_LIBS     ?= $(patsubst lib%.so,-l%,$(TARGET_SHARED) \
               $(patsubst lib%.a,-l%,$(TARGET_STANDARD)))

PC_FILENAME  ?= $(PKGNAME)
PC_FILENAMES ?= $(PC_FILENAME)

PC_FILES     := $(foreach pcfile,$(PC_FILENAMES),$(OBJ_BASE)/pc/$(pcfile).pc)

# 1: basename
# 2: pcfilename
get_cont = $(if $($(1)_$(2)),$($(1)_$(2)),$($(1)))

# 1: lib list
convert_lib_list =                                  \
	$(patsubst lib%.a,-l%,$(filter %.a,$(1)))   \
	$(patsubst lib%.so,-l%,$(filter %.so,$(1))) \
	$(filter -l%,$(1))

# 1: name
# 2: output file
# 3: inc path (one only)
# 4: libs
# 5: requires_libs
generate_pcfile =                                                            \
	mkdir -p $(dir $(2))                                                 \
	;echo -n                                                    > $(2)   \
	$(if $(3),;echo "incdir=/empty_incdir"                     >> $(2))  \
	;echo "Name: $(1)"                                         >> $(2)   \
	;echo "Version: 0"                                         >> $(2)   \
	;echo "Description: L4 library"                            >> $(2)   \
	$(if $(3),;echo "Cflags: $(addprefix -I\$${incdir}/,$(3))" >> $(2))  \
	$(if $(4),;echo "Libs: $(sort $(4))"                       >> $(2))  \
	$(if $(5),;echo "Requires: $(5)"                           >> $(2))  \
	$(if $(BID_GEN_CONTROL),;echo "Provides: $(1)"             >> $(PKGDIR)/Control) \
	$(if $(BID_GEN_CONTROL),;echo "Requires: $(5)"             >> $(PKGDIR)/Control) ;

$(OBJ_BASE)/pc/%.pc: $(GENERAL_D_LOC)
	$(VERBOSE)$(call generate_pcfile,$*,$@,$(call get_cont,CONTRIB_INCDIR,$*),$(call convert_lib_list,$(call get_cont,PC_LIBS,$*)),$(call get_cont,REQUIRES_LIBS,$*))

all:: $(PC_FILES)

endif

DEPS	+= $(foreach file,$(TARGET), $(dir $(file)).$(notdir $(file)).d)

$(filter-out $(LINK_INCR) %.so %.o.a %.o.pr.a, $(TARGET)):%.a: $(OBJS)
	@$(AR_MESSAGE)
	$(VERBOSE)$(MKDIR) $(basename $@)
	$(VERBOSE)$(RM) $@
	$(VERBOSE)$(AR) crs $@ $(OBJS)
	@$(BUILT_MESSAGE)

# shared lib
$(filter %.so, $(TARGET)):%.so: $(OBJS) $(CRTN) $(CRT0) $(CRTP) $(LIBDEPS)
	@$(LINK_SHARED_MESSAGE)
	$(VERBOSE)$(MKDIR) $(basename $@)
	$(VERBOSE)$(call MAKEDEP,$(LD)) $(LD) -m $(LD_EMULATION) \
	   -o $@ $(LDFLAGS_SO) $(addprefix -T,$(LDSCRIPT)) $(CRTP) \
	   $(OBJS) $(REQUIRES_LIBS_LIST) $(LDFLAGS) \
	   $(GCCLIB) $(GCCLIB_EH) $(CRTN)
	@$(BUILT_MESSAGE)

# build an object file (which looks like a lib to a later link-call), which
# is either later included as a whole or not at all (important for static
# constructors)
$(filter $(LINK_INCR) %.o.a %.o.pr.a, $(TARGET)):%.a: $(OBJS) $(LIBDEPS)
	@$(LINK_PARTIAL_MESSAGE)
	$(VERBOSE)$(MKDIR) $(basename $@)
	$(VERBOSE)$(call MAKEDEP,$(LD)) $(LD) -m $(LD_EMULATION) \
	   -T$(LDSCRIPT_INCR) \
	   -o $@ -r $(OBJS) $(LDFLAGS)
	$(if $(LINK_INCR_ONLYGLOBSYM_$@)$(LINK_INCR_ONLYGLOBSYMFILE_$@), \
	   $(VERBOSE)$(OBJCOPY) \
	   $(foreach f,$(LINK_INCR_ONLYGLOBSYMFILE_$@),--keep-global-symbols=$(f)) \
	   $(foreach f,$(LINK_INCR_ONLYGLOBSYM_$@),-G $(f)) \
	   $@)
	@$(BUILT_MESSAGE)

endif	# architecture is defined, really build

.PHONY: all clean cleanall config help install oldconfig txtconfig
-include $(DEPSVAR)
help::
	@echo "  all            - compile and install the libraries locally"
ifneq ($(SYSTEM),)
	@echo "                   to $(INSTALLDIR_LOCAL)"
endif
	@echo "  install        - compile and install the libraries globally"
ifneq ($(SYSTEM),)
	@echo "                   to $(INSTALLDIR)"
endif
	@echo "  scrub          - delete backup and temporary files"
	@echo "  clean          - delete generated object files"
	@echo "  cleanall       - delete all generated, backup and temporary files"
	@echo "  help           - this help"
	@echo
ifneq ($(SYSTEM),)
	@echo "  libraries are: $(TARGET)"
else
	@echo "  build for architectures: $(TARGET_SYSTEMS)"
endif

endif	# _L4DIR_MK_LIB_MK undefined