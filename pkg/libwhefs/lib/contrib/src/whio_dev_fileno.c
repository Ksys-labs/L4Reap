/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain

  Many thanks to Lew Pitcher for his tips on implementing this:

  http://groups.google.com/group/comp.unix.programmer/browse_thread/thread/9ffb66c1d0a4f7f3/7c28cd32b63d99a4

*/
/************************************************************************
Implementations for a whio_dev type for wrapping a file descriptor
handle.  This is almost a 100% copy/paste of the code from
whio_dev_FILE.c, but it uses the lower-level read()/write() API
instead of the fXXX(FILE*,...) API. Simple tests in libwhefs show this
to provide dramatic speed increases.
************************************************************************/

#if !defined(_POSIX_C_SOURCE)
/* required for for fileno(), ftello(), fdatasync(), maybe others */
#  define _POSIX_C_SOURCE 200112L
//#  define _POSIX_C_SOURCE 199309L
//#  define _POSIX_C_SOURCE 199506L
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* ftruncate(), fdatasync() */
#include <fcntl.h>
#include <errno.h>

#if defined(__GNUC__) || defined(__TINYC__)
#if !defined(GCC_VERSION) || (GCC_VERSION < 40100)
/* i don't actually know which versions need this, but 4.0.2 does. */
    extern int ftruncate(int, off_t);
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
typedef struct whio_dev_fileno
{
    /**
       Underlying FILE handle. Owned by this
       object.
    */
    FILE * fp;
    int fileno;
    char const * filename;
    bool atEOF;
    int errstate;
    short iomode;
} whio_dev_fileno;


/**
   Initialization object for whio_dev_fileno objects. Also used as
   whio_dev::typeID for such objects.
*/
#define WHIO_DEV_fileno_INIT { \
    0, /* fp */ \
    0, /* fileno */ \
    0, /* filename */ \
    false, /* atEOF */ \
    0, /* errstate */                       \
   -1 /*iomode*/ \
    }
static const whio_dev_fileno whio_dev_fileno_meta_empty = WHIO_DEV_fileno_INIT;

#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_fileno_alloc_slots object.
*/
whio_dev_fileno_alloc_count = 5
};
struct
{
    whio_dev_fileno objs[whio_dev_fileno_alloc_count];
    char used[whio_dev_fileno_alloc_count];
} whio_dev_fileno_alloc_slots = { {WHIO_DEV_fileno_INIT}, {0} };
#endif

static whio_dev_fileno * whio_dev_fileno_alloc()
{
    whio_dev_fileno * obj = 0;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whio_dev_fileno_alloc_count; ++i )
    {
	if( whio_dev_fileno_alloc_slots.used[i] ) continue;
	whio_dev_fileno_alloc_slots.used[i] = 1;
	whio_dev_fileno_alloc_slots.objs[i] = whio_dev_fileno_meta_empty;
	obj = &whio_dev_fileno_alloc_slots.objs[i];
	break;
    }
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_fileno *) malloc( sizeof(whio_dev_fileno) );
    return obj;
}

static void whio_dev_fileno_free( whio_dev_fileno * obj )
{
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_fileno_alloc_slots.objs[0]) ||
	(obj > &whio_dev_fileno_alloc_slots.objs[whio_dev_fileno_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_fileno_alloc_slots.objs[0]);
	whio_dev_fileno_alloc_slots.objs[ndx] = whio_dev_fileno_meta_empty;
	whio_dev_fileno_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_fileno API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a whio_dev_fileno.
 */
#define WHIO_fileno_DECL(RV) whio_dev_fileno * f = (dev ? (whio_dev_fileno*)dev->impl.data : 0); \
    if( !f || !f->fp || ((void const *)&whio_dev_fileno_meta_empty != dev->impl.typeID) ) return RV

static whio_size_t whio_dev_fileno_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_fileno_DECL(whio_rc.SizeTError);
    if( ! dest || !n ) return 0;
    ssize_t rc = read( f->fileno, dest, n );
    if( 0 == rc )
    {
	f->atEOF = true;
    }
    else if( (ssize_t)-1 == rc )
    {
	f->errstate = errno;
	rc = 0;
    }
    return (whio_size_t)rc;
}

static whio_size_t whio_dev_fileno_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_fileno_DECL(0);
    if( ! src || !n ) return 0;
    ssize_t rc = write( f->fileno, src, n );
    if( (ssize_t)-1 == rc )
    {
	f->errstate = errno;
	rc = 0;
    }
    return rc;
}

static int whio_dev_fileno_error( whio_dev * dev )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
    /**
      ferror(f->fp) is not likely to be valid b/c we're
      using the low-level i/o API, but what the heck...
    */
    //return ferror(f->fp);
    return f->errstate;
}

static int whio_dev_fileno_clear_error( whio_dev * dev )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
    /** Because we use the low-level read/write() API instead of fread()/fwrite()
	and friends, using clearerr(f->fp) isn't really going to give us anything.
	We'll go ahead and call it and assume the best.
    */
    //clearerr(f->fp);
    f->errstate = 0;
    f->atEOF = false;
    return whio_rc.OK;
}

static int whio_dev_fileno_eof( whio_dev * dev )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
    return  f->atEOF ? 1 : 0;
}

static whio_size_t whio_dev_fileno_tell( whio_dev * dev )
{
    WHIO_fileno_DECL(whio_rc.SizeTError);
    off_t rc = lseek( f->fileno, 0L, SEEK_CUR );
    return (rc>=0) ? (whio_size_t)rc : whio_rc.SizeTError;
}

