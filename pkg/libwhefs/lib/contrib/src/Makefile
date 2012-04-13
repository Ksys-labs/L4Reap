#!/usr/bin/make -f
include ../config.make


WHIO_SOURCES := \
	whio.c \
	whio_common.c \
	whio_dev.c \
	whio_dev_FILE.c \
	whio_dev_fileno.c \
	whio_dev_mem.c \
	whio_dev_subdev.c \
	whio_encode.c \
	whio_stream.c \
	whio_stream_dev.c \
	whio_stream_FILE.c \
	whio_zlib.c \
	whio_stream_stl.cpp \
	whprintf.c

MISC_SOURCES := \
	whglob.c \
	whbits.c \
	whdbg.c

WHEFS_SOURCES := \
	whefs_block.c \
	whefs.c \
	whefs_cache.c \
	whefs_client_util.c \
	whefs_encode.c \
	whefs_file.c \
	whefs_fs.c \
	whefs_fs_closer.c \
	whefs_hash.c \
	whefs_inode.c \
	whefs_nodedev.c \
	whefs_string.c \
	$(MISC_SOURCES)


SOURCES := $(WHEFS_SOURCES) $(WHIO_SOURCES)

WHIO_OBJ := $(patsubst %.c,%.o,$(WHIO_SOURCES))
WHIO_OBJ := $(patsubst %.cpp,%.o,$(WHIO_OBJ))
#$(error WHIO_OBJ=$(WHIO_OBJ))
ifeq (1,$(WHIO_ENABLE_ZLIB))
  WHIO_LIB_LDFLAGS += -lz
  whio_zlib.o: CPPFLAGS+=-DWHIO_ENABLE_ZLIB=1
endif

WHEFS_OBJ := $(WHEFS_SOURCES:%.c=%.o)

OBJECTS := $(SOURCES:%.c=%.o)
OBJECTS := $(patsubst %.cpp,%.o,$(OBJECTS))
$(OBJECTS): $(ALL_MAKEFILES)

whio.o whio%.o: CPPFLAGS+=-DWHIO_CONFIG_ENABLE_STATIC_MALLOC=$(WHIO_ENABLE_STATIC_MALLOC)
whefs.o whefs%.o: CPPFLAGS+=-DWHEFS_CONFIG_ENABLE_STATIC_MALLOC=$(WHEFS_ENABLE_STATIC_MALLOC) \
	-DWHEFS_CONFIG_ENABLE_BITSET_CACHE=$(WHEFS_ENABLE_BITSET_CACHE) \
	-DWHEFS_CONFIG_ENABLE_FCNTL=$(WHEFS_ENABLE_FCNTL) \
	-DWHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE=$(WHEFS_ENABLE_STRINGS_HASH_CACHE)
whefs_fs.o: CPPFLAGS+=-DWHEFS_CONFIG_ENABLE_MMAP=$(ENABLE_MMAP) -DWHEFS_CONFIG_ENABLE_MMAP_ASYNC=$(ENABLE_MMAP_ASYNC)

#liba-whefs.OBJECTS := $(OBJECTS)
#liba-whefs
libwhefs.LIB.OBJECTS := $(OBJECTS)
$(call ShakeNMake.CALL.RULES.LIBS,libwhefs)

libwhefs.DLL.OBJECTS := $(libwhefs.LIB.OBJECTS)
libwhefs.DLL.LDFLAGS := $(WHIO_LIB_LDFLAGS)
$(call ShakeNMake.CALL.RULES.DLLS,libwhefs)

all: $(libwhefs.LIB) $(libwhefs.DLL)
#	@echo ALL_MAKEFILES=$(ALL_MAKEFILES)
#	@echo OBJECTS=$(OBJECTS)
#	@echo TOP_SRCDIR=$(TOP_SRCDIR)
#	@echo MAKEFILE_LIST=$(MAKEFILE_LIST)

