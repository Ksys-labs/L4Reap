#if !defined(WANDERINGHORSE_NET_WHIO_ENCODE_H_INCLUDED)
#define WANDERINGHORSE_NET_WHIO_ENCODE_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/

#include <wh/whio/whio_dev.h>
#include <stddef.h> /* size_t on my box */
/** @file whio_encode.h

   This file contains an API for encoding/decoding binary data to/from
   memory buffers and i/o devices.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
   This enum defines some on-disk sizes for the utility routines
   which encode/decode data to/from whio_dev objects.
*/
enum whio_sizeof_encoded {

/** @var whio_sizeof_encoded_uint64

   whio_sizeof_encoded_uint64 is the length (in bytes) of an encoded uint64 value.
   It is 9: 1 tag byte + 8 data bytes.

   @see whio_decode_uint64()
   @see whio_encode_uint64()
*/
whio_sizeof_encoded_uint64 = 9,
whio_sizeof_encoded_int64 = 9,
/** @var whio_sizeof_encoded_uint32

   whio_sizeof_encoded_uint32 is the length (in bytes) of an encoded uint32 value.
   It is 5: 1 tag byte + 4 data bytes.

   @see whio_decode_uint32()
   @see whio_encode_uint32()
*/
whio_sizeof_encoded_uint32 = 5,
whio_sizeof_encoded_int32 = 5,

/** @var whio_sizeof_encoded_uint16

   whio_sizeof_encoded_uint16 is the length (in bytes) of an encoded uint16 value.
   It is 3: 1 tag byte + 2 data bytes.

   @see whio_decode_uint16()
   @see whio_encode_uint16()
*/
whio_sizeof_encoded_uint16 = 3,
whio_sizeof_encoded_int16 = 3,

/** @var whio_sizeof_encoded_uint8

   whio_sizeof_encoded_uint8 is the length (in bytes) of an encoded uint8 value.
   It is 2: 1 tag byte + 1 data byte.

   @see whio_decode_uint8()
   @see whio_encode_uint8()
*/
whio_sizeof_encoded_uint8 = 2,
whio_sizeof_encoded_int8 = 2,

/** @var whio_size_cstring

   whio_size_cstring is the encoded length of a C-style string,
   NOT INCLUDING the actual string bytes. i.e. this is the header
   size.

   @see whio_decode_cstring()
   @see whio_encode_cstring()
*/
whio_sizeof_encoded_cstring = 1 + whio_sizeof_encoded_uint32,

/**
   The encoded size of a whio_size_t object. Its size depends
   on the value of WHIO_SIZE_T_BITS.
*/
whio_sizeof_encoded_size_t =
#if WHIO_SIZE_T_BITS == 64
    whio_sizeof_encoded_uint64
#elif WHIO_SIZE_T_BITS == 32
    whio_sizeof_encoded_uint32
#elif WHIO_SIZE_T_BITS == 16
    whio_sizeof_encoded_uint16
#elif WHIO_SIZE_T_BITS == 8
    whio_sizeof_encoded_uint8
#else
#error "WHIO_SIZE_T_BITS is not a supported value. Try one of (8,16,32,64)!"
#endif

};

   
/**
   Encodes a 32-bit integer value into 5 bytes - a leading tag/check
   byte, then the 4 bytes of the number, in big-endian format. Returns
   the number of bytes written, which will be equal to
   whio_sizeof_encoded_uint32 on success.

   dest must be valid memory at least whio_sizeof_encoded_uint32 bytes long.

   @see whio_decode_uint32()
*/
size_t whio_encode_uint32( unsigned char * dest, uint32_t i );
/**
   The unsigned counterpart of whio_encode_uint32().
*/
size_t whio_encode_int32( unsigned char * dest, int32_t i );

