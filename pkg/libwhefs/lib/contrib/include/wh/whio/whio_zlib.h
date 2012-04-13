#if !defined(WANDERINGHORSE_NET_WHIO_ZLIB_H_INCLUDED)
#define WANDERINGHORSE_NET_WHIO_ZLIB_H_INCLUDED 1

#if ! defined(WHIO_ENABLE_ZLIB)
#  define WHIO_ENABLE_ZLIB 0
#endif

#include "whio_stream.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
   Compresses src to dest using gzip compression of the level specified
   by the level parameter (3 is an often-used choice). zlib provides
   several constants for this value:

   Z_NO_COMPRESSION, Z_BEST_SPEED, Z_BEST_COMPRESSION, and
   Z_DEFAULT_COMPRESSION.

   If level is not in the bounds defined by those constants, it will
   be adjusted up (if too low) or down (if too high) to the minimum or
   maximum compression level.

   src must be a readable stream and dest must be writeable. They may
   not be the same object.

   If whio is not compiled with WHIO_ENABLE_ZLIB defined to a true value,
   this function does nothing and returned whio_rc.UnsupportedError.

   Returns whio_rc.OK on success, else some error value from one of
   zlib routines (a non-zero error code defined in zlib.h). If
   !src or !dest or (src==dest) then whio_rc.ArgError is returned.

   The compressed data is decompressable by gzip-compatible tools.

   Note that because a whio_stream instance can be created for any
   whio_dev device (see whio_stream_for_dev()), it is possible to use
   this routine to compress any i/o device to another.  However,
   random access with transparent compression/decompression is not
   supported (very few people have every managed to code that).

   On error, any number of bytes may or may not have been read from src
   or written to dest.

   @see whio_stream_gunzip()
   @see whio_stream_for_dev()
 */
int whio_stream_gzip( whio_stream * restrict src, whio_stream * restrict dest, int level );

/**
   Assumes src contains gzipped data and decompresses it to dest.

   src must be a readable stream and dest must be writeable. They may
   not be the same object.

   If whio is not compiled with WHIO_ENABLE_ZLIB defined to a true value,
   this function does nothing and returned whio_rc.UnsupportedError.

   Returns whio_rc.OK on success, else some error value from one of
   zlib routines (a non-zero error code defined in zlib.h). If !src or
   !dest or (src==dest) then whio_rc.ArgError is returned.

   On error, any number of bytes may or may not have been read from src
   or written to dest.

   @see whio_stream_gzip()
   @see whio_stream_for_dev()
*/
int whio_stream_gunzip( whio_stream * restrict src, whio_stream * restrict dest );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHIO_ZLIB_H_INCLUDED */
