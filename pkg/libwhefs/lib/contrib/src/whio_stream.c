#include <stdlib.h> /* free/malloc */

#include <wh/whio/whio.h>
#include <wh/whio/whio_stream.h>
#include <wh/whprintf.h>


#ifdef __cplusplus
#define ARG_UNUSED(X)
extern "C" {
#else
#define ARG_UNUSED(X) X
#endif

bool whio_stream_default_isgood( whio_stream * ARG_UNUSED(self) )
{
    return false;
}

whio_size_t whio_stream_default_read( whio_stream * ARG_UNUSED(self),
				 void * ARG_UNUSED(dest),
				 whio_size_t ARG_UNUSED(max) )
{
    return 0;
}

whio_size_t whio_stream_default_write( whio_stream * ARG_UNUSED(self),
				void const * ARG_UNUSED(src),
				whio_size_t ARG_UNUSED(len) )
{
    return 0;
}

int whio_stream_default_flush( whio_stream * ARG_UNUSED(self) )
{
    return whio_rc.ArgError;
}

bool whio_stream_default_close( whio_stream * ARG_UNUSED(self) )
{
    if( self->client.dtor ) self->client.dtor( self->client.data );
    self->client = whio_client_data_empty;
    return false;
}

void whio_stream_default_finalize( whio_stream * ARG_UNUSED(self) )
{
    if(self)
    {
	self->api->close( self );
	//C11N_LOGME_DEALLOCT(whio_stream);
	free(self);
    }
}

const whio_stream_api whio_stream_api_empty = 
    {
    whio_stream_default_read,
    whio_stream_default_write,
    whio_stream_default_close,
    whio_stream_default_finalize,
    whio_stream_default_flush,
    whio_stream_default_isgood
    };

const whio_stream whio_stream_empty = 
    {
    &whio_stream_api_empty,
    whio_impl_data_empty_m,
    whio_client_data_empty_m
    };

bool whio_stream_getchar( whio_stream * self, char * tgt )
{
    if( ! self ) return false;
    char x = 0;
    if( 1 != self->api->read( self, &x, 1 ) ) return false;
    if( tgt ) *tgt = x;
    return true;
}


/** @implements whprintf_appender

   gprintf_appender implementation which appends all input to
   a whio_stream. Requires arg to be-a whio_stream. n bytes from
   the data argument are written to that stream. On success, the number
   of bytes written is returned.
*/
static long whio_stream_printf_appender( void * arg, char const * data, long n )
{
    if( ! arg || !data ) return -1;
    whio_size_t sz = n;
    if( n < sz ) return -1; /* negative n */
    whio_stream * str = (whio_stream*)arg;
    sz = str->api->write( str, data, sz );
    return (sz == whio_rc.SizeTError) ? 0 : (long) sz; // FIXME: check for overflow!
}

whio_size_t whio_stream_writefv( whio_stream * str, const char *fmt, va_list ap )
{
    long rc = whprintfv( whio_stream_printf_appender, str, fmt, ap );
    return (rc < 0) ? 0 : (whio_size_t)rc;
}

whio_size_t whio_stream_writef( whio_stream * str, const char *fmt, ... )
{
    va_list vargs;
    va_start( vargs, fmt );
    whio_size_t rc = whio_stream_writefv( str, fmt, vargs );
    va_end(vargs);
    return rc;
}

int whio_stream_copy( whio_stream * restrict istr, whio_stream * restrict ostr )
{
    if( istr == ostr ) return 0;
    whio_size_t rdrc = 0;
    whio_size_t wrc = 0;
    enum { bufSize = 1024 * 4 };
    unsigned char buf[bufSize];
    do
    {
        rdrc = istr->api->read( istr, buf, bufSize );
        if( ! rdrc ) return whio_rc.IOError;
        wrc = ostr->api->write( ostr, buf, rdrc );
        if( rdrc != wrc ) return whio_rc.IOError;
    } while( bufSize == rdrc );
    return whio_rc.OK;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