/**
   The converse of whio_encode_uint32(), this tries to read an
   encoded 32-bit value from the given memory. On success it returns
   whio_rc.OK and sets tgt (if not null) to that value. On error it
   returns some other value from whio_rc and does not modify tgt.

   src must be valid memory at least whio_sizeof_encoded_uint32 bytes
   long.

   Error values include:

   - whio_rc.ArgError = !src

   - whio_rc.ConsistencyError = the bytes at the current location
   were not encoded with whio_encode_uint32().

   @see whio_encode_uint32()

*/
int whio_decode_uint32( unsigned char const * src, uint32_t * tgt );
/**
   The unsigned counterpart of whio_decode_uint32().
*/
int whio_decode_int32( unsigned char const * src, int32_t * tgt );

/**
   Similar to whio_encode_uint32(), with the same conventions, but
   works on 16-bit numbers. Returns the number of bytes written, which
   will be equal to whio_sizeof_encoded_uint16 on success.

   dest must be valid memory at least whio_sizeof_encoded_uint16
   bytes long.

   @see whio_decode_uint16()
*/
size_t whio_encode_uint16( unsigned char * dest, uint16_t i );
/**
   The unsigned counterpart of whio_encode_uint16().
*/
size_t whio_encode_int16( unsigned char * dest, int16_t i );

/**
   Similar to whio_decode_uint32(), with the same conventions and
   error codes, but works on 16-bit numbers.  On success it returns
   whio_rc.OK and sets target to that value. On error it returns some
   other value from whio_rc and does not modify tgt.

   src must be valid memory at least whio_sizeof_encoded_uint16 bytes
   long.


   @see whio_encode_uint16()
*/

int whio_decode_uint16( unsigned char const * src, uint16_t * tgt );
/**
   The unsigned counterpart of whio_decode_uint16().
*/
int whio_decode_int16( unsigned char const * src, int16_t * tgt );

/**
   The uint8 counterpart of whio_encode_uint16(). Returns
   whio_sizeof_encoded_uint8 on success and 0 on error. The only
   error condition is that dest is null.
*/
size_t whio_encode_uint8( unsigned char * dest, uint8_t i );
/**
   The unsigned counterpart of whio_encode_uint8().
*/
size_t whio_encode_int8( unsigned char * dest, int8_t i );

/**
   The uint8 counterpart of whio_decode_uint16(). Returns whio_rc.OK
   on success. If !src then whio_rc.ArgError is returned. If src
   does not appear to be an encoded value then whio_rc.ConsistencyError
   is returned.
*/
int whio_decode_uint8( unsigned char const * src, uint8_t * tgt );
/**
   The unsigned counterpart of whio_decode_uint8().
*/
int whio_decode_int8( unsigned char const * src, int8_t * tgt );


/**
   Encodes v to dest. This is just a proxy for one of:
   whio_encode_uint8(), whio_encode_uint16(), whio_encode_uint32() or
   whio_encode_uint64(), depending on the value of WHIO_SIZE_T_BITS.
*/
whio_size_t whio_encode_size_t( unsigned char * dest, whio_size_t v );

/**
   Decodes v from src. This is just a proxy for one of:
   whio_decode_uint8(), whio_decode_uint16(), whio_decode_uint32() or
   whio_decode_uint64(), depending on the value of WHIO_SIZE_T_BITS.
*/
int whio_decode_size_t( unsigned char const * src, whio_size_t * v );

/**
   Encodes v to dev using whio_size_t_encode().
*/
whio_size_t whio_dev_encode_size_t( whio_dev * dev, whio_size_t v );

/**
   Decodes v from dev using whio_size_t_decode().
*/
int whio_dev_decode_size_t( whio_dev * dev, whio_size_t * v );

/**
   The 64-bit variant of whio_encode_uint32(). Follows the same
   conventions as that function but handles a uint64_t value instead
   of uint32_t.

   @see whio_encode_uint16()
   whio_encode_uint32()
   whio_decode_uint64()
*/
size_t whio_encode_uint64( unsigned char * dest, uint64_t i );
/**
   The unsigned counterpart of whio_encode_uint64().
*/
size_t whio_encode_int64( unsigned char * dest, int64_t i );

