/*
  Implementations for whio gzip support.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#include <wh/whio/whio_zlib.h>
#include <wh/whio/whio_config.h>

#if WHIO_ENABLE_ZLIB
#include <zlib.h>
#endif /* WHIO_ENABLE_ZLIB */

int whio_stream_gzip( whio_stream * restrict src, whio_stream * restrict dest, int level )
{
#if ! WHIO_ENABLE_ZLIB
    return whio_rc.UnsupportedError;
#else
    if( !src || !dest || (src == dest) ) return whio_rc.ArgError;
    if( level != Z_DEFAULT_COMPRESSION )
    {
	if( level < Z_NO_COMPRESSION ) level = Z_NO_COMPRESSION;
	else if (level > Z_BEST_COMPRESSION) level = Z_BEST_COMPRESSION;
    }
    /* Code taken 99% from http://zlib.net/zlib_how.html */
    int ret;
    int flush;
    size_t have;
    z_stream strm;
    enum { bufSize  = 1024 * 8 };
    unsigned char in[bufSize];
    unsigned char out[bufSize];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = //deflateInit(&strm, level)
	deflateInit2( &strm, level, Z_DEFLATED, 16+MAX_WBITS, 8, Z_DEFAULT_STRATEGY )
	;
    if (ret != Z_OK)
    {
	WHIO_DEBUG("defaultInit() failed with rc %d\n", ret );
        return ret;
    }

    /* compress until end of file */
    do
    {
	size_t iosize = src->api->read( src, in, bufSize );
	strm.avail_in = iosize;
        if( ! src->api->isgood(src)  )
	{
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = (iosize < bufSize) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;
	/* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do
	{
	    strm.avail_out = bufSize;
            strm.next_out = out;
	    ret = deflate(&strm, flush);    /* no bad return value */
	    if( Z_STREAM_ERROR == ret )
	    {
		WHIO_DEBUG("deflate() returned Z_STREAM_ERROR!\n");
                (void)deflateEnd(&strm);
		return Z_STREAM_ERROR;
	    }
	    have = bufSize - strm.avail_out;
	    if( have )
	    {
		iosize = dest->api->write( dest, out, have );
		if( (iosize != have)
		    || !dest->api->isgood(dest) )
		{
		    WHIO_DEBUG("Write of %u bytes failed - wrote only %u.\n", have, iosize );
		    (void)deflateEnd(&strm);
		    return Z_ERRNO;
		}
            }
	} while (strm.avail_out == 0);
	if( strm.avail_in != 0)
	{
	    WHIO_DEBUG("Not all input was consumed! %u byte(s) remain!", strm.avail_in );
	    (void)deflateEnd(&strm);
	}
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    //assert(ret == Z_STREAM_END);        /* stream will be complete */
    /* clean up and return */
    (void)deflateEnd(&strm);
    return (ret == Z_STREAM_END) ? Z_OK : ret;
#endif /* WHIO_ENABLE_ZLIB */

}


int whio_stream_gunzip( whio_stream * restrict src, whio_stream * restrict dest )
{
#if ! WHIO_ENABLE_ZLIB
    return whio_rc.UnsupportedError;
#else
    if( !src || !dest || (src == dest) ) return whio_rc.ArgError;
    /* Code taken 99% from http://zlib.net/zlib_how.html */
    int ret;
    size_t have;
    z_stream strm;
    enum { bufSize  = 1024 * 8 };
    unsigned char in[bufSize];
    unsigned char out[bufSize];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = //inflateInit( &strm )
	inflateInit2( &strm, 16+MAX_WBITS )
	;
    if (ret != Z_OK)
    {
	WHIO_DEBUG("Initialization of z_stream failed with rc %d!\n", ret );
        return ret;
    }
    do
    {
	size_t iosize = src->api->read( src, in, bufSize );
	strm.avail_in = iosize;
	if( ! src->api->isgood( src ) )
	{
            (void)inflateEnd( &strm );
	    WHIO_DEBUG("!src->isgood()!\n" );
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        /* run inflate() on input until output buffer not full */
        do
	{
            strm.avail_out = bufSize;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret)
	    {
	      case Z_NEED_DICT:
		  WHIO_DEBUG("inflate() says Z_NEED_DICT\n");
		  ret = Z_DATA_ERROR; /* and fall through */
	      case Z_STREAM_ERROR:
		  WHIO_DEBUG("Z_STREAM_ERROR\n");
	      case Z_DATA_ERROR:
		  WHIO_DEBUG("Z_DATA_ERROR\n");
	      case Z_MEM_ERROR:
		  WHIO_DEBUG("inflate() returned unwanted value %d!\n", ret );
		  (void)inflateEnd(&strm);
		  return ret;
	      default:
		  break;
            }
            have = bufSize - strm.avail_out;
	    if( have )
	    {
		iosize = dest->api->write( dest, out, have );
		if ( (iosize != have)
		     || !dest->api->isgood(dest) )
		{
		    WHIO_DEBUG("write failed or !dest->isgood()! Wrote %u of %u bytes?\n", iosize, have );
		    (void)inflateEnd(&strm);
		    return Z_ERRNO;
		}
	    }
	} while (strm.avail_out == 0);
	/* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    (void)inflateEnd( &strm );
    return (ret == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;
#endif /* WHIO_ENABLE_ZLIB */
}
