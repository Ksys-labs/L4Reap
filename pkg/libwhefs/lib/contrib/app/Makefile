#!/usr/bin/make -f
include ../config.make


WHEFS_BINS_LDFLAGS := -L$(LIBWHEFS.LIBDIR) -lwhefs # $(LIBWHEFS_CLIENT_LDFLAGS) 
WHEFS_BINS_DEPS := $(LIBWHEFS.A) $(LIBWHEFS.DLL) $(ALL_MAKEFILES)

.PHONY: bins

########################################################################
# test bin
test.BIN.OBJECTS := test.o whargv.o
test.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,test)
$(test.BIN): $(WHEFS_BINS_DEPS)
bins: $(test.BIN)

whio-test.BIN.OBJECTS := whio-test.o
whio-test.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whio-test)
$(whio-test.BIN): $(WHEFS_BINS_DEPS)
bins: $(whio-test.BIN)


########################################################################
# Converts arbitrary input to a C char array.
ifeq (1,1)
bin2c.BIN.OBJECTS := bin2c.o
bin2c.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,bin2c)
$(bin2c.BIN): $(WHEFS_BINS_DEPS)
bins: $(bin2c.BIN)
endif

########################################################################
#
ifeq (1,1)
whefs2c.BIN.OBJECTS := whefs2c.o
#whefs2c.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs2c)
$(whefs2c.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs2c.BIN)
endif


########################################################################
# whefs-ls
ifeq (1,1)
whefs-ls.BIN.OBJECTS := ls.o whargv.o
whefs-ls.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-ls)
$(whefs-ls.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-ls.BIN)
endif

########################################################################
# whefs-mkfs
ifeq (1,1)
whefs-mkfs.BIN.OBJECTS := mkfs.o whargv.o
whefs-mkfs.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-mkfs)
$(whefs-mkfs.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-mkfs.BIN)
endif

########################################################################
# whefs-cp
ifeq (1,1)
whefs-cp.BIN.OBJECTS := cp.o whargv.o
whefs-cp.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-cp)
$(whefs-cp.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-cp.BIN)
endif

########################################################################
# whefs-rm
ifeq (1,1)
whefs-rm.BIN.OBJECTS := rm.o whargv.o
whefs-rm.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-rm)
$(whefs-rm.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-rm.BIN)
endif

########################################################################
# whefs-addblocks
ifeq (1,1)
whefs-addblocks.BIN.OBJECTS := addblocks.o whargv.o
whefs-addblocks.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-addblocks)
$(whefs-addblocks.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-addblocks.BIN)
endif

########################################################################
# whefs-addblocks
ifeq (1,1)
whefs-cat.BIN.OBJECTS := cat.o whargv.o
whefs-cat.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,whefs-cat)
$(whefs-cat.BIN): $(WHEFS_BINS_DEPS)
bins: $(whefs-cat.BIN)
endif

########################################################################
# The subfs demo demonstrates a VFS within a VFS.
ifeq (1,1)
subfs.BIN.OBJECTS := subfs.o whargv.o
subfs.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,subfs)
$(subfs.BIN): $(WHEFS_BINS_DEPS)
bins: $(subfs.BIN)
endif


########################################################################
# The staticfs demo creates a VFS, imports some files, converts the VFS
# to C code, builds an application with that VFS built in as a static
# variable, and uses the VFS from that memory.
ifeq (1,1)
whefs-mkfs-for-files.sh:
StaticWHEFS.whefs: $(WHEFS_BINS_DEPS) whefs-mkfs-for-files.sh $(whefs-ls.BIN) $(whefs-cp.BIN) $(whefs-mkfs.BIN)
	bash ./whefs-mkfs-for-files.sh $@ [a-z]*.c [a-z]*.h </dev/null
CLEAN_FILES += StaticWHEFS*.whefs StaticWHEFS.c
StaticWHEFS.c: StaticWHEFS.whefs $(whefs2c.BIN)
	./$(whefs2c.BIN) StaticWHEFS < StaticWHEFS.whefs > $@
staticfs.o: StaticWHEFS.c staticfs.c
staticfs.BIN.OBJECTS := staticfs.o
staticfs.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,staticfs)
$(staticfs.BIN): $(WHEFS_BINS_DEPS)
# bins: $(staticfs.BIN)
endif

#ifeq (1,1)
#avl.BIN.OBJECTS := avl.o whargv.o
#avl.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
#$(call ShakeNMake.CALL.RULES.BINS,avl)
#$(avl.BIN): $(WHEFS_BINS_DEPS)
#bins: $(avl.BIN)
#endif

ifeq (1,1)
plus.BIN.OBJECTS := plus.o
plus.BIN.LDFLAGS := $(WHEFS_BINS_LDFLAGS)
$(call ShakeNMake.CALL.RULES.BINS,plus)
$(plus.BIN): $(WHEFS_BINS_DEPS)
bins: $(plus.BIN)
endif


all: $(test.BIN) $(bin2c.BIN) \
	$(whefs-addblocks.BIN) \
	$(whefs-cat.BIN) \
	$(whefs-cp.BIN) \
	$(whefs-ls.BIN) \
	$(whefs-mkfs.BIN) \
	$(whefs-rm.BIN) \
	$(subfs.BIN) \
	$(whio-test.BIN) \
	$(plus.BIN)
