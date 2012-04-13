/*
  Implementations for the whefs_file-related API.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/
#include <stdlib.h> /* malloc() and friends */
#include <string.h> /* strchr() */
#include "whefs_details.c"
#include <wh/whio/whio_streams.h> /* whio_stream_for_dev() */

#define whefs_file_empty_m { \
    0, /* fs */ \
    0, /* flags */ \
    0, /* dev */ \
    0 /* inode_id */                       \
    }

const whefs_file whefs_file_empty = whefs_file_empty_m;
#define WHEFS_FILE_ISOPENED(F) ((F) && ((F)->flags & WHEFS_FLAG_Opened))
#define WHEFS_FILE_ISRO(F) ((F) && ((F)->flags & WHEFS_FLAG_Read))
#define WHEFS_FILE_ISRW(F) ((F) && ((F)->flags & WHEFS_FLAG_Write))
#define WHEFS_FILE_ISERR(F) ((F) && ((F)->flags & WHEFS_FLAG_FileError))
#define WHEFS_FILE_SET_ERR(F,ERR) ((F) && ((F)->flags |= WHEFS_FLAG_FileError))


#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whefs_file_alloc_slots object.
*/
whefs_file_alloc_count = 10
};

static struct
{
    whefs_file objs[whefs_file_alloc_count];
    char used[whefs_file_alloc_count];
} whefs_file_alloc_slots = { {whefs_file_empty_m}, {0} };
#endif

static whefs_file * whefs_file_alloc()
{
    whefs_file * obj = 0;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whefs_file_alloc_count; ++i )
    {
	if( whefs_file_alloc_slots.used[i] ) continue;
	whefs_file_alloc_slots.used[i] = 1;
	whefs_file_alloc_slots.objs[i] = whefs_file_empty;
	obj = &whefs_file_alloc_slots.objs[i];
	break;
    }
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj )
    {
        obj = (whefs_file *) malloc( sizeof(whefs_file) );
        if( obj ) *obj = whefs_file_empty;
    }
    return obj;
}

static void whefs_file_free( whefs_file * restrict obj )
{
    if(obj) whefs_string_clear( &obj->name, false );
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whefs_file_alloc_slots.objs[0]) ||
	(obj > &whefs_file_alloc_slots.objs[whefs_file_alloc_count-1]) )
    { /* it does not belong to us */
        *obj = whefs_file_empty;
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whefs_file_alloc_slots.objs[0]);
	whefs_file_alloc_slots.objs[ndx] = whefs_file_empty;
	whefs_file_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    *obj = whefs_file_empty;
    free(obj);
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
}

/**
   Opens a pseudofile in read-only mode. f must be an allocated
   whefs_file object. This routine initializes f->dev and directs it
   to the found inode.  Returns whefs_rc.OK on success.

   This routine has an embarassing intimate relationship with
   whefs_fopen().
*/
static int whefs_fopen_ro( whefs_file * restrict f, char const * name )
{
    whefs_inode n = whefs_inode_empty;
    int rc = whefs_inode_by_name( f->fs, name, &n );
    if( whefs_rc.OK == rc )
    {
	if( f->dev ) f->dev->api->finalize(f->dev);
	f->dev = whefs_dev_for_inode( f->fs, n.id, false );
	if( f->dev )
	{
	    f->inode = n.id;
	    f->flags = WHEFS_FLAG_Opened | WHEFS_FLAG_Read;
	}
	else
	{
	    f->flags = WHEFS_FLAG_FileError;
	    rc = whefs_rc.AllocError; /* guessing! */
	}
    }
    return rc;
}

/**
   Like whefs_fopen_ro(), but sets up f in read/write mode.
*/
static int whefs_fopen_rw( whefs_file * restrict f, char const * name )
{
    if( ! f || ! name ) return whefs_rc.ArgError;
    if( ! whefs_fs_is_rw(f->fs) ) return whefs_rc.AccessError;
    whefs_inode n = whefs_inode_empty;
    int rc = whefs_inode_by_name( f->fs, name, &n );
    if( whefs_rc.OK != rc ) do
    {
        //WHEFS_DBG_FYI("whefs_inode_by_name(fs,[%s]) found no inode. Trying to create one...",name);
	/**
	   Create new entry...
	*/
	rc = whefs_inode_next_free( f->fs, &n, true );
	if( rc != whefs_rc.OK ) break;
	rc = whefs_inode_name_set( f->fs, n.id, name );
	if( rc != whefs_rc.OK ) break;
	whefs_inode_update_mtime( f->fs, &n );
	rc = whefs_inode_flush( f->fs, &n );
	//if( rc != whefs_rc.OK ) break;
	break;
    } while(1);
    //WHEFS_DBG("f->flags = 0x%04x", f->flags );
    if( rc != whefs_rc.OK )
    {
	WHEFS_DBG_ERR("open-for-write (%s) failed with rc %d\n!",name,rc);
	return rc;
    }
    if( f->dev ) f->dev->api->finalize(f->dev);
    f->dev = whefs_dev_for_inode( f->fs, n.id, true );
    if( f->dev )
    {
	f->inode = n.id;
	f->flags = WHEFS_FLAG_Opened | WHEFS_FLAG_Write;
    }
    else
    {
	f->flags = WHEFS_FLAG_FileError;
        rc = whefs_rc.InternalError;
    }
    return rc;
}

