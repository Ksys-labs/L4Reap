all:
SHELL=/bin/bash
MAKE_REQUIRED_VERSION := 381# MAKE_VERSION stripped of any dots
VERSION_CHECK := \
	$(shell \
	test $$(echo $(MAKE_VERSION) | sed -e 's/\.//g') -ge \
	"$(MAKE_REQUIRED_VERSION)" 2>/dev/null \
	&& echo 1 || echo 0)

ifneq (1,$(VERSION_CHECK))
$(error Your version of Make ($(MAKE_VERSION)) is too old to use this code!)
endif

ShakeNMake.MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
$(ShakeNMake.MAKEFILE):# avoid breaking some deps checks if someone renames this file (been there, done that)

PACKAGE.MAKEFILE := $(firstword $(MAKEFILE_LIST))# normally either Makefile or GNUmakefile
$(PACKAGE.MAKEFILE):

########################################################################
# Maintainer's/hacker's notes:
#
# Vars names starting with ShakeNMake are mostly internal to this
# makefile and are considered "private" unless documented otherwise.
# Notable exceptions are most of the ShakeNMake.CALL entries, which
# are $(call)able functions, and ShakeNMake.EVAL entries, which are
# $(eval)able code.
#
########################################################################


########################################################################
# ShakeNMake.CALL.FIND_BIN call()able function:
# $1 = app name
# $2 = optional path
ShakeNMake.CALL.FIND_BIN = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(2) $(PATH)))))
########################################################################
ShakeNMake.BINS.AR := $(call ShakeNMake.CALL.FIND_BIN,$(AR))
ShakeNMake.BINS.GCC := $(call ShakeNMake.CALL.FIND_BIN,gcc)


ifneq (,$(COMSPEC))
$(warning Setting ShakeNMake.SMELLS.LIKE.WINDOWS to 1)
ShakeNMake.SMELLS.LIKE.WINDOWS := 1
ShakeNMake.EXTENSIONS.DLL = .DLL# maintenance reminder: this must stay upper-case!
ShakeNMake.EXTENSIONS.EXE = .EXE# maintenance reminder: this must stay upper-case!
else
ShakeNMake.SMELLS.LIKE.WINDOWS := 0
ShakeNMake.EXTENSIONS.DLL = .so
ShakeNMake.EXTENSIONS.EXE =# no whitespace, please
endif

########################################################################
# Cleanup rules.
# To ensure proper cleanup order, define deps on clean-., like so:
# clean-.: clean-mysubdir
# clean-mysubdir: clean-prereqsubdir
#...
.PHONY: clean distclean clean-. distclean-.
clean-%:
	$(MAKE) -C $* clean
clean-.:
	@-echo "Cleaning up $(TOP_SRCDIR)..."; \
	touch $@.$$$$; rm -f $@.$$$$ $(CLEAN_FILES) 

clean: clean-.

distclean-%:
	$(MAKE) -C $* distclean
distclean-.:
	@-echo "Cleaning up $(TOP_SRCDIR)..."; \
	touch $@.$$$$; rm -f $@.$$$$ $(CLEAN_FILES) $(DISTCLEAN_FILES) 
distclean: distclean-.


########################################################################
# An internal hack to enable "quiet" output. $(1) is a string which
# is shown ONLY if ShakeNMake.QUIET!=1
ShakeNMake.QUIET ?= 0
define ShakeNMake.CALL.SETX
if test x1 = "x$(ShakeNMake.QUIET)"; then echo $(1); else set -x; fi
endef
########################################################################

########################################################################
# Building a static library:
#
#   myLib.LIB.OBJECTS = foo.o bar.o
#   $(call ShakeNMake.CALL.RULES.LIBS,myLib)
#
# A target named myLib.LIB is created for building the library and
# $(myLib.LIB) holds the name of the compiled lib (typically
# "myLib.a").
#
########################################################################
# ShakeNMake.EVAL.RULES.LIB creates rules to build static library
# $(1).a
define ShakeNMake.EVAL.RULES.LIB
$(1).LIB = $(1).a
$(1).LIB: $$($(1).LIB)
CLEAN_FILES += $$($(1).LIB)
$$($(1).LIB): $$($(1).LIB.OBJECTS)
	@$(call ShakeNMake.CALL.SETX,"AR [$$@] ..."); \
		$$(ShakeNMake.BINS.AR) crs $$@ $$($(1).LIB.OBJECTS)
