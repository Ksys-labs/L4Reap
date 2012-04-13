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
#include <wh/whio/whio_encode.h>
#include <wh/whio/whio_common.h> /* only for whio_rc */
#include <stdlib.h> /* calloc() */
#include <string.h> /* memset() */
static const unsigned char whio_uint64_tag_char = 0x80 | 64;
size_t whio_encode_uint64( unsigned char * dest, uint64_t i )
{
    if( ! dest ) return 0;
    static const uint64_t mask = UINT64_C(0x00ff);
    size_t x = 0;
    dest[x++] = whio_uint64_tag_char;
    dest[x++] = (unsigned char)((i>>56) & mask);
    dest[x++] = (unsigned char)((i>>48) & mask);
    dest[x++] = (unsigned char)((i>>40) & mask);
    dest[x++] = (unsigned char)((i>>32) & mask);
    dest[x++] = (unsigned char)((i>>24) & mask);
    dest[x++] = (unsigned char)((i>>16) & mask);
    dest[x++] = (unsigned char)((i>>8) & mask);
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_uint64;
}


int whio_decode_uint64( unsigned char const * src, uint64_t * tgt )
{
    if( ! src || ! tgt ) return whio_rc.ArgError;
    if( whio_uint64_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
#define SHIFT(X) ((uint64_t)src[X])
    uint64_t val = (SHIFT(1) << UINT64_C(56))
	+ (SHIFT(2) << UINT64_C(48))
	+ (SHIFT(3) << UINT64_C(40))
	+ (SHIFT(4) << UINT64_C(32))
	+ (SHIFT(5) << UINT64_C(24))
	+ (SHIFT(6) << UINT64_C(16))
	+ (SHIFT(7) << UINT64_C(8))
	+ (SHIFT(8));
#undef SHIFT
    *tgt = val;
    return whio_rc.OK;
}


static const unsigned char whio_int64_tag_char = 0x80 | 65;
size_t whio_encode_int64( unsigned char * dest, int64_t i )
{
    if( ! dest ) return 0;
    static const int64_t mask = INT64_C(0x00ff);
    size_t x = 0;
    dest[x++] = whio_int64_tag_char;
    dest[x++] = (unsigned char)((i>>56) & mask);
    dest[x++] = (unsigned char)((i>>48) & mask);
    dest[x++] = (unsigned char)((i>>40) & mask);
    dest[x++] = (unsigned char)((i>>32) & mask);
    dest[x++] = (unsigned char)((i>>24) & mask);
    dest[x++] = (unsigned char)((i>>16) & mask);
    dest[x++] = (unsigned char)((i>>8) & mask);
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_int64;
}


int whio_decode_int64( unsigned char const * src, int64_t * tgt )
{
    if( ! src || ! tgt ) return whio_rc.ArgError;
    if( whio_int64_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
#define SHIFT(X) ((int64_t)src[X])
    int64_t val = (SHIFT(1) << INT64_C(56))
	+ (SHIFT(2) << INT64_C(48))
	+ (SHIFT(3) << INT64_C(40))
	+ (SHIFT(4) << INT64_C(32))
	+ (SHIFT(5) << INT64_C(24))
	+ (SHIFT(6) << INT64_C(16))
	+ (SHIFT(7) << INT64_C(8))
	+ (SHIFT(8));
#undef SHIFT
    *tgt = val;
    return whio_rc.OK;
}


static const unsigned char whio_uint32_tag_char = 0x80 | 32;
size_t whio_encode_uint32( unsigned char * dest, uint32_t i )
{
    if( ! dest ) return 0;
    static const unsigned short mask = 0x00ff;
    size_t x = 0;
    /** We tag all entries with a prefix mainly to simplify debugging
	of the vfs files (it's easy to spot them in a file viewer),
	but it incidentally also gives us a sanity-checker at
	read-time (we simply confirm that the first byte is this
	prefix).
    */
    dest[x++] = whio_uint32_tag_char;
    dest[x++] = (unsigned char)(i>>24) & mask;
    dest[x++] = (unsigned char)(i>>16) & mask;
    dest[x++] = (unsigned char)(i>>8) & mask;
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_uint32;
}

int whio_decode_uint32( unsigned char const * src, uint32_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_uint32_tag_char != src[0] )
    {
	//WHIO_DEBUG("read bytes are not an encoded integer value!\n");
	return whio_rc.ConsistencyError;
    }
    uint32_t val = (src[1] << 24)
	+ (src[2] << 16)
	+ (src[3] << 8)
	+ (src[4]);
    if( tgt ) *tgt = val;
    return whio_rc.OK;
}

static const unsigned char whio_int32_tag_char = 0x80 | 33;
size_t whio_encode_int32( unsigned char * dest, int32_t i )
{
    if( ! dest ) return 0;
    static const unsigned short mask = 0x00ff;
    size_t x = 0;
    /** We tag all entries with a prefix mainly to simplify debugging
	of the vfs files (it's easy to spot them in a file viewer),
	but it incidentally also gives us a sanity-checker at
	read-time (we simply confirm that the first byte is this
	prefix).
    */
    dest[x++] = whio_int32_tag_char;
    dest[x++] = (unsigned char)(i>>24) & mask;
    dest[x++] = (unsigned char)(i>>16) & mask;
    dest[x++] = (unsigned char)(i>>8) & mask;
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_int32;
}

int whio_decode_int32( unsigned char const * src, int32_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_int32_tag_char != src[0] )
    {
	//WHIO_DEBUG("read bytes are not an encoded integer value!\n");
	return whio_rc.ConsistencyError;
    }
    int32_t val = (src[1] << 24)
	+ (src[2] << 16)
	+ (src[3] << 8)
	+ (src[4]);
    if( tgt ) *tgt = val;
    return whio_rc.OK;
}