whefs_file * whefs_fopen( whefs_fs * fs, char const * name, char const * mode )
{
    if( ! fs || !name || !*name || !mode || !*mode ) return 0;
    unsigned int flags = 0;
    if( 0 && (0 != strchr( mode, 'w' )) )
    { // FIXME: add support for mode 'w' and 'w+'
	flags = WHEFS_FLAG_ReadWrite;
    }
    else if( 0 != strchr( mode, 'r' ) )
    {
	if( 0 != strchr( mode, '+' ) ) flags = WHEFS_FLAG_ReadWrite;
	else flags = WHEFS_FLAG_Read;
    }
    if( ! flags ) return 0;
    if( (flags & WHEFS_FLAG_Write) && !whefs_fs_is_rw(fs) )
    {
	WHEFS_DBG_WARN("EFS is opened read-only, so we cannot open files in read/write mode.");
	return 0;
    }
    whefs_file * f = whefs_file_alloc();
    if( ! f ) return 0;
    *f = whefs_file_empty;
    f->fs = fs;
    f->flags = flags;
    int rc = whefs_rc.IOError;
    //WHEFS_DBG_FYI("fopen(fs,[%s],[%s]) flags=0x%08x ISRW=%d", name, mode, flags, WHEFS_FILE_ISRW(f) );
    rc = WHEFS_FILE_ISRW(f)
	? whefs_fopen_rw( f, name )
	: whefs_fopen_ro( f, name );
    if( (rc != whefs_rc.OK) || !WHEFS_FILE_ISOPENED(f) || WHEFS_FILE_ISERR(f) )
    {
	whefs_fclose( f );
	f = 0;
    }
    else
    {
        
        whefs_fs_closer_file_add( fs, f );
    }
    //WHEFS_DBG("opened whefs_file [%s]. mode=%s, flags=%08x", name, mode, f->flags );
    return f;
}

whio_dev * whefs_dev_open( whefs_fs * fs, char const * name, bool writeMode )
{
    if( ! fs || !name ) return 0;
    if( writeMode && ! whefs_fs_is_rw(fs) )
    {
	return 0;
    }
    whefs_inode ino = whefs_inode_empty;
    if( whefs_rc.OK != whefs_inode_by_name( fs, name, &ino ) )
    { // Try to create one...
	if( ! writeMode )
	{
	    WHEFS_DBG_WARN("Open for read failed: did not find pseudofile named [%s].",name);
	    return 0;
	}
	if( whefs_rc.OK != whefs_inode_next_free( fs, &ino, true ) )
	{
	    WHEFS_DBG_WARN("Opening inode for [%s] failed! EFS is likely full.", name );
	    return 0;
	}
	if( whefs_rc.OK != whefs_inode_name_set( fs, ino.id, name ) )
	{
	    WHEFS_DBG_WARN("Setting inode #%"WHEFS_ID_TYPE_PFMT" name to [%s] failed!", ino.id, name );
	    return 0;
	}
	whefs_inode_flush( fs, &ino );
    }
    whio_dev * dev = whefs_dev_for_inode( fs, ino.id, writeMode );
    if( ! dev )
    {
	WHEFS_FIXME("Creation of i/o device for inode #%"WHEFS_ID_TYPE_PFMT" failed - "
		    "be sure we delete the inode entry if it didn't exist before this call!",
		    ino.id );
    }
    return dev;
}


/**
   Internal type to allow us to properly disconnect a whio_stream
   object from close-at-shutdown without needing to add a custom
   whio_stream implementation.
*/
typedef struct
{
    whefs_fs * fs;
    whio_stream const * stream;
} whefs_stream_closer_kludge;

