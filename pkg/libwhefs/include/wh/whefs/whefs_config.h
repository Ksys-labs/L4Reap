#if !defined(WANDERINGHORSE_NET_WHEFS_CONFIG_H_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_CONFIG_H_INCLUDED

/*
  This file contains the compile-time-configurable parts of
  libwhefs. Versions of whefs compiled using different options will
  not be compatible, as these options can affect data sizes and
  ranges, as well as the VFS filesystem layout.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain

  Maintenance reminders:

  - This file should be included BEFORE any system includes, if possible, because
  it sets up some #defines which are used to enable, e.g. specific POSIX APIs.

*/

#if defined(WHEFS_ID_TYPE_BITS)
#  error "WHEFS_ID_TYPE_BITS must not be defined before including this file! See the docs for WHEFS_ID_TYPE_BITS for details!"
#endif

#if ! defined __STDC_FORMAT_MACROS
/* The Linux stdint.h says:

"The ISO C99 standard specifies that these macros must only be
 defined if explicitly requested."
*/
#  define __STDC_FORMAT_MACROS 1
#endif
#include <stdint.h> /* standardized fixed-size integer types */
#include <inttypes.h> /* printf/scanf format specifiers. */
#include "whefs_license.h"
#include <wh/whio/whio_config.h> /* WHIO_SIZE_T_BITS */

