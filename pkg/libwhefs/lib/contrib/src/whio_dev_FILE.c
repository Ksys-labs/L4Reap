/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
/************************************************************************
Implementations for a whio_dev type for wrapping a FILE handle.
It complies with the whio_dev interface, and all
implementation-specified behaviours of the interface are documented
along with the factory functions for creating the device objects.
************************************************************************/

#if !defined(_POSIX_C_SOURCE)
/* required for for fileno(), ftello(), maybe others */
#  define _POSIX_C_SOURCE 200112L
//#  define _POSIX_C_SOURCE 199309L
//#  define _POSIX_C_SOURCE 199506L
#endif

#include <unistd.h> /* ftruncate() */
#include <stdlib.h>
#include <stdio.h>
#if defined(__GNUC__) || defined(__TINYC__)
#if !defined(GCC_VERSION) || (GCC_VERSION < 40100)
/* i don't actually know which versions need this, but 4.0.2 does. */
    extern int ftruncate(int , off_t);
    extern int fsync(int fd);
//#  warning "Kludging ftruncate() and fsync() declartions."
//#else
//#  warning "Hoping ftruncate() and fsync() are declared."
#endif
#endif /* __GNUC__ */

#include <wh/whio/whio_devs.h>


/**
   Internal implementation details for the whio_dev FILE wrapper.
*/
typedef struct whio_dev_FILE
{
    /**
       Underlying FILE handle. Owned by this
       object.
    */
    FILE * fp;
    int fileno;
    /**
       Flags whether we need to do a flush (i.e. if any writes
       have been called since the last flush).
     */
    short needsFlush;
    bool ownsFile;
    short iomode;
} whio_dev_FILE;


/**
   Initialization object for whio_dev_FILE objects. Also used as
   whio_dev::typeID for such objects.
*/
#define WHIO_DEV_FILE_INIT { \
    0, /* fp */ \
    0, /* fileno */ \
    0, /* needsFlush */ \
    0, /* ownsFile */                       \
   -1 /*iomode*/ \
    }
static const whio_dev_FILE whio_dev_FILE_meta_init = WHIO_DEV_FILE_INIT;

#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_FILE_alloc_slots object.
*/
whio_dev_FILE_alloc_count = 5
};
struct
{
    whio_dev_FILE objs[whio_dev_FILE_alloc_count];
    char used[whio_dev_FILE_alloc_count];
} whio_dev_FILE_alloc_slots = { {WHIO_DEV_FILE_INIT}, {0} };
#endif

static whio_dev_FILE * whio_dev_FILE_alloc()
{
    whio_dev_FILE * obj = 0;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whio_dev_FILE_alloc_count; ++i )
    {
	if( whio_dev_FILE_alloc_slots.used[i] ) continue;
	whio_dev_FILE_alloc_slots.used[i] = 1;
	obj = &whio_dev_FILE_alloc_slots.objs[i];
	break;
    }
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_FILE *) malloc( sizeof(whio_dev_FILE) );
    if( obj ) *obj = whio_dev_FILE_meta_init;
    return obj;
}

static void whio_dev_FILE_free( whio_dev_FILE * obj )
{
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_FILE_alloc_slots.objs[0]) ||
	(obj > &whio_dev_FILE_alloc_slots.objs[whio_dev_FILE_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_FILE_alloc_slots.objs[0]);
	whio_dev_FILE_alloc_slots.objs[ndx] = whio_dev_FILE_meta_init;
	whio_dev_FILE_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_FILE API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a whio_dev_FILE.
 */
#define WHIO_FILE_DECL(RV) whio_dev_FILE * f = (dev ? (whio_dev_FILE*)dev->impl.data : 0); \
    if( !f || !f->fp || ((void const *)&whio_dev_FILE_meta_init != dev->impl.typeID) ) return RV

static whio_size_t whio_dev_FILE_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_FILE_DECL(whio_rc.SizeTError);
    if( f->needsFlush ) dev->api->flush(dev);
    return (dev && dest)
	? fread( dest, sizeof(char), n, f->fp )
	: 0;
}

static whio_size_t whio_dev_FILE_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_FILE_DECL(0);
    f->needsFlush = (n ? 1 : 0);
    return (dev && src && n)
	? fwrite( src, sizeof(char), n, f->fp )
	: 0;
}

static int whio_dev_FILE_error( whio_dev * dev )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    return ferror(f->fp);
}

static int whio_dev_FILE_clear_error( whio_dev * dev )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    clearerr(f->fp);
    return whio_rc.OK;
}

static int whio_dev_FILE_eof( whio_dev * dev )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    return feof(f->fp);
}

static whio_size_t whio_dev_FILE_tell( whio_dev * dev )
{
    WHIO_FILE_DECL(whio_rc.SizeTError);
    off_t rc = ftello(f->fp);
    return (rc>=0) ? (whio_size_t)rc : whio_rc.SizeTError;
}