endef
define ShakeNMake.CALL.RULES.LIBS
$(foreach liba,$(1),$(eval $(call ShakeNMake.EVAL.RULES.LIB,$(liba))))
endef
# end ShakeNMake.EVAL.RULES.LIB
########################################################################

########################################################################
# ShakeNMake.EVAL.RULES.DLL builds builds
# $(1)$(ShakeNMake.EXTENSIONS.DLL) from object files defined by
# $(1).DLL.OBJECTS and $(1).DLL.SOURCES. Flags passed on to the shared
# library linker ($(CXX)) include:
#
#   LDFLAGS, $(1).DLL.LDFLAGS, LDADD, -shared -export-dynamic
#   $(1).DLL.OBJECTS
#
# Also defines the var $(1).DLL, which expands to the filename of the DLL,
# (normally $(1)$(ShakeNMake.EXTENSIONS.DLL)).
define ShakeNMake.EVAL.RULES.DLL
$(1).DLL = $(1)$(ShakeNMake.EXTENSIONS.DLL)
ifneq (.DLL,$(ShakeNMake.EXTENSIONS.DLL))
$(1).DLL: $$($(1).DLL)
endif
CLEAN_FILES += $$($(1).DLL)
$$($(1).DLL): $$($(1).DLL.SOURCES) $$($(1).DLL.OBJECTS)
	@test x = "x$$($(1).DLL.OBJECTS)$$($(1).DLL.SOURCES)" && { \
	echo "$(1).DLL.OBJECTS and/or $(1).DLL.SOURCES are/is undefined!"; exit 1; }; \
	$(call ShakeNMake.CALL.SETX,"LD [$$@] ..."); \
	 $$(CXX) -o $$@ -shared -rdynamic $$(LDFLAGS) \
		$$($(1).DLL.LDFLAGS) $$($(1).DLL.OBJECTS) \
		$$($(1).DLL.CXXFLAGS)
endef
########################################################################
# $(call ShakeNMake.CALL.RULES.DLLS,[list]) calls and $(eval)s
# ShakeNMake.EVAL.RULES.DLL for each entry in $(1)
define ShakeNMake.CALL.RULES.DLLS
$(foreach dll,$(1),$(eval $(call ShakeNMake.EVAL.RULES.DLL,$(dll))))
endef
# end ShakeNMake.CALL.RULES.DLLS and friends
########################################################################


########################################################################
# ShakeNMake.EVAL.RULES.BIN is intended to be called like so:
# $(eval $(call ShakeNMake.EVAL.RULES.BIN,MyApp))
#
# It builds a binary named $(1) by running $(CXX) and passing it:
#
# LDFLAGS, $(1).BIN.LDFLAGS
# $(1).BIN.OBJECTS
define ShakeNMake.EVAL.RULES.BIN
$(1).BIN = $(1)$(ShakeNMake.EXTENSIONS.EXE)
$(1).BIN: $$($(1).BIN)
# Many developers feel that bins should not be cleaned by 'make
# clean', but instead by distclean, but i'm not one of those
# developers. i subscribe more to the school of thought that distclean
# is for cleaning up configure-created files. That said, shake-n-make
# isn't designed to use a configure-like process, so that is probably
# moot here and we probably (maybe?) should clean up bins only in
# distclean. As always: hack it to suit your preference:
CLEAN_FILES += $$($(1).BIN)
$$($(1).BIN): $$($(1).BIN.OBJECTS)
	@test x = "x$$($(1).BIN.OBJECTS)" && { \
	echo "$(1).BIN.OBJECTS is undefined!"; exit 1; }; \
	$(call ShakeNMake.CALL.SETX,"CXX [$$@] ..."); \
	$$(CXX) -o $$@ \
		$$(CXXFLAGS) $$($(1).BIN.CXXFLAGS) \
		$$($(1).BIN.OBJECTS) \
		$$(LDFLAGS) $$($(1).BIN.LDFLAGS)