#ifdef __cplusplus
extern "C" {
#endif

/** @typedef whefs_id_type

   whefs_id_type is the numeric type used to store block and inode IDs
   in a VFS, as well as any count/size values which refer to such
   items. The library supports building with different sizes of this
   type, largely as a way to conserve memory, and this typedef helps
   support that. See the documentation for WHEFS_ID_TYPE_BITS for full
   details.
*/

/** @def WHEFS_ID_TYPE_BITS

WHEFS_ID_TYPE_BITS tells us how many bits to use for inode and data
block IDs. The supported values are 8, 16, 32, or 64, and libraries
compiled with different values will not be compatible (nor can they
read the others' format, though i'm looking into ways to be able to
enable that).

The number of bits here is significant because:

- We are limited to (2^WHEFS_ID_TYPE_BITS-2) inodes and blocks
(but for all practical purposes, this isn't a limitation even with
16 bits).

- The layout of the filesystem is dependent on the number of bits in
this type, as the number of bytes used for certain data fields changes.

- When/if inode or block caching is added, the cache size will incrase
by (WHEFS_ID_TYPE_BITS/8) for each ID field stored in the cache
(that's a minimum of 2 IDs per object, and maybe up to 6 per inode
once directory support is added). The number grows quickly.

In practice, a 16-bit ID type is completely sufficient, but the
library was originally created with size_t as the identifier type, so
it supports 32-bit IDs as well. The switch to 16 as the default was
made to help save memory when/if inode caching gets added to the
library. In any case, the filesystem's minimalistic implementation is
not scalable to tens of thousands of files, and so 16 bits is a very
realistic limit.

In theory, 64-bit IDs are also okay, but (A) for this particular use
case that is way overkill, (B) it's a huge waste of memory (8 bytes
where we realistically don't need more than 2), and (C) the filesystem
model itself is not scalable to that level of use (e.g. billions of
files). inode/block IDs are always sequential, starting at 1, and by
the time we hit that number of blocks or inodes, the computer's memory
would almost certainly be filled.

i would strongly prefer to have WHEFS_ID_TYPE_BITS as an enum constant
instead of a macro value, but we unfortunately need some conditional
compilation based on the bit count.

If this constant is changed, all whefs client code which depends on it
must be recompiled and all filesystems written using the old value
will not be readable. That is the reason it is not set up with an
ifndef guard, so clients cannot blithely change it. If you are copying
whefs directly into another project, feel free to change the value all
you want, but be aware of the compatibility ramifications. Doing so
may also screw up any printf() (or similar) commands which have a
hard-coded format specifier which no longer matches after changing
this value. To work around this, the constant WHEFS_ID_TYPE_PFMT gets
defined (dependent on the value of WHEFS_ID_TYPE_BITS) and can be used
in place of hard-coding the printf format specifier.

    @see WHEFS_ID_TYPE_PFMT
    @see WHEFS_ID_TYPE_SFMT

*/
#define WHEFS_ID_TYPE_BITS 16

/** @def WHEFS_ID_TYPE_PFMT

    WHEFS_ID_TYPE_PFMT is a helper string for writing portable
    printf-style format strings in conjunction with whefs_id_type. It
    works by using C99-standard macros defined in inttypes.h. It
    selects which specifier to use based on the value of
    WHEFS_ID_TYPE_BITS.  The system's inttypes.h will select a
    platform/bitness-dependent value.

    @see WHEFS_ID_TYPE_BITS
    @see WHEFS_ID_TYPE_SFMT
*/

/** @def WHEFS_ID_TYPE_SFMT

    WHEFS_ID_TYPE_SFMT is the scanf counterpart of WHEFS_ID_TYPE_PFMT.

    @see WHEFS_ID_TYPE_BITS
    @see WHEFS_ID_TYPE_PFMT
*/


/** @var whefs_fs_magic_bytes

    whefs_fs_magic_bytes is the whefs "internal" magic cookie, an
    array of numeric values used for sanity checking and confirming
    that input files are really whefs VFS files. This is independent
    of the client cookie, and dependent on the library version. When
    the core library's binary expectations change, in particular how
    the FS objects are stored change, this number will change. That
    will inherently make all other versions of whefs (both older and
    newer!) incompatible with VFSs created by this version.

    The convention is to store YYYY-MM-DD-WHEFS_ID_TYPE_BITS, using the
    date of the change as the value. Conventions aside, the only hard
    requirement (if you're customizing this) is that the array be
    terminated by a 0, as some routines may rely on that so they don't
    have to know the length of the array in advance.

    Maintenance reminder: When the version number is changed,
    be sure to change WHEFS_MAGIC_STRING_PREFIX as well!

    @see WHEFS_MAGIC_STRING_PREFIX WHEFS_MAGIC_STRING
*/
static const uint32_t whefs_fs_magic_bytes[] = { 2009, 12, 1, WHEFS_ID_TYPE_BITS, 0 };
/** @def WHEFS_MAGIC_STRING_PREFIX

    WHEFS_MAGIC_STRING_PREFIX is an internal helper macro to avoid
    some code repetition.

    Maintenance reminder: keep this value in sync with that defined
    in whefs_fs_magic_bytes!

    @see whefs_fs_magic_bytes WHEFS_MAGIC_STRING
*/
#define WHEFS_MAGIC_STRING_PREFIX "whefs version 20091201 with "

#if WHEFS_ID_TYPE_BITS == 8
/* for very, very limited filesystems. There's lots of room for overflows here! */
#  define WHEFS_MAGIC_STRING WHEFS_MAGIC_STRING_PREFIX"8-bit IDs"
#  define WHEFS_ID_TYPE_PFMT PRIu8
#  define WHEFS_ID_TYPE_SFMT SCNu8
    typedef uint8_t whefs_id_type;
#elif WHEFS_ID_TYPE_BITS == 16
/* the most realistic value, IMO. */
#  define WHEFS_MAGIC_STRING WHEFS_MAGIC_STRING_PREFIX"16-bit IDs"
#  define WHEFS_ID_TYPE_PFMT PRIu16
#  define WHEFS_ID_TYPE_SFMT SCNu16
    typedef uint16_t whefs_id_type;
#elif WHEFS_ID_TYPE_BITS == 32
#  define WHEFS_MAGIC_STRING WHEFS_MAGIC_STRING_PREFIX"32-bit IDs"
#  define WHEFS_ID_TYPE_PFMT PRIu32
#  define WHEFS_ID_TYPE_SFMT SCNu32
    typedef uint32_t whefs_id_type;
#elif WHEFS_ID_TYPE_BITS == 64
#  define WHEFS_ID_TYPE_PFMT PRIu64
#  define WHEFS_ID_TYPE_SFMT SCNu64
#  define WHEFS_MAGIC_STRING WHEFS_MAGIC_STRING_PREFIX"64-bit IDs"
    typedef uint64_t whefs_id_type;
#else
#  error "WHEFS_ID_TYPE_BITS must be one of: 8, 16, 32, 64"
#endif

#if WHEFS_ID_TYPE_BITS > WHIO_SIZE_T_BITS
#  error "WHEFS_ID_TYPE_BITS must be <= WHIO_SIZE_T_BITS"
#endif

/** @def WHEFS_MAGIC_STRING

   The default magic cookie string used by the library.
*/

/** @enum whefs_constants

    This enum contains some important compile-time constants
    for whefs.
*/
enum whefs_constants {
/**
   WHEFS_MAX_FILENAME_LENGTH defines the hard maximum string length
   for filenames in an EFS, not including the null terminator. Since
   the filesystem namespace is flat, this DOES include the names of
   any parent "directories" (which aren't really directories, as far
   as the EFS is concerned).

   whefs_fs_options::filename_length must not be greater than this
   number.

   This value plays a big part in the in-memory size of certain
   internals, and should be kept to a reasonable length. While 1024 sounds
   reasonable, setting it that high will drastically increase the
   memory costs of certain operations.
 */
WHEFS_MAX_FILENAME_LENGTH = 128,

/**
   Newer-style name for WHEFS_MAX_FILENAME_LENGTH.
*/
whefs_sizeof_max_filename = WHEFS_MAX_FILENAME_LENGTH,

/**
   The length of the whefs_fs_magic_bytes array, not including the
   tailing 0 entry.
*/
whefs_fs_magic_bytes_len = (sizeof(whefs_fs_magic_bytes)
			    /sizeof(whefs_fs_magic_bytes[0]) - 1)

};

/** @def WHEFS_CONFIG_ENABLE_FCNTL

If WHEFS_CONFIG_ENABLE_FCNTL is true then some features for doing
POSIX-style fcntl() advisory locks are added. The locking
code is far from complete and should not be relied upon
for concurrency purposes.

Once it is complete, the fcntl() locking will only work for file-based
base VFSes, not memory-based ones. When it is opened the VFS tries
to determine if it is using a file and if it is then locks will be
used for certain operations.

Maintenance reminder: if this is true be sure to include fcntl.h in
the files which need it.
*/
#if !defined(WHEFS_CONFIG_ENABLE_FCNTL)
#define WHEFS_CONFIG_ENABLE_FCNTL 0
#endif

/** @def WHEFS_CONFIG_ENABLE_THREADS

WHEFS_CONFIG_ENABLE_THREADS doesn't yet do anything. It is reserved for
when some form of thread locking is enabled.

*/
#if !defined(WHEFS_CONFIG_ENABLE_THREADS)
#  define WHEFS_CONFIG_ENABLE_THREADS 0
#endif

/** @def WHEFS_MACROIZE_SMALL_CHECKS

   WHEFS_MACROIZE_SMALL_CHECKS tells the API to swap out certain check
   functions with macros. Profiling has shown, e.g. whefs_block_id_is_valid()
   and whefs_inode_id_is_valid() to take about 1% of the runtime in some tests.
   By converting them to macros we cut out a lot of function calls, or we can
   remove them altogether if we ensure that consistency checks are put in all
   entryways.
*/
#if defined(DOXYGEN)
  /* we do this so that doxygen can pick up the non-macroized funcs. */
#  define WHEFS_MACROIZE_SMALL_CHECKS 0
#else
#  define WHEFS_MACROIZE_SMALL_CHECKS 1
#endif

/** @def WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE

If WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE is set to false then the
code for caching the hashcode of inode names is disabled by
default. The cache costs a small amount (about 8 bytes/cached inode
entry, more if WHEFS_ID_TYPE_BITS is bigger than 16) but can
*drastically* speed up searches for inode by name if the operation is
done more than once (the initial load populates the cache).

The state of this cache can be changed at runtime using
whefs_fs_set_inode_hash_cache().
*/
#if !defined(WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE)
#define WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE 1
#endif

/** @def WHEFS_CONFIG_ENABLE_STATIC_MALLOC

    See WHIO_CONFIG_ENABLE_STATIC_MALLOC, from whio_config.h, for a full
    description. The only difference is that this option is used
    only for certain whefs-specific types.
*/
#if !defined(WHEFS_CONFIG_ENABLE_STATIC_MALLOC)
#define WHEFS_CONFIG_ENABLE_STATIC_MALLOC 0
#endif


/** @def WHEFS_CONFIG_ENABLE_MMAP

   If set to true then a whefs which uses a file as backing storage will
   attempt to use mmap() to provide faster access.

    TODO: make the use (or non-use) of mmap() toggleable if this option
    is enabled.
*/
#if !defined(WHEFS_CONFIG_ENABLE_MMAP)
#define WHEFS_CONFIG_ENABLE_MMAP 0
#endif

/** @def WHEFS_CONFIG_ENABLE_MMAP_ASYNC

    If WHEFS_CONFIG_ENABLE_MMAP is false then this macro is ignored,
    otherwise:

    If WHEFS_CONFIG_ENABLE_MMAP_ASYNC is true then mmap() flushing is
    done asynchronously, otherwise is is synchronous. Since all writes
    to pseudofiles trigger a flush (so that the inode info gets
    updates), running mmap() access in synchronous mode can slow it
    down. Running asynchronously speeds it up but could also
    theoretically lead to corruption in more cases than synchronous
    writes do. e.g. what happens if a SIGINT comes in after the write
    has returned (looking like success to the caller) but before the
    commit to disk?

    That said - my simple tests have been inconclusive as to whether
    async mode provides a huge benefit over synchronous mode, and async
    is disabled by default.

    TODO: make this option runtime-configurable.
*/
#if !defined(WHEFS_CONFIG_ENABLE_MMAP_ASYNC)
#  define WHEFS_CONFIG_ENABLE_MMAP_ASYNC 0
#endif

/** @def WHEFS_CONFIG_ENABLE_BITSET_CACHE

If WHEFS_CONFIG_ENABLE_BITSET_CACHE is true then the EFS caches
(using a bitset) whether or not any given inode or block is marked as
used.  This speeds up some operations dramatically but costs malloced
memory: 1 bit per inode plus 1 bit per block plus 1 byte.

This approach to caching is going to Cause Grief (or at least
Discomfort) when dealing with multi-app concurrency issues, as we
cannot keep it in sync across multiple applications.

TODO: re-evaluate the real cost of this cache. Memory is very low, but
profiling has, in some cases, implied that it costs us more
performance than we lose when it is disabled.
*/
#if !defined(WHEFS_CONFIG_ENABLE_BITSET_CACHE)
#define WHEFS_CONFIG_ENABLE_BITSET_CACHE 1
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHEFS_CONFIG_H_INCLUDED */