static whio_size_t whio_dev_FILE_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_FILE_DECL(whio_rc.SizeTError);
    if( 0 == fseeko( f->fp, pos, whence ) )
    {
	off_t t = ftello( f->fp );
	return (t >= 0) ? (whio_size_t)t : whio_rc.SizeTError;
    }
    return whio_rc.SizeTError;
}

static int whio_dev_FILE_flush( whio_dev * dev )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    f->needsFlush = 0;
    return (0 == fflush( f->fp ))
	? whio_rc.OK
	: whio_rc.IOError;
}

static int whio_dev_FILE_trunc( whio_dev * dev, whio_off_t len )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    return ftruncate( f->fileno, len );
    /** ^^^ is there a way to truncate a FILE handle without using fileno()? */
}

static int whio_dev_FILE_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    WHIO_FILE_DECL(whio_rc.ArgError);
    /**
       The standard ioctl() looks like:

       int ioctl(int d, int request, ...);

       which means there's no way for us to pass our ... directly to
       it.  So it appears to be impossible to emulate the system's
       ioctl() this without literally checking every possible ioctl
       and casting the first ... arg to the proper type (which is
       likely platform-dependent).
    */
    int rc = whio_rc.UnsupportedError;
    switch( arg )
    {
      case whio_dev_ioctl_FILE_fd:
	  rc = whio_rc.OK;
	  *(va_arg(vargs,int*)) = f->fileno;
	  break;
      default: break;
    };
    return rc;
}

short whio_dev_FILE_iomode( whio_dev * dev )
{
    WHIO_FILE_DECL(-1);
    return f->iomode;
}

static bool whio_dev_FILE_close( whio_dev * dev )
{
    if( dev )
    {
	dev->api->flush(dev);
	if( dev->client.dtor ) dev->client.dtor( dev->client.data );
	dev->client = whio_client_data_empty;
	whio_dev_FILE * f = (whio_dev_FILE*)dev->impl.data;
	if( f )
	{
	    dev->impl.data = 0;
	    if( f->fp && f->ownsFile ) fclose( f->fp );
	    f->fileno = 0;
	    //free( f );
	    whio_dev_FILE_free( f );
	    return true;
	}
    }
    return false;
}

static void whio_dev_FILE_finalize( whio_dev * dev )
{
    if( dev )
    {
	dev->api->close( dev );
	whio_dev_free(dev);
    }
}
#undef WHIO_FILE_DECL

static const whio_dev_api whio_dev_FILE_api =
    {
    whio_dev_FILE_read,
    whio_dev_FILE_write,
    whio_dev_FILE_close,
    whio_dev_FILE_finalize,
    whio_dev_FILE_error,
    whio_dev_FILE_clear_error,
    whio_dev_FILE_eof,
    whio_dev_FILE_tell,
    whio_dev_FILE_seek,
    whio_dev_FILE_flush,
    whio_dev_FILE_trunc,
    whio_dev_FILE_ioctl,
    whio_dev_FILE_iomode
    };

static const whio_dev whio_dev_FILE_init =
    {
    &whio_dev_FILE_api,
    { /* impl */
    0, /* data. Must be-a (whio_dev_FILE*) */
    (void const *)&whio_dev_FILE_meta_init /* typeID */
    }
    };

whio_dev * whio_dev_for_FILE( FILE * F, bool takeOwnership )
{
#if 0
    // TODO: test this, and enable it if it really does what i think it should do:
    if( (off_t)-1 == lseek( fileno(F), 0L, SEEK_CUR ) )
    {/* device does not seem to be seekable (not random-access). */
	return 0;
    }
#endif
    whio_dev * dev = whio_dev_alloc();
    if( ! dev ) return 0;
    whio_dev_FILE * meta = whio_dev_FILE_alloc();
    if( ! meta )
    {
	whio_dev_free(dev);
	return 0;
    }
    *dev = whio_dev_FILE_init;
    *meta = whio_dev_FILE_meta_init;
    dev->impl.data = meta;
    meta->fp = F;
    meta->ownsFile = takeOwnership;
    meta->fileno = fileno(F);
    meta->iomode = -1;
    return dev;
}

#if 0 /* now implemented in whio_dev_fileno.c, but this may be interesting for later. */
whio_dev * whio_dev_for_filename( char const * fname, char const * mode )
{
    if( ! fname || !mode ) return 0;
    FILE * f = fopen( fname, mode );
    if( ! f ) return 0;
    whio_dev * d = whio_dev_for_FILE( f, true );
    if( ! d )
    {
	fclose(f);
        return 0;
    }
    whio_dev_FILE * meta = (whio_dev_FILE*)d->impl.data;
    meta->iomode = whio_mode_to_iomode( mode );
    return d;
}
#endif