/**
   The 64-bit variant of whio_decode_uint32(). Follows the same
   conventions as that function but handles a uint64_t value instead
   of uint32_t.

   @see whio_decode_uint16()
   whio_decode_uint32()
   whio_encode_uint64()
*/
int whio_decode_uint64( unsigned char const * src, uint64_t * tgt );
/**
   The unsigned counterpart of whio_decode_uint64().
*/
int whio_decode_int64( unsigned char const * src, int64_t * tgt );

/**
   Uses whio_encode_uint32() to write n elements from the given
   array to dev.  Returns whio_rc.OK on success. Returns the number of
   items written.

   @see whio_encode_uint32()
*/
size_t whio_encode_uint32_array( unsigned char * dest, size_t n, uint32_t const * list );

/**
   Reads n consecutive numbers from src, populating list (which must
   point to at least n uint32_t objects) with the results. Returns the
   number of items read, which will be less than n on error.

   @see whio_decode_uint32()
*/
size_t whio_decode_uint32_array( unsigned char const * src, size_t n, uint32_t * list );

/**
   Encodes a C string into the destination by writing a tag byte, the length of
   the string, and then the string bytes. If n is 0 then n is equivalent to
   strlen(s). Zero is also legal string length.

   Returns the number of bytes written, which will be (n +
   whio_sizeof_encoded_cstring) on success, 0 if !dev or !s.

   dest must be at least (n + whio_sizeof_encoded_cstring) bytes long,
   and on success exactly that many bytes will be written. The null
   terminator (if any) is not stored and not counted in the length.
   s may contain null characters.

   @see whio_decode_cstring()
*/
size_t whio_encode_cstring( unsigned char * dest, char const * s, uint32_t n );

/**
   The converse of whio_encode_cstring(), this routine tries to
   decode a string from the given source memory.

   src must contain at least (whio_sizeof_encoded_cstring + N) bytes,
   where N is the number which is encoded in the first part of the data.
   On success exactly that many bytes will be read from src. The null
   terminator (if any) is not stored and not counted in the length.
   s may contain null characters.

   On success, tgt is assigned to the new (null-terminated) string
   (allocated via calloc()) and length (if it is not null) is set to
   the length of the string (not counting the terminating null). The
   caller must free the string using free(). If the string has a
   length of 0 then tgt is set to 0, not "", and no memory is
   allocated.

   Neither dev nor tgt may be 0, but length may be 0.

   Returns whio_rc.OK on success.

   On error, neither tgt nor length are modified and some non-OK value
   is returned:

   - whio_rc.ArgError = dev or tgt are 0.

   - whio_rc.ConsistencyError = src does not contain a string written
   by whio_encode_cstring().

   Example:

@code
char * str = 0;
size_t len = 0;
int rc = whio_decode_cstring( mySource, &str, &len );
if( whio_rc.OK != rc ) ... error ...
... use str ...
free(str);
@endcode

   @see whio_encode_cstring()
*/
int whio_decode_cstring( unsigned char const * src, char ** tgt, size_t * length );


/**
   Encodes a 32-bit integer value into 5 bytes - a leading tag/check
   byte, then the 4 bytes of the number, in big-endian format. Returns
   the number of bytes written, which will be equal to
   whio_dev_sizeof_uint32 on success.

   @see whio_dev_decode_uint32()
*/
size_t whio_dev_encode_uint32( whio_dev * dev, uint32_t i );

/**
   The converse of whio_dev_encode_uint32(), this tries to read an encoded
   32-bit value from the current position of dev. On success it returns
   whio_rc.OK and sets target to that value. On error it returns some
   other value from whio_rc and does not modify tgt.

   Error values include:

   - whio_rc.ArgError = !dev or !tgt

   - whio_rc.IOError = an error while reading the value (couldn't read enough bytes)

   - whio_rc.ConsistencyError = the bytes at the current location were not encoded
   with whio_dev_encode_uint32().

   @see whio_dev_encode_uint32()

*/
int whio_dev_decode_uint32( whio_dev * dev, uint32_t * tgt );

