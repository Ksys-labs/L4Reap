/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/

/* Note from linux stdint.h:

   The ISO C99 standard specifies that in C++ implementations these
   should only be defined if explicitly requested. 
*/
#if defined __cplusplus && ! defined __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
#endif
#include "whefs_encode.h"
#include <wh/whefs/whefs.h> /* only for whefs_rc */
#include <wh/whio/whio_encode.h>
#include <stdlib.h> /* calloc() */

uint64_t whefs_bytes_hash( void const * data, uint32_t len )
{
    /**
       One-at-a-time hash code taken from:

       http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
    */
    if( ! data || !len ) return 0;
    unsigned const char *p = data;
    uint64_t h = 0;
    uint32_t i;
    for ( i = 0; i < len; i++ )
    {
	h += p[i];
	h += ( h << 10 );
	h ^= ( h >> 6 );
    }
    h += ( h << 3 );
    h ^= ( h >> 11 );
    h += ( h << 15 );
    return h;
}

/**
   tag byte for encoded whefs_id_type objects.
*/
static const unsigned int whefs_id_type_tag_char = 0x08 | 8;
size_t whefs_dev_id_encode( whio_dev * dev, whefs_id_type v )
{
#if WHEFS_ID_TYPE_BITS == 64
    return whio_dev_encode_uint64( dev, v );
#elif WHEFS_ID_TYPE_BITS == 32
    return whio_dev_encode_uint32( dev, v );
#elif WHEFS_ID_TYPE_BITS == 16
    return whio_dev_encode_uint16( dev, v );
#elif WHEFS_ID_TYPE_BITS == 8
    if( ! dev ) return whefs_rc.ArgError;
    unsigned char buf[2];
    buf[0] = whefs_id_type_tag_char;
    buf[1] = v;
    return dev->api->write( dev, buf, 2 );
#else
#error "whefs_id_type size (WHEFS_ID_TYPE_BITS) is not supported!"
#endif
}

size_t whefs_id_encode( unsigned char * dest, whefs_id_type v )
{
#if WHEFS_ID_TYPE_BITS == 64
    return whio_encode_uint64( dest, v );
#elif WHEFS_ID_TYPE_BITS == 32
    return whio_encode_uint32( dest, v );
#elif WHEFS_ID_TYPE_BITS == 16
    return whio_encode_uint16( dest, v );
#elif WHEFS_ID_TYPE_BITS == 8
    if( ! dest ) return whefs_rc.ArgError;
    dest[0] = whefs_id_type_tag_char;
    dest[1] = v;
    return whefs_sizeof_encoded_id_type;
#else
#error "whefs_id_type size (WHEFS_ID_TYPE_BITS) is not supported!"
#endif
}

int whefs_dev_id_decode( whio_dev * dev, whefs_id_type * v )
{
#if WHEFS_ID_TYPE_BITS == 64
    return whio_dev_decode_uint64( dev, v );
#elif WHEFS_ID_TYPE_BITS == 32
    return whio_dev_decode_uint32( dev, v );
#elif WHEFS_ID_TYPE_BITS == 16
    return whio_dev_decode_uint16( dev, v );
#elif WHEFS_ID_TYPE_BITS == 8
    if( ! v || ! dev ) return whefs_rc.ArgError;
    unsigned char buf[2] = {0,0};
    size_t sz = dev->api->read( dev, buf, 2 );
    if( 2 != sz)
    {
	return whefs_rc.IOError;
    }
    else if( buf[0] != whefs_id_type_tag_char )
    {
	return whefs_rc.ConsistencyError;
    }
    else
    {
	*v = buf[1];
	return whefs_rc.OK;
    }
#else
#error "whefs_id_type is not a supported type!"
#endif
}

int whefs_id_decode( unsigned char const * src, whefs_id_type * v )
{
#if WHEFS_ID_TYPE_BITS == 64
    return whio_decode_uint64( src, v );
#elif WHEFS_ID_TYPE_BITS == 32
    return whio_decode_uint32( src, v );
#elif WHEFS_ID_TYPE_BITS == 16
    return whio_decode_uint16( src, v );
#elif WHEFS_ID_TYPE_BITS == 8
    if( ! src ) return whefs_rc.ArgError;
    else if( src[0] != whefs_id_type_tag_char )
    {
	return whefs_rc.ConsistencyError;
    }
    else
    {
	if( v ) *v = src[1];
    }
    return whefs_rc.OK;
#else
#error "whefs_id_type is not a supported type!"
#endif
}