static const unsigned char whio_uint16_tag_char = 0x80 | 16;
size_t whio_encode_uint16( unsigned char * dest, uint16_t i )
{
    if( ! dest ) return 0;
    static const uint16_t mask = 0x00ff;
    uint8_t x = 0;
    dest[x++] = whio_uint16_tag_char;
    dest[x++] = (unsigned char)(i>>8) & mask;
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_uint16;
}

int whio_decode_uint16( unsigned char const * src, uint16_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_uint16_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
    uint16_t val = + (src[1] << 8)
	+ (src[2]);
    *tgt = val;
    return whio_rc.OK;
}

static const unsigned char whio_int16_tag_char = 0x80 | 17;
size_t whio_encode_int16( unsigned char * dest, int16_t i )
{
    if( ! dest ) return 0;
    static const int16_t mask = 0x00ff;
    int8_t x = 0;
    dest[x++] = whio_int16_tag_char;
    dest[x++] = (unsigned char)(i>>8) & mask;
    dest[x++] = (unsigned char)(i & mask);
    return whio_sizeof_encoded_int16;
}

int whio_decode_int16( unsigned char const * src, int16_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_int16_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
    int16_t val = + (src[1] << 8)
	+ (src[2]);
    *tgt = val;
    return whio_rc.OK;
}


static const unsigned char whio_uint8_tag_char = 0x80 | 8;
size_t whio_encode_uint8( unsigned char * dest, uint8_t i )
{
    if( ! dest ) return 0;
    dest[0] = whio_uint8_tag_char;
    dest[1] = i;
    return whio_sizeof_encoded_uint8;
}

int whio_decode_uint8( unsigned char const * src, uint8_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_uint8_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
    *tgt = src[1];
    return whio_rc.OK;
}

static const unsigned char whio_int8_tag_char = 0x80 | 8;
size_t whio_encode_int8( unsigned char * dest, int8_t i )
{
    if( ! dest ) return 0;
    dest[0] = whio_int8_tag_char;
    dest[1] = i;
    return whio_sizeof_encoded_int8;
}

int whio_decode_int8( unsigned char const * src, int8_t * tgt )
{
    if( ! src ) return whio_rc.ArgError;
    if( whio_int8_tag_char != src[0] )
    {
	return whio_rc.ConsistencyError;
    }
    *tgt = src[1];
    return whio_rc.OK;
}