/**
   Similar to whio_dev_encode_uint32(), with the same conventions, but
   works on 16-bit numbers. Returns the number of bytes written, which
   will be equal to whio_dev_sizeof_uint16 on success.

   @see whio_dev_decode_uint16()
*/
size_t whio_dev_encode_uint16( whio_dev * dev, uint16_t i );

/**
   Similar to whio_dev_decode_uint32(), with the same conventions and
   error codes, but works on 16-bit numbers.  On success it returns
   whio_rc.OK and sets target to that value. On error it returns some
   other value from whio_rc and does not modify tgt.

   @see whio_dev_uint16_encode()
*/

int whio_dev_decode_uint16( whio_dev * dev, uint16_t * tgt );


/**
   The 64-bit variant of whio_dev_encode_uint32(). Follows the same
   conventions as that function but handles a uint64_t value instead
   of uint32_t.

   @see whio_dev_uint16_encode()
   @see whio_dev_encode_uint32()
   @see whio_dev_decode_uint64()
*/
size_t whio_dev_encode_uint64( whio_dev * fs, uint64_t i );

/**
   The 64-bit variant of whio_dev_decode_uint32(). Follows the same
   conventions as that function but handles a uint64_t value instead
   of uint32_t.

   @see whio_dev_decode_uint16()
   @see whio_dev_decode_uint32()
   @see whio_dev_uint64_encode()
*/
int whio_dev_decode_uint64( whio_dev * dev, uint64_t * tgt );

/**
   Uses whio_dev_encode_uint32() to write n elements from the given
   array to dev.  Returns whio_rc.OK on success. Returns the number of
   items written.

   @see whio_dev_encode_uint32()
*/
size_t whio_dev_encode_uint32_array( whio_dev * dev, size_t n, uint32_t const * list );

/**
   Reads n consecutive numbers from dev, populating list (which must
   point to at least n uint32_t objects) with the results. Returns the
   number of items read, which will be less than n on error.

   @see whio_dev_decode_uint32()
*/
size_t whio_dev_decode_uint32_array( whio_dev * dev, size_t n, uint32_t * list );

/**
   Decodes a whio_size_t object from dev. On success whio_rc.OK is returned
   and tgt (if not null) is modified, otherwise tgt is not modified.
*/
int whio_dev_decode_size_t( whio_dev * dev, whio_size_t * tgt );

/**
   Encodes a C string into the device by writing a tag byte, the length of
   the string, and then the string bytes. If n is 0 then n is equivalent to
   strlen(s). Zero is also legal string length.

   Returns the number of bytes written, which will be (n +
   whio_dev_size_cstring) on success, 0 if !dev or !s.

   @see whio_dev_decode_cstring()
*/
uint32_t whio_dev_encode_cstring( whio_dev * dev, char const * s, uint32_t n );

/**
   The converse of whio_dev_encode_cstring(), this routine tries to
   decode a string from the current location in the device.

   On success, tgt is assigned to the new (null-terminated) string
   (allocated via calloc()) and length (if it is not null) is set to
   the length of the string (not counting the terminating null). The
   caller must free the string using free(). If the string has a
   length of 0 then tgt is set to 0, not "", and no memory is
   allocated.

   Neither dev nor tgt may be 0, but length may be 0.

   Returns whio_rc.OK on success.

   On error, neither tgt nor length are modified and some non-OK value
   is returned:

   - whio_rc.ArgError = dev or tgt are 0.

   - whio_rc.ConsistencyError = current position of the device does not
   appear to be an encoded string written by whio_dev_encode_cstring().

   - whio_rc.IOError = some form of IO error.


   Example:

@code
char * str = 0;
size_t len = 0;
int rc = whio_dev_decode_cstring( myDevice, &str, &len );
if( whio_rc.OK != rc ) ... error ...
... use str ...
free(str);
@endcode


   @see whio_dev_encode_cstring()
*/
int whio_dev_decode_cstring( whio_dev * dev, char ** tgt, uint32_t * length );
/**
   Parses fmt in the same manner as whio_encode_pack() and returns
   the number of bytes which would be needed to encode that set
   of data. On error it returns 0, which is never a legal encoding
   length value. If itemCount is not null then it is set to he number
   if objects parsed from the list.

   e.g.

   @code
   size_t itemCount = 0;
   size_t len = whio_encode_pack_calc_size("828",&itemCount);
   // len==24 and itemCount==3
   @endcode

   Note that the encoded length is longer than the actual data because
   each encoded elements gets a consistency-checking byte added to it,
   to allow the decode routines to more safely navigate their input,
   and the number of items in the list is stored in the encoding. The
   size also includes bytes for the packing structure itself, and is
   not a mere accumulation of the types specified in the format
   string.
*/
size_t whio_encode_pack_calc_size( char const * fmt, size_t *itemCount );
    