/** v must be-a populated whefs_stream_closer_kludge.
    Removes v->stream from fs' closer list and frees
    v.
*/
static void whefs_stream_closer_kludge_dtor( void * v )
{
    if(0) whefs_stream_closer_kludge_dtor(0); /* avoid "static func defined but not used" warning. */
    if(v)
    {
        //WHEFS_DBG("Removing whio_stream from closer list.");
        whefs_stream_closer_kludge * k = (whefs_stream_closer_kludge*)v;
        whefs_fs_closer_stream_remove( k->fs, k->stream );
        free( k );
    }
}

whio_stream * whefs_stream_open( whefs_fs * fs, char const * name, bool writeMode, bool append )
{
    whio_dev * d = whefs_dev_open( fs, name, writeMode );
    if( ! d ) return 0;
    if( writeMode )
    {
        if( append )
        {
            d->api->seek( d, 0L, SEEK_END );
        }
        else
        {
            d->api->truncate( d, 0 );
        }
    }
    whefs_stream_closer_kludge * k = (whefs_stream_closer_kludge*)malloc(sizeof(whefs_stream_closer_kludge));
    if( ! k )
    {
        d->api->finalize(d);
        return 0;
    }
    whio_stream * s = whio_stream_for_dev( d, true );
    if( ! s )
    {
        free(k);
        d->api->finalize(d);
    }
    else
    {
        k->fs = fs;
        k->stream = s;
        whefs_fs_closer_stream_add( fs, s, d );
        d->client.data = k;
        d->client.dtor = whefs_stream_closer_kludge_dtor;
    }
    return s;
}


whio_dev * whefs_fdev( whefs_file * restrict f )
{
    return f ? f->dev : 0;
}

whio_size_t whefs_fseek( whefs_file * restrict f, size_t pos, int whence )
{
    return (f && f->dev)
	? f->dev->api->seek( f->dev, pos, whence )
	: whefs_rc.SizeTError;
}

int whefs_frewind( whefs_file * restrict f )
{
    return (f)
	? whio_dev_rewind( f->dev )
	: whefs_rc.ArgError;
}

int whefs_ftrunc( whefs_file * restrict f, size_t pos )
{
    return (f && f->dev)
	? f->dev->api->truncate( f->dev, pos )
	: whefs_rc.ArgError;
}


whefs_fs * whefs_file_fs( whefs_file * restrict f )
{
    return f ? f->fs : 0;
}

int whefs_fclose( whefs_file * restrict f )
{
    int rc = f ? whefs_rc.OK : whefs_rc.ArgError;
    if( whefs_rc.OK == rc )
    {
        whefs_fs_closer_file_remove( f->fs, f );
	if( WHEFS_FILE_ISRW(f) && WHEFS_FILE_ISOPENED(f) ) whefs_fs_flush( f->fs );
	if( f->dev ) f->dev->api->finalize(f->dev);
	whefs_file_free(f);
    }
    return rc;
}

int whefs_fflush( whefs_file * restrict f )
{
    if( ! f ) return whefs_rc.ArgError;
    else
    {
	if( WHEFS_FILE_ISOPENED(f) && WHEFS_FILE_ISRW(f) )
        {
            return f->dev->api->flush(f->dev);
        }
        return whefs_rc.OK;
    }
}

int whefs_dev_close( whio_dev * restrict dev )
{
    int rc = dev ? whefs_rc.OK : whefs_rc.ArgError;
    if( whefs_rc.OK == rc )
    {
	dev->api->finalize( dev );
    }
    return rc;
}

size_t whefs_fread( whefs_file * restrict f, size_t size, size_t count, void * dest )
{
    if( ! f || !count || (count && !dest) || !size || !f->dev ) return 0;
    size_t x = 0;
    for( ; x < count; ++x )
    {
	size_t rsz = f->dev->api->read( f->dev, WHIO_VOID_PTR_ADD(dest,(x*size)), size );
	if( size != rsz ) break;
    }
    return x;
}

size_t whefs_fwrite( whefs_file * restrict f, size_t sz, size_t count, void const * src )
{
    if( ! f || !count || (count && !src) || !sz || !f->dev ) return 0;
    size_t x = 0;
    for( ; x < count; ++x )
    {
	size_t wsz = f->dev->api->write( f->dev, WHIO_VOID_CPTR_ADD(src,(x*sz)), sz );
	if( sz != wsz ) break;
    }
    //if( x ) f->dev->api->flush(f->dev);
    return x;
}

size_t whefs_fwritev( whefs_file * restrict f, char const * fmt, va_list vargs )
{
    return f ? whio_dev_writefv( f->dev, fmt, vargs ) : 0;
}