size_t whio_encode_uint32_array( unsigned char * dest, size_t n, uint32_t const * list )
{
    size_t i = (dest && n && list) ? 0 : n;
    size_t rc = 0;
    for( ; i < n; ++i, ++rc )
    {
	if( whio_sizeof_encoded_uint32 != whio_encode_uint32( dest, *(list++) ) )
	{
	    break;
	}
    }
    return rc;
}

size_t whio_decode_uint32_array( unsigned char const * src, size_t n, uint32_t * list )
{
    size_t i = (src && n && list) ? 0 : n;
    size_t rc = 0;
    for( ; i < n; ++i, ++rc )
    {
	if( whio_rc.OK != whio_decode_uint32( src, &list[i] ) )
	{
	    break;
	}
    }
    return rc;
}

static const unsigned char whio_cstring_tag_char = 0x80 | '"';
size_t whio_encode_cstring( unsigned char * dest, char const * s, uint32_t n )
{
    if( ! dest || !s ) return 0;
    if( ! n )
    {
	char const * x = s;
	loop: if( x && *x ) { ++x; ++n; goto loop; }
	//for( ; x && *x ; ++x, ++n ){}
    }
    *(dest++) = whio_cstring_tag_char;
    size_t rc = whio_encode_uint32( dest, n );
    if( whio_sizeof_encoded_uint32 != rc ) return rc;
    dest += rc;
    rc = 1 + whio_sizeof_encoded_uint32;
    size_t i = 0;
    for( ; i < n; ++i, ++rc )
    {
	*(dest++) = *(s++);
    }
    *dest = 0;
    return rc;
}

int whio_decode_cstring( unsigned char const * src, char ** tgt, size_t * length )
{
    if( !src || ! tgt ) return whio_rc.ArgError;

    if( whio_cstring_tag_char != *src )
    {
	return whio_rc.ConsistencyError;
    }
    ++src;
    uint32_t slen = 0;
    int rc = whio_decode_uint32( src, &slen );
    if( whio_rc.OK != rc ) return rc;
    if( ! slen )
    {
	*tgt = 0;
	if(length) *length = 0;
	return whio_rc.OK;
    }
    char * buf = (char *)calloc( slen + 1, sizeof(char) );
    if( ! buf ) return whio_rc.AllocError;
    if( length ) *length = slen;
    *tgt = buf;
    size_t i = 0;
    src += whio_sizeof_encoded_uint32;
    for(  ; i < slen; ++i )
    {
	*(buf++) = *(src++);
    }
    *buf = 0;
    return whio_rc.OK;
}

/**
   tag byte for encoded whio_id_type objects.
*/
static const unsigned int whio_size_t_tag_char = 0x08 | 'p';
whio_size_t whio_encode_size_t( unsigned char * dest, whio_size_t v )
{
#if WHIO_SIZE_T_BITS == 64
    return whio_encode_uint64( dest, v );
#elif WHIO_SIZE_T_BITS == 32
    return whio_encode_uint32( dest, v );
#elif WHIO_SIZE_T_BITS == 16
    return whio_encode_uint16( dest, v );
#elif WHIO_SIZE_T_BITS == 8
    return whio_encode_uint8( dest, v );
#else
#error "whio_size_t size (WHIO_SIZE_T_BITS) is not supported!"
#endif
}

int whio_decode_size_t( unsigned char const * src, whio_size_t * v )
{
#if WHIO_SIZE_T_BITS == 64
    return whio_decode_uint64( src, v );
#elif WHIO_SIZE_T_BITS == 32
    return whio_decode_uint32( src, v );
#elif WHIO_SIZE_T_BITS == 16
    return whio_decode_uint16( src, v );
#elif WHIO_SIZE_T_BITS == 8
    return whio_decode_uint8( src, v );
#else
#error "whio_size_t is not a supported type!"
#endif
}

whio_size_t whio_dev_encode_size_t( whio_dev * dev, whio_size_t v )
{
    unsigned char buf[whio_sizeof_encoded_size_t];
    whio_encode_size_t( buf, v );
    return dev->api->write( dev, &buf, whio_sizeof_encoded_size_t );
}