# note about 'set -x': i do this because it normalizes backslashed
# newline, extra spaces, and other oddities of formatting.
endef
########################################################################
# $(call ShakeNMake.CALL.RULES.BINS,[list]) calls and $(eval)s
# ShakeNMake.EVAL.RULES.BIN for each entry in $(1)
define ShakeNMake.CALL.RULES.BINS
$(foreach bin,$(1),$(eval $(call ShakeNMake.EVAL.RULES.BIN,$(bin))))
endef
# end ShakeNMake.CALL.RULES.BIN and friends
########################################################################

########################################################################
# Automatic dependencies generation for C/C++ code...
# To disable deps generation, set ShakeNMake.USE_MKDEPS=0 *before*
# including this file.
#ShakeNMake.USE_MKDEPS := 1
ifeq (,$(ShakeNMake.BINS.GCC))
ShakeNMake.USE_MKDEPS ?= 0
else
ShakeNMake.USE_MKDEPS ?= 1
endif
#$(warning ShakeNMake.USE_MKDEPS=$(ShakeNMake.USE_MKDEPS));
ifeq (1,$(ShakeNMake.USE_MKDEPS))
ShakeNMake.CISH_SOURCES ?= $(wildcard *.cpp *.c *.c++ *.h *.hpp *.h++ *.hh)
#$(warning ShakeNMake.CISH_SOURCES=$(ShakeNMake.CISH_SOURCES))
ifneq (,$(ShakeNMake.CISH_SOURCES))
ShakeNMake.CISH_DEPS_FILE := .make.c_deps
ShakeNMake.BINS.MKDEP = gcc -E -MM $(CPPFLAGS) $(INCLUDES)
CLEAN_FILES += $(ShakeNMake.CISH_DEPS_FILE)
$(ShakeNMake.CISH_DEPS_FILE): $(PACKAGE.MAKEFILE) $(ShakeNMake.MAKEFILE) $(ShakeNMake.CISH_SOURCES)
	@touch $@; test x = "x$(ShakeNMake.CISH_SOURCES)" && exit 0; \
	$(ShakeNMake.BINS.MKDEP) $(ShakeNMake.CISH_SOURCES) 2>/dev/null > $@
# normally we also want:
#  || $(ShakeNMake.BINS.RM) -f $@ 2>/dev/null
# because we don't want a bad generated makefile to kill the build, but gcc -E
# is returning 1 when some generated files do not yet exist.
deps: $(ShakeNMake.CISH_DEPS_FILE)

ifneq (,$(strip $(filter distclean clean,$(MAKECMDGOALS))))
# $(warning Skipping C/C++ deps generation.)
# ABSOLUTEBOGO := $(shell $(ShakeNMake.BINS.RM) -f $(ShakeNMake.CISH_DEPS_FILE))
else
#$(warning Including C/C++ deps.)
-include $(ShakeNMake.CISH_DEPS_FILE)
endif

endif
# ^^^^ ifneq(,$(ShakeNMake.CISH_SOURCES))
endif
# ^^^^ end $(ShakeNMake.USE_MKDEPS)
########################################################################

########################################################################
# Deps, Tromey's Way:
# $(call make-depend,source-file,object-file,depend-file)
ifeq (0,1)
.dfiles.list = $(wildcard *.o.d)
ifneq (,$(.dfiles.list))
# $(error include $(.dfiles.list))
include $(.dfiles.list)
endif

define make-depend
$(ShakeNMake.BINS.GCC) -MM \
	-MF $3         \
	-MP            \
	-MT $2         \
	$(CFLAGS)      \
	$(CPPFLAGS)    \
	$(TARGET_ARCH) \
	$1
endef

CLEAN_FILES += $(wildcard *.o.d)
%.o: %.c
	$(call make-depend,$<,$@,$(@).d)
	$(COMPILE.c) $(OUTPUT_OPTION) $<
endif # Tromey's Way
