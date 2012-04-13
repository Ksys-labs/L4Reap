PKGDIR		?= ../../
L4DIR		?= $(PKGDIR)/../..

PC_FILENAME	= libwhefs
TARGET		= libwhefs.a libwhefs.so

LIBS		= $(GCCLIB)
REQUIRES_LIBS	= libc_be_file_stdin zlib libstdc++ libc_be_l4refile libc_be_l4re 


PRIVATE_INCDIR	= $(PKGDIR)/include

DEFINES		= -DWHEFS_CONFIG_ENABLE_MMAP=1 -DWHEFS_CONFIG_ENABLE_BITSET_CACHE=1 \
		-DWHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE=1 \
		-DWHIO_DEBUG_ENABLED=1 -DWHDBG_CONFIG_ENABLE=1
#-DDEBUG=1 -UNDEBUG

WARNINGS	= -Wno-unused

LDFLAGS		=  --warn-common --no-undefined

SRC_C	= \
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
	whglob.c \
	whbits.c \
	whdbg.c \
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
	whprintf.c

SRC_CC 	= whio_stream_stl.cc

vpath %.cc $(SRC_DIR)/../contrib/src

vpath %.c $(SRC_DIR)/../contrib/src

include $(L4DIR)/mk/lib.mk