int whio_dev_decode_size_t( whio_dev * dev, whio_size_t * tgt )
{
    unsigned char buf[whio_sizeof_encoded_size_t]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_size_t is the correct size. */
    memset( buf, 0, whio_sizeof_encoded_size_t );
    size_t rc = dev->api->read( dev, buf  /*Flawfinder: ignore  This is safe in conjunction with whio_sizeof_encoded_xxx*/, whio_sizeof_encoded_size_t );
    return ( whio_sizeof_encoded_size_t == rc )
        ? whio_decode_size_t( buf, tgt )
        : whio_rc.IOError;
}


size_t whio_dev_encode_uint64( whio_dev * dev, uint64_t i )
{
    unsigned char buf[whio_sizeof_encoded_uint64]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint64 is the correct size. */
    size_t const x = whio_encode_uint64( buf, i );
    return ( whio_sizeof_encoded_uint64 == x )
        ? whio_dev_write( dev, buf, whio_sizeof_encoded_uint64 )
        : 0;
}

int whio_dev_decode_uint64( whio_dev * dev, uint64_t * tgt )
{
    if( ! dev || ! tgt ) return whio_rc.ArgError;
    unsigned char buf[whio_sizeof_encoded_uint64]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint64 is the correct size. */
    memset( buf, 0, whio_sizeof_encoded_uint64 );
    size_t rc = dev->api->read( dev, buf  /*Flawfinder: ignore  This is safe in conjunction with whio_sizeof_encoded_uint64*/, whio_sizeof_encoded_uint64 );
    return ( whio_sizeof_encoded_uint64 == rc )
        ? whio_decode_uint64( buf, tgt )
        : whio_rc.IOError;
}


size_t whio_dev_encode_uint32( whio_dev * dev, uint32_t i )
{
    unsigned char buf[whio_sizeof_encoded_uint32]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint32 is the correct size. */
    size_t const x = whio_encode_uint32( buf, i );
    return ( whio_sizeof_encoded_uint32 == x )
        ? whio_dev_write( dev, buf, whio_sizeof_encoded_uint32 )
        : 0;
}

int whio_dev_decode_uint32( whio_dev * dev, uint32_t * tgt )
{
    if( ! dev || ! tgt ) return whio_rc.ArgError;
    unsigned char buf[whio_sizeof_encoded_uint32]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint32 is the correct size. */
    memset( buf, 0, whio_sizeof_encoded_uint32 );
    size_t rc = dev->api->read( dev, buf  /*Flawfinder: ignore  This is safe in conjunction with whio_sizeof_encoded_uint32*/, whio_sizeof_encoded_uint32 );
    return ( whio_sizeof_encoded_uint32 == rc )
        ? whio_decode_uint32( buf, tgt )
        : whio_rc.IOError;
}


static const unsigned char whio_dev_uint16_tag_char = 0x80 | 16;
size_t whio_dev_encode_uint16( whio_dev * dev, uint16_t i )
{
    unsigned char buf[whio_sizeof_encoded_uint16]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint16 is the correct size. */
    size_t const x = whio_encode_uint16( buf, i );
    return ( whio_sizeof_encoded_uint16 == x )
        ? whio_dev_write( dev, buf, whio_sizeof_encoded_uint16 )
        : 0;
}

int whio_dev_decode_uint16( whio_dev * dev, uint16_t * tgt )
{
    if( ! dev || ! tgt ) return whio_rc.ArgError;
    unsigned char buf[whio_sizeof_encoded_uint16]; /* Flawfinder: ignore This is intentional and safe as long as whio_sizeof_encoded_uint16 is the correct size. */
    memset( buf, 0, whio_sizeof_encoded_uint16 );
    size_t rc = dev->api->read( dev, buf  /*Flawfinder: ignore  This is safe in conjunction with whio_sizeof_encoded_uint16*/, whio_sizeof_encoded_uint16 );
    return ( whio_sizeof_encoded_uint16 == rc )
        ? whio_decode_uint16( buf, tgt )
        : whio_rc.IOError;
}

size_t whio_dev_encode_uint32_array( whio_dev * dev, size_t n, uint32_t const * list )
{
    size_t i = (dev && n && list) ? 0 : n;
    size_t rc = 0;
    for( ; i < n; ++i, ++rc )
    {
	if( whio_sizeof_encoded_uint32 != whio_dev_encode_uint32( dev, *(list++) ) )
	{
	    break;
	}
    }
    return rc;
}