static whio_size_t whio_dev_fileno_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_fileno_DECL(whio_rc.SizeTError);
    off_t rc = lseek( f->fileno, pos, whence );
    if( pos == rc )
    {
	/**
	   The man page for fseek() says (on my system):

	   "A successful call to the fseek() function clears the
	   end-of-file indicator for the stream."
	*/
	f->atEOF = false;
    }
    return (rc>=0) ? (whio_size_t) rc : whio_rc.SizeTError;
}

static int whio_dev_fileno_flush( whio_dev * dev )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
    return fsync( f->fileno );
    /* reminder: i realy want fdatasync(), but some platforms
    *cough* Solaris *cough* don't appear to have it. */
}

static int whio_dev_fileno_trunc( whio_dev * dev, whio_off_t len )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
    int rc = ftruncate( f->fileno, len );
    if( 0 == rc )
    {
	whio_dev_fileno_flush( dev );
    }
    return rc;
}

short whio_dev_fileno_iomode( whio_dev * dev )
{
    WHIO_fileno_DECL(-1);
    return f->iomode;
}

static int whio_dev_fileno_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    WHIO_fileno_DECL(whio_rc.ArgError);
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
      case whio_dev_ioctl_GENERAL_name:
	  do
	  {
	      char const ** cpp = (va_arg(vargs,char const **));
	      if( cpp )
	      {
		  rc = whio_rc.OK;
		  *cpp = f->filename;
	      }
	      else
	      {
		  rc = whio_rc.ArgError;
	      }
	  } while(0);
	  break;
      case whio_dev_ioctl_FCNTL_lock_nowait:
      case whio_dev_ioctl_FCNTL_lock_wait:
      case whio_dev_ioctl_FCNTL_lock_get:
	  do
	  {
	      struct flock * fl = (va_arg(vargs,struct flock *));
	      if( fl )
	      {
		  int lockCmd = (whio_dev_ioctl_FCNTL_lock_nowait == arg)
		      ? F_SETLK
		      : ((whio_dev_ioctl_FCNTL_lock_get == arg) ? F_GETLK :  F_SETLKW);
		  rc = fcntl( f->fileno, lockCmd, fl );
	      }
	      else
	      {
		  rc = whio_rc.ArgError;
	      }
	  } while(0);
	  break;
      default: break;
    };
    return rc;
}

static bool whio_dev_fileno_close( whio_dev * dev )
{
    if( dev )
    {
	dev->api->flush(dev);
	if( dev->client.dtor ) dev->client.dtor( dev->client.data );
	dev->client = whio_client_data_empty;
	whio_dev_fileno * f = (whio_dev_fileno*)dev->impl.data;
	if( f )
	{
	    dev->impl.data = 0;
	    if( f->fp ) fclose( f->fp );
	    *f = whio_dev_fileno_meta_empty;
	    whio_dev_fileno_free( f );
	    return true;
	}
    }
    return false;
}

static void whio_dev_fileno_finalize( whio_dev * dev )
{
    if( dev )
    {
	dev->api->close( dev );
	whio_dev_free(dev);
    }
}
#undef WHIO_fileno_DECL

static const whio_dev_api whio_dev_fileno_api =
    {
    whio_dev_fileno_read,
    whio_dev_fileno_write,
    whio_dev_fileno_close,
    whio_dev_fileno_finalize,
    whio_dev_fileno_error,
    whio_dev_fileno_clear_error,
    whio_dev_fileno_eof,
    whio_dev_fileno_tell,
    whio_dev_fileno_seek,
    whio_dev_fileno_flush,
    whio_dev_fileno_trunc,
    whio_dev_fileno_ioctl,
    whio_dev_fileno_iomode
    };

static const whio_dev whio_dev_fileno_empty =
    {
    &whio_dev_fileno_api,
    { /* impl */
    0, /* data. Must be-a (whio_dev_fileno*) */
    (void const *)&whio_dev_fileno_meta_empty /* typeID */
    }
    };

/**
   Implementation for whio_dev_for_fileno() and whio_dev_for_filename().

   If fname is 0 or empty then fdopen(fileno,mode) is used, otherwise
   fopen(fname,mode) is used.
*/
static whio_dev * whio_dev_for_file_impl( char const * fname, int filenum, char const * mode )
{
    if( ((!fname || !*fname) && (filenum<1)) || (!mode || !*mode) )
    {
        return 0;
    }
    /** Maintenance reminder:

        i would like to move these two allocs to below the fopen(),
        but if we open the file first then we have to check whether we
        created the file, and delete it if we did not.
    */
    whio_dev * dev = whio_dev_alloc();
    if( ! dev )
    {
        return 0;
    }
    whio_dev_fileno * meta = whio_dev_fileno_alloc();
    if( ! meta )
    {
	whio_dev_free(dev);
	return 0;
    }
    FILE * f = (fname && *fname) ? fopen(fname,mode) : fdopen( filenum, mode );
    if( ! f )
    {
        whio_dev_free(dev);
        whio_dev_fileno_free(meta);
	return 0;
    }
    *dev = whio_dev_fileno_empty;
    *meta = whio_dev_fileno_meta_empty;
    dev->impl.data = meta;
    meta->fp = f;
    meta->fileno = fileno(f);
    meta->filename = fname;
    meta->iomode = whio_mode_to_iomode( mode );
    return dev;
}

whio_dev * whio_dev_for_fileno( int fileno, char const * mode )
{
    return whio_dev_for_file_impl( 0, fileno, mode );
}

#if 1 /* there is a separate implementation in whio_dev_FILE.c, but the API
	 docs describe this one. */
whio_dev * whio_dev_for_filename( char const * fname, char const * mode )
{
    return (fname && *fname)
        ? whio_dev_for_file_impl( fname, -1, mode )
        : NULL;
}
#endif

