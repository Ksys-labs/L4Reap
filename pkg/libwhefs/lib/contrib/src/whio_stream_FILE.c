
#if !defined(_POSIX_C_SOURCE)
/* required for for fileno(), ftello(), maybe others */
#  define _POSIX_C_SOURCE 200112L
#endif

/*
  whio_stream wrapper implementation for FILE handles.
*/
#include <stdlib.h> /* malloc()/free() */
#include <string.h> /* strstr() */
#include <stdio.h> /* (FILE*) */
#include <wh/whio/whio_stream.h>

static bool whio_stream_FILE_isgood( whio_stream * self );
static whio_size_t whio_stream_FILE_read( whio_stream * self, void * dest, whio_size_t max );
static whio_size_t whio_stream_FILE_write( whio_stream * self, void const * src, whio_size_t len );
static int whio_stream_FILE_flush( whio_stream * self );
static void whio_stream_FILE_finalize( whio_stream * self );
static bool whio_stream_FILE_close( whio_stream * self );
static short whio_stream_FILE_iomode( whio_stream * self );

const whio_stream_api whio_stream_api_FILE = 
    {
    whio_stream_FILE_read,
    whio_stream_FILE_write,
    whio_stream_FILE_close,
    whio_stream_FILE_finalize,
    whio_stream_FILE_flush,
    whio_stream_FILE_isgood,
    whio_stream_FILE_iomode
    };

const whio_stream whio_stream_FILE_init = 
    {
    &whio_stream_api_FILE,
    { /* impl */
    0, /* data */
    &whio_stream_api_FILE /* typeID */
    }
    };


typedef struct whio_stream_FILEINFO
{
    /**
       File handle this object proxies.
    */
    FILE * fp;
    /**
       fileno(fp)
    */
    int fileno;
    /**
       If this object owns its FILE pointer (it opened it itself)
       then api->finalize() will fclose() it.
     */
    bool ownsFile;
    short iomode;
} whio_stream_FILEINFO;
static const whio_stream_FILEINFO whio_stream_FILEINFO_init =
    {
    0, /* fp */
    0, /* fileno */
    false, /* ownsFile */
    -1 /* iomode */
    };

whio_stream * whio_stream_for_FILE( FILE * fp, bool takeOwnership )
{
    if( ! fp ) return 0;
    whio_stream * st = (whio_stream *) malloc( sizeof(whio_stream) );
    if( ! st ) return 0;
    whio_stream_FILEINFO * meta = (whio_stream_FILEINFO*) malloc( sizeof(whio_stream_FILEINFO) );
    if( ! meta )
    {
	free( st );
	return 0;
    }
    *st = whio_stream_FILE_init;
    st->impl.data = meta;
    *meta = whio_stream_FILEINFO_init;
    meta->ownsFile = takeOwnership;
    meta->fp = fp;
    meta->fileno = fileno(fp);
    meta->iomode = -1;
    return st;
}

whio_stream * whio_stream_for_filename( char const * src, char const * mode )
{
    if( ! src || ! mode ) return 0;
    FILE * fp = fopen( src, mode );
    if( ! fp ) return 0;
    whio_stream * st = whio_stream_for_FILE(fp, true);
    if( ! st )
    {
	fclose(fp);
    }
    else
    {
        whio_stream_FILEINFO * meta = (whio_stream_FILEINFO*)st->impl.data;
        meta->iomode = whio_mode_to_iomode( mode );
    }
    return st;
}

whio_stream * whio_stream_for_fileno( int fileno, bool writeMode )
{
    FILE * fp = fdopen( fileno, writeMode ? "wb" : "r+b" );
    if( ! fp ) return 0;
    whio_stream * st = whio_stream_for_FILE(fp, true);
    if( ! st )
    {
	fclose(fp);
    }
    else
    {
        whio_stream_FILEINFO * meta = (whio_stream_FILEINFO*)st->impl.data;
        meta->iomode = writeMode ? 1 : 0;
    }
    return st;
}

/**
   Helper macro for the whio_stream_FILE_xxx() API.
*/
#define WHIO_STR_FILE_DECL(RV) whio_stream_FILEINFO * meta = (self && (self->impl.typeID==&whio_stream_api_FILE)) ? (whio_stream_FILEINFO*)self->impl.data : 0; \
    if( ! meta ) return RV


/**
   whio_stream_api.isgood implementation for whio_stream_FILE.
*/
static bool whio_stream_FILE_isgood( whio_stream * self )
{
    if( self && self->impl.data )
    {
	WHIO_STR_FILE_DECL(false);
	return meta->fp && (0 == ferror(meta->fp));
    }
    return false;
}

static short whio_stream_FILE_iomode( whio_stream * self )
{
	WHIO_STR_FILE_DECL(-1);
        return meta->iomode;
}

/**
   whio_stream_api.read implementation for whio_stream_FILE.
*/
static whio_size_t whio_stream_FILE_read( whio_stream * self, void * dest, whio_size_t max )
{
    WHIO_STR_FILE_DECL(0);
    if( ! self || !max || !dest	) return 0;
    else return fread( dest, sizeof(char), max, meta->fp );
}

/**
   whio_stream_api.write implementation for whio_stream_FILE.
*/
static whio_size_t whio_stream_FILE_write( whio_stream * self, void const * src, whio_size_t len )
{
    WHIO_STR_FILE_DECL(false);
    if( ! self ) return false;
    else return fwrite( src, sizeof(char), len, meta->fp );
}

static int whio_stream_FILE_flush( whio_stream * self )
{
    WHIO_STR_FILE_DECL(whio_rc.ArgError);
    return (meta->fp)
	? fflush(meta->fp)
	: whio_rc.InternalError;
}

#undef WHIO_STR_FILE_DECL
static bool whio_stream_FILE_close( whio_stream * self )
{
    whio_stream_FILEINFO * meta = (self && self->impl.data)
	? (whio_stream_FILEINFO*)self->impl.data
	: 0;
    if( ! meta ) return false;
    self->api->flush( self );
    if( self->client.dtor ) self->client.dtor( self->client.data );
    self->client = whio_client_data_empty;
    self->impl.data = 0;
    if( meta->fp && meta->ownsFile )
    {
        fclose( meta->fp );
    }
    *meta = whio_stream_FILEINFO_init;
    free(meta);
    return true;
}

/**
   whio_stream_api.destroy implementation for whio_stream_FILE.
*/
static void whio_stream_FILE_finalize( whio_stream * self )
{
    if( self )
    {
	self->api->close( self );
	*self = whio_stream_FILE_init;
	free(self);
    }
}