size_t whio_dev_decode_uint32_array( whio_dev * dev, size_t n, uint32_t * list )
{
    size_t i = (dev && n && list) ? 0 : n;
    size_t rc = 0;
    for( ; i < n; ++i, ++rc )
    {
	if( whio_rc.OK != whio_dev_decode_uint32( dev, &list[i] ) )
	{
	    break;
	}
    }
    return rc;
}

static const unsigned char whio_dev_cstring_tag_char = 0x80 | '"';
whio_size_t whio_dev_encode_cstring( whio_dev * dev, char const * s, uint32_t n )
{
    if( ! dev || !s ) return 0;
    if( ! n )
    {
	char const * x = s;
	loop: if( x && *x ) { ++x; ++n; goto loop; }
	//for( ; x && *x ; ++x, ++n ){}
    }
    whio_size_t rc = dev->api->write( dev, &whio_dev_cstring_tag_char, 1 );
    if( 1 != rc ) return rc;
    rc += whio_dev_encode_uint32( dev, n );
    if( (1 + whio_sizeof_encoded_uint32) != rc ) return rc;
    return rc + dev->api->write( dev, s, (whio_size_t)n );
}

int whio_dev_decode_cstring( whio_dev * dev, char ** tgt, uint32_t * length )
{
    if( !dev || ! tgt ) return whio_rc.ArgError;

    { /* check tag byte */
	unsigned char tagbuf[1] = {0}; /* Flawfinder: ignore  This is intended and safe. */
	whio_size_t const sz = dev->api->read( dev, tagbuf, 1 );/*Flawfinder: ignore  This is safe used safely.*/
	if( (1 != sz) || (whio_dev_cstring_tag_char != tagbuf[0]) )
	{
	    return sz ? whio_rc.ConsistencyError : whio_rc.IOError;
	}
    }
    uint32_t slen = 0;
    int rc = whio_dev_decode_uint32( dev, &slen );
    if( whio_rc.OK != rc ) return rc;
    if( ! slen )
    {
	*tgt = 0;
	if(length) *length = 0;
	return whio_rc.OK;
    }
    char * buf = (char *)calloc( slen + 1, sizeof(char) );
    if( ! buf ) return whio_rc.AllocError;
    if( slen != dev->api->read( dev, buf /*Flawfinder: ignore  This is safe in conjunction with slen. See above. */, slen ) )
    {
	free( buf );
	return whio_rc.IOError;
    }
    *tgt = buf;
    if( length ) *length = slen;
    return whio_rc.OK;
}

static const unsigned char whio_encode_pack_tag = 0xF0 | 'P';
size_t whio_encode_pack_calc_size( char const * fmt, size_t *itemCount )
{
    if( ! fmt || !*fmt ) return 0;
    char const * at = fmt;
    size_t rc = 1 + whio_sizeof_encoded_uint8;
    size_t exp = 0;
    for( ;at && *at; ++at )
    {
        exp =0;
        switch( *at )
        {
          case ' ':
          case '+':
          case '-': continue;
          case '1': exp = whio_sizeof_encoded_uint8;
              break;
          case '2': exp = whio_sizeof_encoded_uint16;
              break;
          case '4': exp = whio_sizeof_encoded_uint32;
              break;
          case '8': exp = whio_sizeof_encoded_uint64;
              break;
          default: return 0;
        }
        if( ! exp ) return 0;
        if( itemCount ) ++(*itemCount);
        rc += exp;
    }
    return rc;
}

size_t whio_encode_pack( void * dest, size_t * itemsWritten, char const * fmt, ... )
{
    va_list va;
    size_t rc = 0;
    va_start(va,fmt);
    rc = whio_encode_pack_v( dest, itemsWritten, fmt, va );
    va_end(va);
    return rc;
}