size_t whefs_fwritef( whefs_file * restrict f, char const * fmt, ... )
{
    size_t rc;
    va_list vargs;
    va_start(vargs,fmt);
    rc = whefs_fwritev( f, fmt, vargs );
    va_end(vargs);
    return rc;
}

size_t whefs_file_write( whefs_file * restrict f, void const * src, size_t n  )
{
    return (f && f->dev)
	? f->dev->api->write( f->dev, src, n )
	: 0;
}

size_t whefs_file_read( whefs_file * restrict f, void * dest, size_t n  )
{
    return (f && f->dev)
	? f->dev->api->read( f->dev, dest, n )
	: 0;
}

#if 0
not sure about this one;
int whefs_unlink_file( whefs_file * restrict )
{
    if( ! f ) return whefs_rc.ArgError;
    f->dev->api->finalize(f->dev);
    f->dev = 0;
    whefs_inode ino = whefs_inode_empty;
    ino.id = f->inode;
    whefs_inode_read( f->fs, &ino );
    int rc = whefs_inode_unlink( f->fs, &ino );
    whefs_fclose( f );
    return rc;
}
#endif

int whefs_unlink_filename( whefs_fs * fs, char const * fname )
{
    if( ! fs || !fname ) return whefs_rc.ArgError;
    whefs_inode ino = whefs_inode_empty;
    int rc = whefs_inode_by_name( fs, (char const *) /* FIXME: signedness*/ fname, &ino );
    if( whefs_rc.OK == rc )
    {
	rc = whefs_inode_unlink( fs, &ino );
    }
    return rc;
}

/**
   An empty whefs_file_stats object which can (and should)
   be used to initialize whefs_file_stats objects to a zeroed
   state. Relying on compiler default values is a bad idea
   (been there, done that).
*/
static const whefs_file_stats whefs_file_stats_empty =
    {
    0, /* bytes */
    0, /* inode */
    0 /* blocks */
    };

int whefs_fstat( whefs_file const * f, whefs_file_stats * st )
{
    if( ! f || !st ) return whefs_rc.ArgError;
    *st = whefs_file_stats_empty;
    st->inode = f->inode;
    whefs_inode ino = whefs_inode_empty;
    int rc = whefs_inode_id_read( f->fs, f->inode, &ino );
    if( whefs_rc.OK != rc ) return rc;
    st->bytes = ino.data_size;
    whefs_id_type bid = ino.first_block;
    whefs_block bl = whefs_block_empty;
    bl.id = bid;
    while( bl.id )
    {
	++st->blocks;
	rc = whefs_block_read( f->fs, bl.id, &bl );
	if( whefs_rc.OK != rc ) return rc;
	bl.id = bl.next_block;
    }
    return rc;
}

int whefs_file_name_set( whefs_file * restrict f, char const * newName )
{
    if( ! f || (! newName || !*newName) ) return whefs_rc.ArgError;
    if( ! WHEFS_FILE_ISRW(f) ) return whefs_rc.AccessError;
    whefs_inode * ino = 0;
    int rc = whefs_inode_search_opened( f->fs, f->inode, &ino );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("This should never ever happen: f appears to be a valid whefs_file, but we could find no associated opened inode!");
	return rc;
    }
    rc = whefs_inode_name_set( f->fs, ino->id, newName );
    if( whefs_rc.OK != rc ) return rc;
    //whefs_inode_flush( f->fs, ino );
    return whefs_rc.OK;
}

char const * whefs_file_name_get( whefs_file * restrict f )
{
#if 0
    if( ! f ) return 0;
    whefs_inode * ino = 0;
    int rc = whefs_inode_search_opened( f->fs, f->inode, &ino );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("This should never ever happen: f appears to be a "
		      "whefs_file, but we could find no associated opened inode!");
	return 0;
    }
    return ino->name.string;
#else
    int rc =whefs_inode_name_get( f->fs, f->inode, &f->name );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("This should never ever happen: f appears to be a "
		      "whefs_file, but we could find no associated opened inode!");
	return 0;
    }
    return f->name.string;
#endif
}

whio_size_t whefs_fsize( whefs_file const * restrict f )
{
#if 1
    if( ! f ) return whefs_rc.SizeTError;
    whefs_inode * ino = 0;
    int rc = whefs_inode_search_opened( f->fs, f->inode, &ino );
    return (whefs_rc.OK == rc)
	? ino->data_size
	: whefs_rc.SizeTError;
#else /* faster, but not technically const */
    return (f && f->dev)
	? whio_dev_size( f->dev )
	: whefs_rc.SizeTError;
#endif
}
