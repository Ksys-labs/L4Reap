#include "whio_streams.h"
#include <stdlib.h> /* free/malloc */

/*
  Implementation for a whio_stream wrapper for a whio_dev object.
*/
#ifdef __cplusplus
#define ARG_UNUSED(X)
extern "C" {
#else
#define ARG_UNUSED(X) X
#endif

static bool whio_stream_STREAMTYPE_isgood( whio_stream * self );
static whio_size_t whio_stream_STREAMTYPE_read( whio_stream * self, void * dest, whio_size_t max );
static whio_size_t whio_stream_STREAMTYPE_write( whio_stream * self, void const * src, whio_size_t len );
static int whio_stream_STREAMTYPE_flush( whio_stream * ARG_UNUSED(self) );
static bool whio_stream_STREAMTYPE_close( whio_stream * self );
static void whio_stream_STREAMTYPE_finalize( whio_stream * self );

const whio_stream_api whio_stream_api_STREAMTYPE = 
    {
    whio_stream_STREAMTYPE_read,
    whio_stream_STREAMTYPE_write,
    whio_stream_STREAMTYPE_close,
    whio_stream_STREAMTYPE_finalize,
    whio_stream_STREAMTYPE_flush,
    whio_stream_STREAMTYPE_isgood
    };

const whio_stream whio_stream_STREAMTYPE =
    {
    &whio_stream_api_STREAMTYPE,
    { /* impl */
    0, /* data */
    &whio_stream_api_STREAMTYPE /* typeID */
    }
    };


typedef struct whio_stream_STREAMTYPE_meta
{
    whio_dev * dev;
    bool ownsDev;
} whio_stream_STREAMTYPE_meta;

static const whio_stream_STREAMTYPE_meta whio_stream_STREAMTYPE_meta_init =
    {
    0, /* dev */
    0 /* ownsDev */
    };

/**
   Helper macro for the whio_stream_STREAMTYPE_xxx() API.
*/
#define WHIO_STR_DECL(RV) whio_stream_STREAMTYPE_meta * meta = (self && (self->impl.typeID==&whio_stream_api_STREAMTYPE)) ? (whio_stream_STREAMTYPE_meta*)self->impl.data : 0; \
    if( ! meta ) return RV

bool whio_stream_STREAMTYPE_isgood( whio_stream * self )
{
    WHIO_STR_DECL(false);
    return meta->dev ? (0 == meta->dev->api->error(meta->dev)) : false;
}

whio_size_t whio_stream_STREAMTYPE_read( whio_stream * self, void * dest, whio_size_t max )
{
    WHIO_STR_DECL(0);
    return meta->dev ? meta->dev->api->read(meta->dev, dest, max) : 0;
}

whio_size_t whio_stream_STREAMTYPE_write( whio_stream * self, void const * src, whio_size_t len )
{
    WHIO_STR_DECL(0);
    return meta->dev ? meta->dev->api->write(meta->dev, src, len) : 0;
}

int whio_stream_STREAMTYPE_flush( whio_stream * ARG_UNUSED(self) )
{
    WHIO_STR_DECL(whio_rc.ArgError);
    return meta->dev->api->flush( meta->dev );
}

bool whio_stream_STREAMTYPE_close( whio_stream * self )
{
    whio_stream_STREAMTYPE_meta * meta = (self ? (whio_stream_STREAMTYPE_meta*)self->impl.data : 0);
    if( meta )
    {
	self->impl.data = 0;
	if( meta->dev )
	{
	    meta->dev->api->flush( meta->dev );
	    if( meta->ownsDev )
	    {
		meta->dev->api->finalize( meta->dev );
	    }
	}
	free( meta );
	return true;
    }
    return false;
}


void whio_stream_STREAMTYPE_finalize( whio_stream * self )
{
    if( self )
    {
	self->api->close( self );
	free( self );
    }
}

whio_stream * whio_stream_for_dev( whio_dev * dev, bool takeOwnership )
{
    if( ! dev ) return 0;
    whio_stream * str = (whio_stream *) malloc( sizeof(whio_stream) );
    if( ! str ) return 0;
    whio_stream_STREAMTYPE_meta * meta = (whio_stream_STREAMTYPE_meta *) malloc( sizeof(whio_stream_STREAMTYPE_meta) );
    if( ! meta )
    {
	free(str);
	return 0;
    }
    *str = whio_stream_STREAMTYPE;
    *meta = whio_stream_STREAMTYPE_meta_init;
    str->impl.data = meta;
    meta->dev = dev;
    meta->ownsDev = takeOwnership;
    return str;
}
#undef WHIO_STR_DECL


#ifdef __cplusplus
} /* extern "C" */
#endif
