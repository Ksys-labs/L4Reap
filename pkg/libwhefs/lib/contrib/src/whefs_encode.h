#if !defined(WANDERINGHORSE_NET_WHEFS_ENCODE_H_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_ENCODE_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/

#include <wh/whefs/whefs_config.h>
#include <wh/whio/whio_encode.h>
#include <stddef.h> /* size_t on my box */
/** @file whefs_encode.h

   This file contains the encoding/decoding parts of the whefs
   private/internal API.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   This enum defines some on-disk sizes for the utility routines
   which encode/decode data to/from whio_dev objects.
*/
enum whefs_sizeof_encoded {

/** @var whefs_sizeof_encoded_id_type
   On-disk encoded size of whefs_id_type objects. This will always be
   one of 1, 2, 4, or 4, depending on the size of whefs_id_type, plus 1
   tag byte used by the encoding/decoding bits for consistency
   checking.

   Maintenance reminder: we use (WHEFS_ID_TYPE_BITS/8) here instead of
   sizeof(whefs_id_type), because we need a specific guaranteed size
   for each supported value of WHEFS_ID_TYPE_BITS, and sizeof()
   doesn't give us that.
*/
whefs_sizeof_encoded_id_type = ((WHEFS_ID_TYPE_BITS/8)+1),

/**
   The on-disk size of an inode record, not including the
   inode name.
*/
whefs_sizeof_encoded_inode = 1 /* tag byte */
        + whefs_sizeof_encoded_id_type /* id */
        + whio_sizeof_encoded_uint8 /* flags */
	+ whio_sizeof_encoded_uint32 /* mtime */
	+ whio_sizeof_encoded_uint32 /* data_size */
        + whefs_sizeof_encoded_id_type /* first_block */,

/**
   This is the on-disk size of the HEADER for an inode name. The
   actual length is this number plus the associated
   whefs_fs_options::filename_length.
*/
whefs_sizeof_encoded_inode_name_header = 1 /* tag byte */
    + whefs_sizeof_encoded_id_type /* id */
    + whio_sizeof_encoded_uint16 /* length */,

/**
   The size of the internal stack-alloced buffers needed for encoding
   inode name strings, including their metadata.


   Hacker's note: the on-disk size of an encoded inode name for a
   given whefs_fs object can be fetched with whefs_fs_sizeof_name().
*/
whefs_sizeof_encoded_inode_name = 1 /* tag bytes */
+ whefs_sizeof_encoded_inode_name_header
+ WHEFS_MAX_FILENAME_LENGTH
+ 1 /* trailing null */,

/**
   The on-disk size of the metadata parts of a block, which preceeds
   the data area of the block.
*/
whefs_sizeof_encoded_block = 1 /* tag char */
    + whefs_sizeof_encoded_id_type /* bl->id */
    + whio_sizeof_encoded_uint16 /* bl->flags */
    + whefs_sizeof_encoded_id_type /* bl->next_block */,

/** Not yet used. */
whefs_sizeof_encoded_hints = 1 /* tag char */
    + whefs_sizeof_encoded_id_type /* bl->hints.unused_inode_start */
    + whefs_sizeof_encoded_id_type /* bl->hints.unused_block_start */
    + 0

};

/**
   Generates a hash code for the first n bytes of the given memory,
   or 0 if n is 0 or data is null.

   The exact hash routine is unspecified, and may change from version
   to version if a compelling reason to do so is found. The hash code
   is intended to be used in ways that will not cause an
   imcompatibility in whefs file format if the hash implementation
   changes.
 */
uint64_t whefs_bytes_hash( void const * data, uint32_t n );
#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHEFS_ENCODE_H_INCLUDED */