/**
   Encodes a series of values supported by the various whio_encode_xxx()
   routines as an atomic unit. They can be unpacked by using
   whio_decode_pack().

   fmt must contain only the following characters:

   - '1' means the corresponding (...) arg must be a uint8_t.
   - '2' means the corresponding (...) arg must be a uint16_t.
   - '4' means the corresponding (...) arg must be a uint32_t.
   - '8' means the corresponding (...) arg must be a uint64_t.
   - '+' or '-' followed by a digit means the following number argument is signed.

   
   Returns the result of calling whio_encode_xxx() for each argument,
   where XXX is the type designated by the format specifier.

   If itemsWritten is not null then it is set to the number of items
   successfully written. You can figure out what that number _should_ be
   by using whio_encode_pack_calc_size().
   
   e.g.:

   @code
   size_t count = 0;
   size_t rc = whio_encode_pack( mybuffer, &count, "144", myUint16, myInt32a, myInt32b );
   // count should be 3 now.
   // rc will be the same as whio_encode_pack_calc_size("144",0).
   @endcode

   You can use whio_encode_pack_calc_size() to figure out what the return value of whio_encode_pack()
   _should_ be.
   
   TODOS:

   - Consider adding support for whio_encode_cstring(). The problem
   here is that decoding it requires malloc()ing or some weird
   arg-passing conventions, e.g. fmt="s" requiring two args (char *
   dest,size_t destLen) (which wouldn't be all that bad for the uses i
   have in mind).
*/
size_t whio_encode_pack( void * dest, size_t * itemsWritten, char const * fmt, ... );

/**
   va_list form of whio_encode_pack().
*/
size_t whio_encode_pack_v( void * dest, size_t * itemsWritten, char const * fmt, va_list va );
/**
   This is the converse of whio_encode_pack(), and is used to unpack
   sets packaged using that function. It takes the same format string
   as whio_encode_pack(), but requires pointers to arguments to be
   passed as the variadic arguments. e.g. fmt "1" requires a (uint8_t*)
   argument and fmt "-8" requires an int64_t.

   Returns whefs_rc.OK on success.

   If itemsRead is not null then it will be set ot the number items
   successfully decoded, whether or not an error code is returned.

   You can use whio_decode_pack_calc_size() to figure out how many
   items _should_ be decoded for the given format string.

   Be aware that a data set saved with whio_encode_pack() must be
   decoded with the exact same formatting string. Differences will
   cause the decoding process to return whio_rc.ConsistencyError.
*/
int whio_decode_pack( void const * src, size_t * itemsRead, char const * fmt, ... );

/**
   va_list form of whio_deencode_pack().
*/
int whio_decode_pack_v( void const * src, size_t * itemsRead, char const * fmt, va_list va );

/**
   Parses a whio_encode_pack()-formatted string and figures out the size of the
   encoded data. If itemCount is not 0 then it is set to the number of items
   parsed from fmt. On error, 0 is returned but itemCount will contain the number
   of values parsed before the error.
*/
size_t whio_encode_pack_calc_size( char const * fmt, size_t *itemCount );
    
#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHIO_ENCODE_H_INCLUDED */