size_t whio_encode_pack_v( void * dest_, size_t * itemCount, char const * fmt, va_list va )
{

    if( ! fmt || !dest_ ) return 0;
    size_t rc = 0;
    size_t shouldItems = 0;
    size_t const shouldBytes = whio_encode_pack_calc_size( fmt, &shouldItems );
    unsigned char * dest = (unsigned char *)dest_;
    char const * at = fmt;
    *(dest++) = whio_encode_pack_tag;
    unsigned char * countPos = dest;
    dest += whio_sizeof_encoded_uint8;
    rc = (dest - (unsigned char const *)dest_);
    bool isSigned = false;
    size_t ck;
    size_t count = 0;
    size_t exp = 0;
    for( ;at && *at; ++at )
    {
        ck = 0;
        exp = 0;
        switch( *at )
        {
          case ' ': continue;
          case '+':
          case '-': isSigned = true;
              continue;
#define CASE(TAG,STYPE,UTYPE) if( isSigned ) {       \
                  isSigned = false; \
                  ck = whio_encode_##TAG( dest, va_arg(va,STYPE)); \
                  exp = whio_sizeof_encoded_##TAG; \
              } else { \
                  ck = whio_encode_u##TAG( dest, va_arg(va,UTYPE));  \
                  exp = whio_sizeof_encoded_u##TAG;                     \
              } break
          case '1':
              CASE(int8,int,int);
              // ^^^ gcc complaints about type promotion if i pass a uint8_t there.
              // It threatens to abort() if the code is run that way.
          case '2':
              CASE(int16,int,int); // <<--- see uint8_t notes above
          case '4':
              CASE(int32,int32_t,uint32_t);
          case '8':
              CASE(int64,int64_t,uint64_t);
              break;
          default: continue;
#undef CASE
        }
        if( ! ck || (ck != exp) ) return rc;
        ++count;
        rc += ck;
        dest += ck;
    }
    if( itemCount ) *itemCount = count;
    if( shouldBytes == rc )
    {
        whio_encode_uint8( countPos, count );
    }
    return rc;
}


int whio_decode_pack( void const * src, size_t * itemCount, char const * fmt, ... )
{
    va_list va;
    int rc = 0;
    va_start(va,fmt);
    rc = whio_decode_pack_v( src, itemCount, fmt, va );
    va_end(va);
    return rc;
}

int whio_decode_pack_v( void const * src_, size_t * itemCount, char const * fmt, va_list va )
{

    if( ! fmt || !src_ ) return 0;
    int rc = 0;
    size_t shouldItems = 0;
    size_t const shouldBytes = whio_encode_pack_calc_size( fmt, &shouldItems );
    unsigned char const * src = (unsigned char const *)src_;
    char const * at = fmt;
    if( *(src++) != whio_encode_pack_tag )
    {
        return whio_rc.ConsistencyError;
    }
    uint8_t shouldPackedItems = 0;
    rc = whio_decode_uint8( src, &shouldPackedItems );
    if( (whio_rc.OK != rc) || !shouldPackedItems )
    {
        return whio_rc.ConsistencyError;
    }
    src += whio_sizeof_encoded_uint8;
    bool isSigned = false;
    size_t count = 0;
    size_t exp = 0;
    for( ;at && *at; ++at )
    {
        rc = 0;
        exp = 0;
        switch( *at )
        {
          case ' ': continue;
          case '+':
          case '-': isSigned = true;
              continue;
#define CASE(TAG) if( isSigned ) {       \
                  isSigned = false; \
                  rc = whio_decode_##TAG( src, va_arg(va,TAG##_t*)); \
                  exp = whio_sizeof_encoded_##TAG; \
              } else { \
                  rc = whio_decode_u##TAG( src, va_arg(va,u##TAG##_t*));  \
                  exp = whio_sizeof_encoded_u##TAG;                     \
              } break
          case '1':
              CASE(int8);
          case '2':
              CASE(int16);
          case '4':
              CASE(int32);
          case '8':
              CASE(int64);
              break;
          default: continue;
#undef CASE
        }
        if( rc != whio_rc.OK ) return rc;
        if( ! exp ) return whio_rc.RangeError;
        ++count;
        src += exp;
    }
    if( itemCount ) *itemCount = count;
    if( whio_rc.OK == rc )
    {
        if( shouldBytes != (src-(unsigned char const*)src_) )
        {
            rc = whio_rc.ConsistencyError;
        }
        else if( shouldPackedItems != count )
        {
            rc = whio_rc.ConsistencyError;
        }
    }
    return rc;
}
