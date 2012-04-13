/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain

  This file contains most of the guts of the whefs_fs object.
*/

#include <wh/whefs/whefs_config.h> // MUST COME FIRST b/c of __STDC_FORMAT_MACROS.

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <string.h>

#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_string.h>
#include "whefs_encode.h"
#include "whefs_cache.h"
#include "whefs_details.c"
#include <wh/whio/whio_devs.h>
#include <wh/whio/whio_encode.h>
#include <wh/whglob.h>

#if WHEFS_CONFIG_ENABLE_FCNTL
#  include <fcntl.h>
#endif

#if WHEFS_CONFIG_ENABLE_MMAP
#  include <sys/mman.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define WHEFS_LOAD_CACHES_ON_OPEN 0 /* make sure and test whefs-cp and friends if setting this to 0. */
#if ! WHEFS_LOAD_CACHES_ON_OPEN
//#warning "(0==WHEFS_LOAD_CACHES_ON_OPEN) is is known to cause bugs. Fix it!"
/*
  Reminder to self: the problem here is one of "how do we distinguish empty names vs. not-cached names,"
and is complicated by the voodoo we use to store the strings inside whefs_fs::cache::strings.

20090623: The last i checked it appeared to work fine when disabled,
but i need to look closer to be sure.
*/
#endif

/**
   The various WHEFS_FS_STRUCT_xxx macros are parts of the whefs_fs structure.
*/

#if WHEFS_CONFIG_ENABLE_THREADS
/**
   WHEFS_FS_STRUCT_THREAD_INFO is the initializer for whefs_fs.threads.
*/
#  define WHEFS_FS_STRUCT_THREAD_INFO {/*threads*/ \
        0 /* placeholder */ \
    }
#else
#  define WHEFS_FS_STRUCT_THREAD_INFO {0/* placeholder */}
#endif


/* whefs_fs::cache struct ... */
#define WHEFS_FS_STRUCT_CACHE                \
    {/*cache*/                                  \
        0/*hashes*/,                        \
        whefs_hash_cstring/*hashfunc*/,     \
    }

/* whefs_fs::bits struct ... */
#define WHEFS_FS_STRUCT_BITS                  \
    { /* bits */ \
	WHBITS_INIT, /* i */ \
        WHBITS_INIT, /* b */             \
        false, /* i_loaded */            \
        false /* b_loaded */                \
    }

/* whefs_fs::hints struct ... */
#define WHEFS_FS_STRUCT_HINTS                  \
    { /* hints */ \
	1, /* unused_block_start */ \
	2 /* unused_inode_start == 2 b/c IDs 0 and 1 are reserved for not-an-inode and the root node */  \
    }

/**
   An empty whefs_fs object for us in initializing new objects.
*/
//const whefs_fs whefs_fs_empty =
#define whefs_fs_empty_m { \
    /* flags */ (WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE ? WHEFS_FLAG_FS_EnableHashCache : 0), \
    0, /* err */ \
    {0}, /* offsets */ \
    {0}, /* sizes */ \
    0, /* dev */ \
    true, /* ownsDev */ \
    0, /* filesize */ \
    0, /* opened_nodes */ \
    0, /* closers */ \
    0, /* fileno */ \
    WHEFS_FS_STRUCT_BITS,    \
    WHEFS_FS_STRUCT_HINTS,   \
    WHEFS_FS_OPTIONS_DEFAULT, \
    WHEFS_FS_STRUCT_THREAD_INFO, \
    WHEFS_FS_STRUCT_CACHE,       \
    } /* end of whefs_fs */
const whefs_fs whefs_fs_empty = whefs_fs_empty_m;

#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate in the
   whefs_fs_alloc_slots object.
*/
whefs_fs_alloc_count = 2
};
static struct
{
    whefs_fs objs[whefs_fs_alloc_count];
    char used[whefs_fs_alloc_count];
    size_t next;
} whefs_fs_alloc_slots = { {whefs_fs_empty_m}, {0}, 0 };
#endif
/** @internal

   Allocates an empty-initializes a new whefs_fs object, which the
   caller owns and must destroy with whefs_fs_free() (if the object is
   never used) or whefs_fs_finalize() (if it contains data which needs
   to be cleaned up).
*/
static whefs_fs * whefs_fs_alloc()
{
    whefs_fs * obj = 0;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = whefs_fs_alloc_slots.next;
    for( ; i < whefs_fs_alloc_count; ++i )
    {
	if( whefs_fs_alloc_slots.used[i] ) continue;
	whefs_fs_alloc_slots.next = i+1;
	whefs_fs_alloc_slots.used[i] = 1;
	obj = &whefs_fs_alloc_slots.objs[i];
	break;
    }
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whefs_fs *) malloc( sizeof(whefs_fs) );
    if( obj ) *obj = whefs_fs_empty;
    return obj;
}
/** @internal

    Deallocates obj.
*/
static void whefs_fs_free( whefs_fs * obj )
{
    if( obj ) *obj = whefs_fs_empty;
    else return;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whefs_fs_alloc_slots.objs[0]) ||
	(obj > &whefs_fs_alloc_slots.objs[whefs_fs_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whefs_fs_alloc_slots.objs[0]);
	if( whefs_fs_alloc_slots.next > ndx ) whefs_fs_alloc_slots.next = ndx;
	whefs_fs_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
}


int whefs_fs_lock( whefs_fs * restrict fs, bool writeLock, off_t start, int whence, off_t len )
{
#if WHEFS_CONFIG_ENABLE_FCNTL
    if( ! fs ) return whefs_rc.ArgError;
    //WHEFS_DBG_FYI("whefs_fs_lock(%p [fileno=%d],%d,%ld,%d,%ld)",fs,fs->fileno,writeLock,start,whence,len);
    if( fs->fileno < 1 ) return whefs_rc.UnsupportedError;
    else
    {
	struct flock lock;
	lock.l_type = writeLock ? F_WRLCK : F_RDLCK;
	lock.l_start = start;
	lock.l_whence = whence;
	lock.l_len = len;
	int rc = fcntl( fs->fileno, F_SETLKW, &lock);
        WHEFS_DBG_LOCK("whefs_fs_lock(fs=%p,writeMode=%d,start=%ld,whence=%d,len=%ld) rc=%d",fs,writeLock,start,whence,len,rc);
        return rc;
    }
#else
    return whefs_rc.UnsupportedError;
#endif
}

int whefs_fs_unlock( whefs_fs * restrict fs, off_t start, int whence, off_t len )
{
#if WHEFS_CONFIG_ENABLE_FCNTL
    if( ! fs ) return whefs_rc.ArgError;
    if( fs->fileno < 1 ) return whefs_rc.UnsupportedError;
    else
    {
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_start = start;
	lock.l_whence = whence;
	lock.l_len = len;
	int rc = fcntl( fs->fileno, F_SETLK, &lock);
        WHEFS_DBG_LOCK("whefs_fs_unlock(fs=%p,start=%ld,whence=%d,len=%ld) rc=%d",fs,start,whence,len,rc);
        return rc;
    }
#else
    return whefs_rc.UnsupportedError;
#endif
}

int whefs_fs_lock_range( whefs_fs * restrict fs, bool writeLock, whefs_fs_range_locker const * range )
{
    return (fs && range)
	? whefs_fs_lock( fs, writeLock, range->start, range->whence, range->len )
	: whefs_rc.ArgError;
}

int whefs_fs_unlock_range( whefs_fs * restrict fs, whefs_fs_range_locker const * range )
{
    return (fs && range)
	? whefs_fs_unlock( fs, range->start, range->whence, range->len )
	: whefs_rc.ArgError;
}

#if WHEFS_CONFIG_ENABLE_MMAP
/** Internal data for storing info about mmap()ed storage. */
typedef struct
{
    void * mem;
    whio_dev * fdev;
    int fileno;
    whio_size_t size;
    bool writeMode;
    bool async;
}  WhioDevMMapInfo;
static const WhioDevMMapInfo WhioDevMMapInfo_init = {NULL,NULL,0,0,false,false};

/** Internal whio_dev_api::flush() impl for mmap()'d storage. */
static int whio_dev_mmap_flush( whio_dev * dev )
{
    if(0) whio_dev_mmap_flush(0); /* avoid "static func defined but not used" warning. */
    //WHEFS_DBG("msyncing mmap()...");
    WhioDevMMapInfo * m = (WhioDevMMapInfo *)dev->client.data;
    return m->writeMode
        ? msync( m->mem, m->size, m->async ? MS_ASYNC : MS_SYNC )
        : whio_rc.OK;
}
/** Internal whio_dev_api::close() impl for mmap()'d storage. */
static bool whio_dev_mmap_close( whio_dev * dev )
{
    if(0) whio_dev_mmap_close(0); /* avoid "static func defined but not used" warning. */
    if( ! dev ) return false;
    //WHEFS_DBG("closing mmap()...");
    WhioDevMMapInfo * m = (WhioDevMMapInfo *)dev->client.data;
    if( ! m ) return true;
    if( m->mem )
    {
        dev->api->flush( dev );
        //WHEFS_DBG("munmap()...");
        munmap( m->mem, m->size );
    }
    if( m->fdev ) m->fdev->api->finalize(m->fdev);
    *m = WhioDevMMapInfo_init;
    free(m);
    dev->client.data = 0;
    return whio_dev_api_memmap.close( dev );
}

#endif // WHEFS_CONFIG_ENABLE_MMAP

/**
   If WHEFS_CONFIG_ENABLE_MMAP is true then this tries to mmap() the
   underlying storage (fs->dev) IF fs is read/write (profiling shows
   memmapping read access to cost more than direct file access). If it
   succeeds it replaces fs->dev with a proxy device. Returns
   whefs_rc.OK on success. Failure can be ignored unless you REALLY
   need mmap().

   If WHEFS_CONFIG_ENABLE_MMAP is false then whefs_rc.UnsupportedError
   is returned.

   If fs is already mmap()ed or is read-only then whefs_rc.OK is
   returned but fs is not modified.
*/
static int whefs_fs_mmap_connect( whefs_fs * fs )
{
    if( !fs || !fs->dev ) return whefs_rc.ArgError;
    //WHEFS_DBG("Trying mmap? fileno=%d",fs->fileno);
#if ! WHEFS_CONFIG_ENABLE_MMAP
    return whefs_rc.UnsupportedError;
#else
    if( WHEFS_FLAG_FS_IsMMapped & fs->flags ) return whefs_rc.OK;
    if( !whefs_fs_is_rw(fs) ) return whefs_rc.OK;
    if( fs->fileno < 1 ) return whefs_rc.UnsupportedError;
    /**
       HOLY FARGING SHITE! What a speed difference mmap() makes!!!

       Here we do a bit of trickery: we swap out fs->dev with a proxy
       device which mmap()'s the file.
    */
    static whio_dev_api whio_dev_api_mmap = {0};
    static bool doneIt = false;
    if( !doneIt )
    {
        whio_dev_api_mmap = whio_dev_api_memmap;
        whio_dev_api_mmap.flush = whio_dev_mmap_flush;
        whio_dev_api_mmap.close = whio_dev_mmap_close;
        doneIt = true;
    }
    whio_size_t dsz = whio_dev_size( fs->dev );
    void * m = mmap( 0, dsz, whefs_fs_is_rw(fs) ? PROT_WRITE : PROT_READ, MAP_SHARED, fs->fileno, 0 );
    if( ! m )
    {
        WHEFS_DBG_WARN("mmap() failed for %"WHIO_SIZE_T_PFMT" bytes of fs->fileno (#%d)!",dsz,fs->fileno);
        return whefs_rc.IOError;
    }
    whio_dev * md = whefs_fs_is_rw(fs)
        ? whio_dev_for_memmap_rw( m, dsz )
        : whio_dev_for_memmap_ro( m, dsz );
    if( ! md )
    {
        WHEFS_DBG_WARN("whio_dev_for_memmap_%s() failed for %"WHIO_SIZE_T_PFMT" bytes of fs->fileno (#%d)!",
                       (whefs_fs_is_rw(fs) ? "rw" : "ro"), dsz,fs->fileno);
        return whefs_rc.IOError;
    }
    md->api = &whio_dev_api_mmap;
    WhioDevMMapInfo * minfo = (WhioDevMMapInfo*)malloc(sizeof(WhioDevMMapInfo)); // FIXME: add static allocator for WhioDevMMApInfo
    if( ! minfo )
    {
        WHEFS_DBG_ERR("Allocation of %u bytes for WhioDevMMapInfo failed!",sizeof(WhioDevMMapInfo));
        md->api->finalize(md);
        return whefs_rc.AllocError;
    }
    md->client.data = minfo;
    minfo->size = dsz;
    minfo->fileno = fs->fileno;
    minfo->mem = m;
    minfo->fdev = fs->dev;
    minfo->writeMode = whefs_fs_is_rw(fs);
    minfo->async = WHEFS_CONFIG_ENABLE_MMAP_ASYNC ? true : false;
    fs->dev = md;
    fs->flags |= WHEFS_FLAG_FS_IsMMapped;
    WHEFS_DBG_FYI("Swapped out EFS file-based whio_dev with mmap() wrapper! Flushing in %s mode.",
                  WHEFS_CONFIG_ENABLE_MMAP_ASYNC ? "asynchronous" : "synchronous");
    return whefs_rc.OK;
#endif
}

/**
   If !WHEFS_CONFIG_ENABLE_MMAP this simply returns
   whefs_rc.UnsupportedError, otherwise:

   If fs->dev is a mmap() device proxy then it is removed and fs->dev
   is redirected to the non-proxy device. This is necessary before
   certain operations, namely a truncate() on an mmap()'d file.

   We should be able to add a whio_dev::truncate() impl to our proxy which
   could take care of re-mmap()ing for us.
*/
static int whefs_fs_mmap_disconnect( whefs_fs * fs )
{
#if ! WHEFS_CONFIG_ENABLE_MMAP
    return whefs_rc.UnsupportedError;
#else
    if( ! fs ) return whefs_rc.ArgError;
    if( ! (fs->flags & WHEFS_FLAG_FS_IsMMapped) ) return whefs_rc.OK;
    if( fs->fileno < 1 ) return whefs_rc.InternalError;
    if( fs->dev->impl.typeID != &whio_dev_api_memmap ) return whefs_rc.InternalError;
    // We appear to have an mmap() proxy in place...
    WhioDevMMapInfo * m = (WhioDevMMapInfo *)fs->dev->client.data;
    whio_dev * dx = m->fdev;
    m->fdev = 0;
    fs->dev->api->finalize(fs->dev);
    fs->dev = dx;
    WHEFS_DBG_FYI("Disconnected mmap() proxy. Restored fs->dev to %p.",
                  (void const *)fs->dev );
    return whefs_rc.OK;
#endif
}


whio_size_t whefs_fs_read( whefs_fs * restrict fs, void * dest, whio_size_t n )
{
    return (fs && fs->dev)
	? fs->dev->api->read( fs->dev, dest, n )
	: 0;
}
whio_size_t whefs_fs_write( whefs_fs * restrict fs, void const * src, whio_size_t n )
{
    return (fs && fs->dev)
	? fs->dev->api->write( fs->dev, src, n )
	: 0;
}

whio_size_t whefs_fs_writeat( whefs_fs * fs, whio_size_t pos, void const * src, whio_size_t n )
{
    whio_size_t x = whefs_fs_seek( fs, (off_t)pos, SEEK_SET );
    if( x != pos ) return 0;
    return whefs_fs_write( fs, src, n );
}

whio_size_t whefs_fs_readat( whefs_fs * fs, whio_size_t pos, void * dest, whio_size_t n )
{
    whio_size_t x = whefs_fs_seek( fs, (off_t)pos, SEEK_SET );
    if( x != pos ) return 0;
    return whefs_fs_read( fs, dest, n );
}

whio_size_t whefs_fs_seek( whefs_fs * restrict fs, off_t offset, int whence )
{
    return (fs && fs->dev)
	? fs->dev->api->seek( fs->dev, offset, whence )
	: whio_rc.SizeTError;
}

whio_size_t whefs_fs_tell( whefs_fs * restrict fs )
{
    return (fs && fs->dev)
	? fs->dev->api->tell( fs->dev )
	: whio_rc.SizeTError;
}

int whefs_fs_flush( whefs_fs * restrict fs )
{
    if(fs && fs->dev)
    {
        if( whefs_fs_is_rw(fs) )
        {
            return fs->dev->api->flush( fs->dev );
        }
        return whefs_rc.AccessError;
    }
    else
    {
        return whefs_rc.ArgError;
    }
}


typedef enum
{
WHEFS_ERR_None = 0x0000,
WHEFS_ERR_IO   = 0x10000000,
WHEFS_ERR_IO_READ   = WHEFS_ERR_IO | 0x01,
WHEFS_ERR_IO_WRITE   = WHEFS_ERR_IO | 0x02
} whefs_errors;

void whefs_fs_caches_names_clear( whefs_fs * restrict fs )
{
    if( fs->cache.hashes )
    {
        WHEFS_DBG_CACHE("Emptying names hash cache using %"WHEFS_ID_TYPE_PFMT" of %"WHEFS_ID_TYPE_PFMT" entries and %u bytes.",
                        fs->cache.hashes->count,
                        fs->cache.hashes->alloced,
                        whefs_hashid_list_sizeof(fs->cache.hashes) );
#if 0
        whefs_hashid * h = 0;
	whefs_id_type i;
        for( i = 0; i < fs->cache.hashes->count; ++i )
        {
            h = &fs->cache.hashes->list[i];
            WHEFS_DBG_CACHE("inode #%"WHEFS_ID_TYPE_PFMT" cached entry.", h->id);
        }
#endif
        whefs_hashid_list_alloc( &fs->cache.hashes, 0 );// reminder: we keep fs->cache.hashes itself until finalization.
    }
}
void whefs_fs_caches_clear( whefs_fs * restrict fs )
{
    whbits_free_bits( &fs->bits.i );
    whbits_free_bits( &fs->bits.b );
    whefs_fs_caches_names_clear( fs );
}

void whefs_fs_finalize( whefs_fs * restrict fs )
{
    if( ! fs ) return;
    whefs_fs_flush(fs);
    whefs_fs_mmap_disconnect( fs );
    whefs_fs_hints_write( fs );
    if( fs->closers )
    {
        if( ! (fs->flags & WHEFS_FLAG_FS_NoAutoCloseFiles) )
        {
            WHEFS_DBG_WARN("We're closing with opened objects! Closing them...");
            while( fs->closers->prev )
            {
                fs->closers = fs->closers->prev;
            }
            whefs_fs_closer_list * x = fs->closers;
            fs->closers = 0; // we must do this b/c whefs_fs_closer_list_close() will indirectly use this list.
            whefs_fs_closer_list_close( x, true );
        }
        else
        {
            WHEFS_DBG_WARN("We're closing with opened objects and auto-close of files is disabled! They are leaking and MUST NOT be properly destroyed now!");
            whefs_fs_closer_list_free( fs->closers );
        }
        fs->closers = 0;
    }
    if( fs->opened_nodes )
    {
	WHEFS_DBG_WARN("We're closing with opened inodes! Closing them...");
        while( fs->opened_nodes )
        {
            WHEFS_DBG_WARN("Auto-closing inode #%"WHEFS_ID_TYPE_PFMT", but leaking its whefs_file, whio_dev, or whio_stream handle (if any).",fs->opened_nodes->inode.id);
            whefs_inode_close( fs, &fs->opened_nodes->inode, fs->opened_nodes->inode.writer );
            // reminder: whefs_inode_close() updates fs->opened_nodes directly.
	}
        // this doesn't stop us from leaking unclosed whefs_file/whio_dev/whio_stream handles!
    }
    whefs_fs_caches_clear(fs);
    whefs_fs_setopt_hash_cache( fs, false, false );
    if( fs->dev )
    {
        /**
           Philosophical problem: because finalizing fs->dev
           might flush the device, *should* finalize fs->dev
           BEFORE we unlock. However, we cannot unlock once
           fs->dev is destroyed.
        */
        whefs_fs_unlock( fs, 0, SEEK_SET, 0 );
	if( fs->ownsDev ) fs->dev->api->finalize( fs->dev );
	fs->dev = 0;
    }
    whefs_fs_free( fs );
}

whefs_fs_options const * whefs_fs_options_get( whefs_fs const * restrict fs )
{
    return fs ? &fs->options : 0;
}
whefs_fs_options const * whefs_fs_opt( whefs_fs const * restrict fs )
{
    return fs ? &fs->options : 0;
}


/**
   Seeks to the start of fs, writes the magic bytes. Returns
   whefs_rc.OK on success.
*/
static int whefs_mkfs_write_magic( whefs_fs * restrict fs )
{
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;
    /*
      TODO: encode the butes in a buffer and only do one write().

      TODO: double-check the expected positions again:

      fs->offsets[WHEFS_OFF_CORE_MAGIC]
      fs->offsets[WHEFS_OFF_CLIENT_MAGIC]
      fs->offsets[WHEFS_OFF_SIZE]
    */
    fs->dev->api->seek( fs->dev, 0L, SEEK_SET );
    whio_dev_encode_uint32_array( fs->dev, whefs_fs_magic_bytes_len, whefs_fs_magic_bytes );
    whio_dev_encode_uint32( fs->dev, fs->filesize /*will be overwritten at end of mkfs*/ );
    whio_dev_encode_uint16( fs->dev, fs->options.magic.length );
    const whio_size_t wrc = whio_dev_write( fs->dev, fs->options.magic.data, fs->options.magic.length );
    return (wrc == fs->options.magic.length)
	? whefs_rc.OK
	: whefs_rc.IOError;
}


static const unsigned char whefs_hints_tag_char = 'H';
int whefs_fs_hints_write( whefs_fs * restrict fs )
{
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;
    if( !whefs_fs_is_rw(fs) ) return whefs_rc.AccessError;
    fs->dev->api->seek( fs->dev, fs->offsets[WHEFS_OFF_HINTS], SEEK_SET );
    whefs_id_type h[] = {
        fs->hints.unused_inode_start,
        fs->hints.unused_block_start
    };
    const whefs_id_type hlen = (sizeof(h)/sizeof(h[0]));
    enum { Len = whefs_sizeof_encoded_hints };
    unsigned char buf[Len];
    unsigned char * bp = buf;
    *(bp++) = whefs_hints_tag_char;
    for( whefs_id_type i = 0; i < hlen; ++i )
    {
        bp += whefs_id_encode( bp, h[i] );
    }
    const whio_size_t wrc = whio_dev_write( fs->dev,
                                            buf,
                                            Len );
    return fs->err =
        (wrc == Len)
	? whefs_rc.OK
	: whefs_rc.IOError;
}
int whefs_fs_hints_read( whefs_fs * restrict fs )
{
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;
    whio_size_t sk = fs->dev->api->seek( fs->dev, fs->offsets[WHEFS_OFF_HINTS], SEEK_SET );
    if( sk != fs->offsets[WHEFS_OFF_HINTS] )
    {
        return fs->err = whefs_rc.IOError;
    }
    enum { Len = whefs_sizeof_encoded_hints };
    unsigned char buf[Len+1];
    unsigned char * bp = buf;
    if( Len != whio_dev_read( fs->dev, buf, Len ) )
    {
        return fs->err = whefs_rc.IOError;
    }
    buf[Len] = 0;
    if( whefs_hints_tag_char != *(bp++) )
    {
        return fs->err = whefs_rc.ConsistencyError;
    }
    int rc = whefs_id_decode( bp, &fs->hints.unused_inode_start );
    if( whefs_rc.OK == rc )
    {
        bp += whefs_sizeof_encoded_id_type;
        rc = whefs_id_decode( bp, &fs->hints.unused_block_start );
        if( whefs_rc.OK != rc ) fs->err = rc;
    }
    return rc;
}

/**
   Writes fs->options to the current position of the stream.  Returns
   whefs_rc.OK on success.
*/
static int whefs_mkfs_write_options( whefs_fs * restrict fs )
{
    whefs_fs_seek( fs, fs->offsets[WHEFS_OFF_OPTIONS], SEEK_SET );
    assert( fs->dev->api->tell( fs->dev ) == fs->offsets[WHEFS_OFF_OPTIONS] );
    size_t pos = whio_dev_encode_uint32( fs->dev, fs->options.block_size );
    size_t sz = whefs_dev_id_encode( fs->dev, fs->options.block_count );
    if( whefs_sizeof_encoded_id_type != sz ) return whefs_rc.IOError;
    sz = whefs_dev_id_encode( fs->dev, fs->options.inode_count );
    if( whefs_sizeof_encoded_id_type != sz ) return whefs_rc.IOError;
    pos += whio_dev_encode_uint16( fs->dev, fs->options.filename_length );
    return (pos>0) /* <--- this is not technically correct. */
	? whefs_rc.OK
	: whefs_rc.IOError;
}

/**
   Returns the on-disk size of whefs_fs_options objects.
*/
static size_t whefs_fs_sizeof_options()
{
    const size_t sz = whio_sizeof_encoded_uint32;
    return sz /* block_size */
	+ whefs_sizeof_encoded_id_type /* block_count */
	+ whefs_sizeof_encoded_id_type /* inode_count */
	+ whio_sizeof_encoded_uint16 /* filename_length */
	;
}

/**
   Returns the on-disk size of inode names for the given
   whefs_fs_options object.
*/
size_t whefs_fs_sizeof_name( whefs_fs_options const * opt )
{
    return !opt
	? 0
	: (whefs_sizeof_encoded_inode_name_header +  opt->filename_length);
}


#if 0 // not needed anymore?
/**
   Returns the on-disk position of the given inode's name record,
   or 0 if id is not valid for fs.
*/
static size_t whefs_name_pos( whefs_fs const * restrict fs, whefs_id_type id )
{
    if( ! whefs_inode_id_is_valid( fs, id ) )
    {
	return 0;
    }
    else
    {
	return fs->offsets[WHEFS_OFF_INODE_NAMES]
	    + ( (id-1) * fs->sizes[WHEFS_SZ_INODE_NAME] );
    }
}
#endif

/**
   Tag byte for use in encoding inode name table entries.
*/
static unsigned char const whefs_inode_name_tag_char = '"';

int whefs_inode_name_get( whefs_fs * restrict fs, whefs_id_type id, whefs_string * tgt )
{ /* Maintenance reminder: this "should" be in whefs_inode.c, but it's not because
     of whefs_inode_name_tag_char.
   */
    if( ! tgt || ! whefs_inode_id_is_valid( fs, id ) ) return whefs_rc.ArgError;
    assert(fs->sizes[WHEFS_SZ_INODE_NAME] && "fs has not been set up properly!");
    int rc = 0;
    enum { bufSize = whefs_sizeof_encoded_inode_name };
    unsigned char buf[bufSize + 1];
    memset( buf, 0, bufSize + 1 );
    whio_size_t const toRead = whefs_fs_sizeof_name(&fs->options);
    assert( toRead <= bufSize );
    whio_size_t spos = fs->offsets[WHEFS_OFF_INODE_NAMES]
        + (fs->sizes[WHEFS_SZ_INODE_NAME] * (id-1));
    whio_size_t rsz = whefs_fs_readat( fs, spos, buf, toRead );
    if( toRead != rsz )
    {
	WHEFS_DBG_ERR("Error #%d reading inode #"WHEFS_ID_TYPE_PFMT"'s name record!",rc,id);
	return whefs_rc.IOError;
    }
    //unsigned char const * buf = fs->buffers.nodeName;
    if( buf[0] != whefs_inode_name_tag_char )
    {
	WHEFS_DBG_ERR("Error reading inode #%"WHEFS_ID_TYPE_PFMT"'s name record! "
		      "Expected byte value 0x%02x but got 0x%02x",
		      id, whefs_inode_name_tag_char, buf[0] );
	return whefs_rc.ConsistencyError;
    }
    unsigned char * bufP = buf;
    ++bufP; // skip tag byte
    bufP += whefs_sizeof_encoded_id_type //skip id field
        //+ whio_sizeof_encoded_uint64 //skip hash field
        ;
    uint16_t sl = 0; // string length
    rc = whio_decode_uint16( bufP, &sl );
    if( whio_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Could not decode string length token from inode #"WHEFS_ID_TYPE_PFMT"'s "
		      "name record! RC=%d",id,rc);
	return rc;
    }
    bufP += whio_sizeof_encoded_uint16; // skip over size field
    //bufP += whio_sizeof_encoded_uint64; // skip hash field
    rc = whefs_string_copy_cstring( tgt, (char const *)bufP );
    if( whio_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Copying of inode #"WHEFS_ID_TYPE_PFMT"'s name record failed! "
		      "RC=%d. String is [%s]",
		      id, rc, bufP );
    }
    if( tgt->length )
    {
        //WHEFS_DBG("Caching inode name [%s]",tgt->string);
        whefs_inode_hash_cache( fs, id, tgt->string );
        // A necessary evil to avoid lots of O(N) searches.
        if( fs->cache.hashes && ! fs->cache.hashes->skipAutoSort )
        {
            whefs_inode_hash_cache_sort(fs);
        }
    }
    return rc;
}

int whefs_fs_name_write( whefs_fs * restrict fs, whefs_id_type id, char const * name )
{
    if( ! whefs_inode_id_is_valid( fs, id ) || !name)
    {
	return whefs_rc.ArgError;
    }
    uint16_t slen = 0;
    {
	char const * c = name;
	uint16_t i = 0;
	for( ; c && *c && (i < fs->options.filename_length); ++i, ++c, ++slen )
	{
	}
	if( (i == fs->options.filename_length) && *c )
	{ /** too long! */
	    return whefs_rc.RangeError;
	}
    }
    int rc = 0;
    /**
       Encode the string to a temp buffer then write it in one go to
       disk. Takes more code than plain i/o, but using this approach
       here means much less overall i/o and requies less error
       handling for the encoding (which can't fail as long as we
       provide the proper parameters and memory buffer sizes).
    */
    const size_t bsz =
        fs->sizes[WHEFS_SZ_INODE_NAME]
        ;
    assert(fs->sizes[WHEFS_SZ_INODE_NAME] && "fs has not been set up properly!");
    assert( bsz == fs->sizes[WHEFS_SZ_INODE_NAME] );
    //    unsigned char * buf = fs->buffers.nodeName;
    enum { bufSize = whefs_sizeof_encoded_inode_name };
    unsigned char buf[bufSize+1];
    memset( buf+1, 0, bufSize );
    buf[0] = whefs_inode_name_tag_char;
    size_t off = 1;
    off += whefs_id_encode( buf + off, id );
    off += whio_encode_uint16( buf + off, slen );
    //uint64_t shash = 0UL; //whefs_bytes_hash( name, slen );
    //off += whefs_uint64_encode( buf + off, shash );
    memcpy( buf + off, name, slen );
    unsigned char const * dbgStr = buf+off;
    off += slen;
    if( off < bsz ) memset( buf + off, 0, bsz - off );
    assert( off <= bsz );
    const off_t spos = fs->offsets[WHEFS_OFF_INODE_NAMES] +
        (bsz * (id-1));
    whio_size_t const sz = whefs_fs_writeat( fs, spos, buf, bsz );
    if( bsz != sz )
    {
	WHEFS_DBG_ERR("Writing inode #%"WHEFS_ID_TYPE_PFMT"[%s] name failed! rc=%d bsz=%u",id,name,rc,bsz);
	return whefs_rc.IOError;
    }
    if( 0 && *name )
    {
	WHEFS_DBG("Writing inode #%"WHEFS_ID_TYPE_PFMT"[%s] name: [%s]",id,name,dbgStr);
    }
    return whefs_rc.OK;

}


/**
   Writes the inode names table to pos fs->offsets[WHEFS_OFF_INODE_NAMES].
   Returns whefs_rc.OK on success.
*/
static int whefs_mkfs_write_names_table( whefs_fs * restrict fs )
{
    whefs_id_type i = 1;
    int rc = whefs_rc.OK;
    whefs_fs_seek( fs, fs->offsets[WHEFS_OFF_INODE_NAMES], SEEK_SET );
    assert( fs->dev->api->tell( fs->dev ) == fs->offsets[WHEFS_OFF_INODE_NAMES] );
    char empty[1] = {'\0'};
    for( ; (i <= fs->options.inode_count) && (whefs_rc.OK == rc); ++i )
    {
	rc = whefs_fs_name_write( fs, i, empty );
    }
    if( whefs_rc.OK == rc )
    {
	// Unfortunate workaround for expectations of mkfs...
	whefs_fs_seek( fs, fs->offsets[WHEFS_OFF_INODES_NO_STR], SEEK_SET );
    }
    return rc;
}



whio_size_t whefs_fs_calculate_size( whefs_fs_options const * opt )
{
    static const whio_size_t sz = (whio_size_t)whio_sizeof_encoded_uint32;
    if( ! opt ) return 0;
    else return (whio_size_t)(
	(whio_sizeof_encoded_uint32 * whefs_fs_magic_bytes_len) // core magic
	+ sz // file size header
	+ whio_sizeof_encoded_uint16 // client magic size
	+ opt->magic.length
	+ whefs_fs_sizeof_options()
        + whefs_sizeof_encoded_hints
	+ (whefs_fs_sizeof_name( opt ) * opt->inode_count)/* inode names table */
	+ (whefs_sizeof_encoded_inode * opt->inode_count) /* inode table */
	+ (whefs_fs_sizeof_block( opt ) * opt->block_count)/* blocks table */
	);
}


/**
   Writes all (empty) blocks of fs to pos
   fs->offsets[WHEFS_OFF_BLOCKS] of the data store.  Returns
   whefs_rc.OK on success.
*/
static int whefs_mkfs_write_blocklist( whefs_fs * restrict fs )
{
    whefs_id_type i = 0;
    int rc = whefs_rc.OK;
    whefs_fs_seek( fs, fs->offsets[WHEFS_OFF_BLOCKS], SEEK_SET );
    assert( fs->dev->api->tell( fs->dev ) == fs->offsets[WHEFS_OFF_BLOCKS] );
    whefs_block bl = whefs_block_empty;
    for( i = 1; (i <= fs->options.block_count) && (whefs_rc.OK == rc); ++i )
    {
	bl.id = i;
	if( whefs_rc.OK != (rc = whefs_block_wipe( fs, &bl, true, true, false )) )
	{
	    WHEFS_DBG_ERR("Error %d while writing the block table!", rc);
	    break;
	}
    }
    return rc;
}

/**
   Writes all (empty) inodes to the current position of
   fs->dev. Returns whefs_rc.OK on success.
*/
static int whefs_mkfs_write_inodelist( whefs_fs * restrict fs )
{
    whefs_fs_seek( fs, fs->offsets[WHEFS_OFF_INODES_NO_STR], SEEK_SET );
    assert( fs->dev->api->tell( fs->dev ) == fs->offsets[WHEFS_OFF_INODES_NO_STR] );
    size_t i = 0;
    int rc = whefs_rc.OK;
    whefs_inode node = whefs_inode_empty;
    enum { bufSize = whefs_sizeof_encoded_inode };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );
    for( i = 1; (i <= fs->options.inode_count) && (whefs_rc.OK == rc); ++i )
    {
	node.id = i;
	rc = whefs_inode_encode( &node, buf );
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("Error #%d while encoding new-style inode #%"WHEFS_ID_TYPE_PFMT"!",
			  rc,i);
	    return rc;
	}
        whio_size_t check = whefs_fs_writeat( fs, whefs_inode_id_pos( fs, i ), buf, bufSize );
        if( check != bufSize )
        {
            rc = whefs_rc.IOError;
	    WHEFS_DBG_ERR("Error #%d while writing inode #%"WHEFS_ID_TYPE_PFMT"!",
			  rc, i);
            break;
        }
    }
    return rc;
}

/**
   Uses whio_dev_ioctl() to try to get a file descriptor number
   associated with fs->dev. If it succeeds we store the descriptor
   so we can later use it to implement file locking.
*/
static void whefs_fs_check_fileno( whefs_fs * restrict fs )
{
    if( !fs || !fs->dev ) return;
    char const * fname = 0;
    whio_dev_ioctl( fs->dev, whio_dev_ioctl_GENERAL_name, &fname );
    if( whio_rc.OK != whio_dev_ioctl( fs->dev, whio_dev_ioctl_FILE_fd, &fs->fileno ) )
    {
        //WHEFS_DBG("Backing store does not appear to be a FILE." );
        return;
    }
    //WHEFS_DBG_FYI("Backing store appears to be a FILE (named [%s]) with descriptor #%d.", fname, fs->fileno );
    //posix_fadvise( fs->fileno, 0L, 0L, POSIX_FADV_RANDOM );
}

/**
   Opens filename in read-write mode (if writeMode is true) or read-only mode
   (if writeMode is 0) and assigns fs->dev to that device. Returns the new
   whio_dev object, which is owned by fs. Any existing fs->dev object
   is destroyed. On error, 0 is returned.

   If createIt is true and writeMode is true and we cannot open the
   file in "r+" mode then it will be created using "w+" mode. createIt
   is ignored if !writeMode or if the file can be opened using "r+"
   mode.
*/
static whio_dev * whefs_open_FILE( char const * filename, whefs_fs * restrict fs,
                                   bool writeMode, bool createIt )
{
    if( ! filename || !fs )
    {
        WHEFS_DBG_WARN("! filename || !fs");
        return 0;
    }
    if( fs->dev )
    {
        fs->dev->api->finalize( fs->dev );
        fs->dev = 0;
    }
    fs->dev = whio_dev_for_filename( filename, writeMode ? "r+b" : "rb" );
    WHEFS_DBG_WARN("whio_dev_for_filename %d", fs->dev);
    if( writeMode && createIt && !fs->dev )
    { /* didn't exist (we assume), so try to create it */
        WHEFS_DBG_WARN("Opening [%s] with 'r+' failed. Trying 'w+'...", filename );
        fs->dev = whio_dev_for_filename( filename, "w+b" );
    }
    if( ! fs->dev ) return 0;
    whefs_fs_check_fileno( fs );
    int lk = whefs_fs_lock( fs, writeMode, 0, SEEK_SET, 0 );
    if( (whefs_rc.OK == lk) || (whefs_rc.UnsupportedError == lk) )
    {
        // okay
    }
    else
    {
        fs->dev->api->finalize( fs->dev );
        fs->dev = 0;
        WHEFS_DBG_WARN("whefs_fs_lock %d", lk);
    }
    return fs->dev;
}

static int whefs_fs_init_bitset_inodes( whefs_fs * restrict fs )
{
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
    if( ! fs ) return whefs_rc.ArgError;
    const size_t nbits = 1; /* flags: used */
    const size_t nbc = fs->options.inode_count * nbits + 1; /* +1 b/c we use the ID (1...N) as bit address */
    //WHEFS_DBG("Initializing bitsets. Node count/bits/bytes=%u/%u/%u", fs->options.inode_count, nbc, nbytes  );
    if( 0 != whbits_init( &fs->bits.i, nbc, 0 ) ) return whefs_rc.AllocError;
#if 0
    WHEFS_DBG("Initialized inode bitset. Node count/bits/bytes=%u/%u/%u",
	      fs->options.inode_count, fs->bits.i.sz_bits, fs->bits.i.sz_bytes );
#endif
    WHEFS_ICACHE_SET_USED(fs,0); /* inode ID 0 is reserved for "not a node". */
    WHEFS_ICACHE_SET_USED(fs,1); /* inode ID 1 is reserved for the root node. */
#endif /* WHEFS_CONFIG_ENABLE_BITSET_CACHE */
    return whefs_rc.OK;
}

static int whefs_fs_init_bitset_blocks( whefs_fs * restrict fs )
{
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
    //WHEFS_DBG("Setting up bitsets for fs@0x%p", (void const *)fs );
    if( ! fs ) return whefs_rc.ArgError;
    const size_t bbits = 1; /* flag: used */
    const size_t bbc = fs->options.block_count * bbits + 1; /* +1 b/c we use the ID (1...N) as bit address */
    //WHEFS_DBG("Initializing bitsets. block count/bits/bytes=%u/%u/%u", fs->options.block_count, bbc, bbytes  );
    if( 0 != whbits_init( &fs->bits.b, bbc, 0 ) ) return whefs_rc.AllocError;
#if 0
    WHEFS_DBG("Initialized bitsets. block count/bits/bytes=%u/%u/%u",
	      fs->options.block_count, fs->bits.b.sz_bits, fs->bits.b.sz_bytes );
#endif
    WHEFS_ICACHE_SET_USED(fs,0); /* inode ID 0 is reserved for "not a block". */
#endif /* WHEFS_CONFIG_ENABLE_BITSET_CACHE */
    return whefs_rc.OK;
}

/**
   Initializes the internal bitset caches. It must not be called
   before fs has a device and a filesystem has been opened. If called
   more than once for the same fs, fs->bits.i and fs->bits.b will get
   reallocated to fit any changes in fs->options.inode_count and
   fs->options.block_count. Does not do any I/O but has to alloc
   memory.

   Returns whefs_rc.OK on success.
*/
static int whefs_fs_init_bitsets( whefs_fs * restrict fs )
{
    int rc = whefs_fs_init_bitset_inodes(fs);
    if( whefs_rc.OK == rc ) rc = whefs_fs_init_bitset_blocks(fs);
    return rc;
}

/**
   Initializes the internal inode cache, reading the state from
   storage.

   Returns whefs_rc.OK on success.
*/
static int whefs_fs_inode_cache_load( whefs_fs * restrict fs )
{
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
    WHEFS_DBG_CACHE("Loading inode bitset cache.");
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;
    //return 0;
    const whefs_id_type nc = whefs_fs_options_get(fs)->inode_count;
    whefs_id_type id = 2; // root node (id=1) is always considered used
    int rc = 0;
    uint32_t flags;
    //whefs_id_type count = 0;
    whefs_inode ino = whefs_inode_empty;
    for( ; id <= nc; ++id )
    {
	//WHEFS_DBG("Trying to cache inode #%"WHEFS_ID_TYPE_PFMT"'s state.", id);
	flags = 0;
	rc = whefs_inode_id_read( fs, id, &ino );
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("Error #%d while reading inode #%"WHEFS_ID_TYPE_PFMT"!", rc, id);
	    return rc;
	}
	if( ino.flags & WHEFS_FLAG_Used )
	{
	    //++count;
	    WHEFS_ICACHE_SET_USED(fs,id);
	}
	else
	{
	    WHEFS_ICACHE_UNSET_USED(fs,id);
	}
    }
    fs->bits.i_loaded = true;
    //WHEFS_DBG_FYI("Initialized inode cache (entries: %u)", count);
#endif /* WHEFS_CONFIG_ENABLE_BITSET_CACHE */
    return whefs_rc.OK;
}

/**
   Loads the block cache. Returns whefs_rc.OK on success,
*/
static int whefs_fs_block_cache_load( whefs_fs * restrict fs )
{
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
    /* FIXME: instead of using whefs_block_read(), simply extract the
       flags field from each block entry. That'll be much faster. */
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;
    //return 0;
    const size_t bc = whefs_fs_options_get(fs)->block_count;
    size_t id = 1;
    int rc = 0;
    whefs_block bl;
    for( ; id <= bc; ++id )
    {
	bl = whefs_block_empty;
	rc = whefs_block_read( fs, id, &bl ); /* this will update the used-blocks cache */
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("Error #%d while reading block #%u!", rc, id);
	    return rc;
	}
	//WHEFS_DBG("block cache. #%u is %s",bl.id, (WHEFS_BCACHE_IS_USED(fs,id) ? "used" : "unused") );
    }
    //WHEFS_DBG("Initialized block cache.");
    fs->bits.b_loaded = true;
#endif /* WHEFS_CONFIG_ENABLE_BITSET_CACHE */
    return whefs_rc.OK;
}

bool whefs_fs_is_rw( whefs_fs const * restrict fs )
{
    return fs && (fs->flags & WHEFS_FLAG_Write);
}
/**
   Loads the inode and block caches. Returns whefs_rc.OK on success,
*/
#if WHEFS_LOAD_CACHES_ON_OPEN
static
#endif
int whefs_fs_caches_load( whefs_fs * restrict fs )
{
    int rc = whefs_fs_inode_cache_load( fs );
    if( whefs_rc.OK == rc ) rc = whefs_fs_block_cache_load( fs );
    if( whefs_rc.OK == rc )
    {
        rc = whefs_fs_setopt_hash_cache( fs, WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE ? true : false, false );
    }
    return rc;
}

/**
   Initializes fs->sizes[] and fs->offsets[]. fs->options must have
   been populated in order for this to work.
*/
static void whefs_fs_init_sizes( whefs_fs * restrict fs )
{
    fs->sizes[WHEFS_SZ_INODE_NO_STR] = whefs_sizeof_encoded_inode;
    fs->sizes[WHEFS_SZ_INODE_NAME] = whefs_fs_sizeof_name( &fs->options );
    fs->sizes[WHEFS_SZ_BLOCK] = whefs_fs_sizeof_block( &fs->options );
    fs->sizes[WHEFS_SZ_OPTIONS] = whefs_fs_sizeof_options();
    fs->sizes[WHEFS_SZ_HINTS] = whefs_sizeof_encoded_hints;
    fs->offsets[WHEFS_OFF_CORE_MAGIC] = 0;

    size_t sz =	/* core magic len */
	(whio_sizeof_encoded_uint32 * whefs_fs_magic_bytes_len);

    fs->offsets[WHEFS_OFF_SIZE] =
	fs->offsets[WHEFS_OFF_CORE_MAGIC]
	+ sz;
    sz = /* file size */
	whio_sizeof_encoded_uint32;

    fs->offsets[WHEFS_OFF_CLIENT_MAGIC] =
	fs->offsets[WHEFS_OFF_SIZE]
	+ sz;
    sz = /* client magic size */
	whio_sizeof_encoded_uint16 // length
	+ fs->options.magic.length;

    fs->offsets[WHEFS_OFF_OPTIONS] =
	fs->offsets[WHEFS_OFF_CLIENT_MAGIC]
	+ sz;

    fs->offsets[WHEFS_OFF_HINTS] =
        fs->offsets[WHEFS_OFF_OPTIONS]
        + fs->sizes[WHEFS_SZ_OPTIONS];

    fs->offsets[WHEFS_OFF_INODE_NAMES] =
	fs->offsets[WHEFS_OFF_HINTS]
	+ fs->sizes[WHEFS_SZ_HINTS];
    sz = /* names table size */
	(fs->options.inode_count * fs->sizes[WHEFS_SZ_INODE_NAME]);

    fs->offsets[WHEFS_OFF_INODES_NO_STR] =
	fs->offsets[WHEFS_OFF_INODE_NAMES]
	+ sz;
    sz = /* new inodes table size */
	(fs->options.inode_count * fs->sizes[WHEFS_SZ_INODE_NO_STR]);

    //fs->offsets[WHEFS_OFF_BLOCK_TABLE] NYI

    fs->offsets[WHEFS_OFF_BLOCKS] =
	fs->offsets[WHEFS_OFF_INODES_NO_STR]
	+ sz;
    sz = /* blocks table size */
	(fs->options.block_count * fs->sizes[WHEFS_SZ_BLOCK]);

    fs->offsets[WHEFS_OFF_EOF] =
	fs->offsets[WHEFS_OFF_BLOCKS]
	+ sz;

#if 0
    fprintf( stdout, "\tOffsets:\n");
#define OFF(X) fprintf(stdout,"\t\tfs->offsets[%s]\t= %u\n",# X, fs->offsets[WHEFS_OFF_ ## X])
    OFF(CORE_MAGIC);
    OFF(SIZE);
    OFF(CLIENT_MAGIC);
    OFF(OPTIONS);
    OFF(HINTS);
    OFF(INODE_NAMES);
    OFF(INODES_NO_STR);
    OFF(BLOCKS);
    OFF(EOF);
#undef OFF
#define OFF(X) fprintf(stdout,"\t\tfs->sizes[%s]\t= %u\n",# X, fs->sizes[WHEFS_SZ_ ## X])
    OFF(INODE_NO_STR);
    OFF(INODE_NAME);
    OFF(BLOCK);
    OFF(OPTIONS);
    OFF(HINTS);
    fflush(stdout);
    assert(0 && "on purpose");
#undef OFF
#endif
}


/**
   Sets up a new, whefs_fs object an initializes the parts which depend only
   on the options. On success tgt is assigned to the new device and whefs_rc.OK
   is returned. On error, tgt is not modified and some other value is returned.
*/
static int whefs_mkfs_stage1( whefs_fs_options const * opt, whefs_fs ** tgt )
{
    if( !opt || !tgt ) return whefs_rc.ArgError;
    if( (opt->inode_count < 2)
	|| (opt->block_size < 32)
	|| !opt->filename_length
	|| (opt->filename_length > WHEFS_MAX_FILENAME_LENGTH)
	|| !opt->magic.length
	|| !opt->magic.data
	|| (opt->block_count < opt->inode_count)
	)
    {
	return whefs_rc.RangeError;
    }
    whefs_fs * fs = whefs_fs_alloc();
    if( ! fs ) return whefs_rc.AllocError;
    *fs = whefs_fs_empty;
    fs->flags |= WHEFS_FLAG_ReadWrite;
    fs->options = *opt;

    whefs_fs_init_sizes( fs );

    int rc = whefs_fs_init_bitsets( fs );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Init of bitsets failed with rc %d!", rc);
	whefs_fs_finalize( fs );
	return rc;
    }
    *tgt = fs;

    return whefs_rc.OK;
}

/**
   Moves the seek position to fs->offsets[WHEFS_OFF_SIZE] and
   writes fs->filesize to that location.
*/
static int whefs_fs_write_filesize( whefs_fs * restrict fs )
{
    fs->dev->api->seek( fs->dev, fs->offsets[WHEFS_OFF_SIZE], SEEK_SET );
    const whio_size_t ck = whio_dev_encode_uint32( fs->dev, fs->filesize );
    return ( whio_sizeof_encoded_uint32 == ck )
        ? whefs_rc.OK
        : whefs_rc.IOError;
}

/**
   Writes out the disk structures for mkfs. fs->dev must be valid.  On
   success whefs_rc.OK is returned. On error, fs is destroyed and some
   other value is returned.
*/
static int whefs_mkfs_stage2( whefs_fs * restrict fs )
{
    if( ! fs || !fs->dev ) return whefs_rc.ArgError;

    size_t szcheck = whefs_fs_calculate_size(&fs->options);
    //WHEFS_DBG("szcheck = %u", szcheck );

    int rc = fs->dev->api->truncate( fs->dev, szcheck );
    if( whio_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Could not truncate EFS container to %u bytes!", szcheck );
	whefs_fs_finalize( fs );
	return rc;
    }

    //size_t bogo = 0;
#define CHECKRC if( rc != whefs_rc.OK ) { whefs_fs_finalize(fs); return rc; } \
    /*bogo= whio_dev_size( fs->dev ); WHEFS_DBG("device size=[%u]", bogo );*/

    // The following orders are important, as fs->offsets[] is used to confirm
    // expected file positions.
    rc = whefs_mkfs_write_magic( fs );
    CHECKRC;
    rc = whefs_mkfs_write_options( fs );
    CHECKRC;
    rc = whefs_fs_hints_write( fs );
    CHECKRC;
    rc = whefs_mkfs_write_names_table( fs );
    CHECKRC;
    rc = whefs_mkfs_write_inodelist( fs );
    CHECKRC;
    rc = whefs_mkfs_write_blocklist( fs );
    CHECKRC;
#undef CHECKRC
    whefs_fs_flush(fs);
    fs->filesize = whio_dev_size( fs->dev );
    //WHEFS_DBG("File size is(?) %u", fs->filesize );

    //szcheck = whefs_fs_calculate_size(&fs->options);
    if( szcheck != fs->filesize )
    {
	WHEFS_DBG_ERR("EFS size error: the calculated size (%u) does not match the real size (%u)!", szcheck, fs->filesize );
	whefs_fs_finalize( fs );
	return whefs_rc.ConsistencyError;
    }
    fs->offsets[WHEFS_OFF_EOF] = fs->dev->api->tell( fs->dev );
    whefs_fs_write_filesize( fs );

    whefs_fs_flush( fs );
    return whefs_rc.OK;
}

int whefs_mkfs( char const * filename, whefs_fs_options const * opt, whefs_fs ** tgt )
{
    if( ! filename || !opt || !tgt ) return whefs_rc.ArgError;
    whefs_fs * fs = 0;
    int rc = whefs_mkfs_stage1( opt, &fs );
    if( whefs_rc.OK != rc ) return rc;

    if( 0 == strcmp(":memory:",filename) )
    {
	if( ! (fs->dev = whio_dev_for_membuf( whefs_fs_calculate_size(opt), 0 )) )
	{
	    whefs_fs_finalize(fs);
	    return whefs_rc.AllocError; /* we're guessing here. */
	}
    }
    else if( ! whefs_open_FILE( filename, fs, true, true ) )
    {
	whefs_fs_finalize(fs);
	return whefs_rc.AccessError;
    }
    if( ! fs->dev )
    {
	whefs_fs_finalize(fs);
	return whefs_rc.InternalError;
    }
    rc = whefs_mkfs_stage2( fs );
    if( whefs_rc.OK != rc )
    {
	/* fs is already destroyed */
	fs = 0;
    }
    else
    {
        whefs_fs_mmap_connect( fs );
	*tgt = fs;
    }
    return rc;
}

int whefs_mkfs_dev( whio_dev * dev, whefs_fs_options const * opt, whefs_fs ** tgt, bool takeDev )
{
    if( ! dev || !opt || !tgt ) return whefs_rc.ArgError;
    whefs_fs * fs = 0;
    int rc = whefs_mkfs_stage1( opt, &fs );
    if( whefs_rc.OK != rc ) return rc;
    fs->ownsDev = false;
    fs->dev = dev;
    rc = whefs_mkfs_stage2( fs );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("mkfs stage 2 failed with rc %d!",rc);
	/* fs was already destroyed */
	fs = 0;
    }
#if 0
    // FIXME: we don't yet have the in-use cache for devices opened using mkfs!
    /* weird. When i do this here i get read() errors in the file-based i/o handler,
       with read() reporting that errno==EBADF (bad file descriptor)!!!

       Maybe(???) it's because of how we're opening the file (with
       "w+"). The workaround may be to close the EFS and
       re-whefs_fsopen_dev() it???
    */
    rc = whefs_fs_caches_load( fs );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Could not initialize EFS caches! Error code=%d.",rc);
	whefs_fs_finalize( fs );
	fs = 0;
    }
#endif
    if( fs )
    {
	fs->ownsDev = takeDev;
        whefs_fs_mmap_connect( fs );
	*tgt = fs;
    }
    return rc;
}

/**
   Performs the final stage of opening a vfs. fs must have been
   initialized and fs->dev must have been set up.

   On error, fs will be destroyed and non-whefs_rc.OK will be
   returned.

   fs will be modified quite significantly here, but this stuff must
   be called before the vfs can be used.
*/
static int whefs_openfs_stage2( whefs_fs * restrict fs )
{
    if( ! fs ) return whefs_rc.ArgError;

    fs->offsets[WHEFS_OFF_CORE_MAGIC] = 0;
    fs->offsets[WHEFS_OFF_SIZE] = (whefs_fs_magic_bytes_len * whio_sizeof_encoded_uint32);
    int rc = whefs_rc.OK;
    uint32_t coreMagic[whefs_fs_magic_bytes_len];

    while(1)
    {
	if( whefs_fs_magic_bytes_len != whio_dev_decode_uint32_array( fs->dev, whefs_fs_magic_bytes_len, coreMagic ) )
	{
	    WHEFS_DBG_ERR("Error reading the core magic bytes.");
	    rc = whefs_rc.BadMagicError;
	    break;
	}
	//WHEFS_DBG("Core magic = %04u %02u %02u %02u", coreMagic[0], coreMagic[1], coreMagic[2], coreMagic[3] );
	size_t i = 0;
	for( ; i < whefs_fs_magic_bytes_len; ++i )
	{
	    if( coreMagic[i] != whefs_fs_magic_bytes[i] )
	    {
		WHEFS_DBG_ERR("Core magic byte #%u does not match the expected value of 0x%04x (%u).",
			      i, whefs_fs_magic_bytes[i], whefs_fs_magic_bytes[i] );
		rc = whefs_rc.BadMagicError;
		break;
	    }
	}
	break;
    }
    if( rc != whefs_rc.OK )
    {
	whefs_fs_finalize(fs);
	return rc;
    }

    uint32_t fsize = 0;
    rc = whio_dev_decode_uint32( fs->dev, &fsize );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Doesn't seem to be a whefs file! error code=%d",rc);
	whefs_fs_finalize( fs );
	return whefs_rc.BadMagicError;
    }
    fs->filesize = fsize;
    fs->offsets[WHEFS_OFF_CLIENT_MAGIC] = fs->dev->api->tell(fs->dev);
    size_t aSize = whio_dev_size( fs->dev );
    if( !fsize || !aSize || (aSize != fsize) )
    { /* reminder: (aSize > fsize) must be allowed for static memory buffers to be usable as i/o devices. */
	WHEFS_DBG_ERR("File sizes don't agree: expected %u but got %u", fsize, aSize );
	whefs_fs_finalize( fs );
	return whefs_rc.ConsistencyError;
    }
    fs->offsets[WHEFS_OFF_EOF] = fsize;

#define CHECK if( whefs_rc.OK != rc ) { whefs_fs_finalize(fs); WHEFS_DBG_WARN("Decode of vfs options failed!"); return rc; }
    whefs_fs_options * opt = &fs->options;
    *opt = whefs_fs_options_nil;
    /* Read FS options... */
    // FIXME: factor this out into whefs_options_read():
    fs->dev->api->seek( fs->dev, fs->offsets[WHEFS_OFF_CLIENT_MAGIC], SEEK_SET );
    rc = whio_dev_decode_uint16( fs->dev, &opt->magic.length );
    CHECK;
    ///fs->offsets[WHEFS_OFF_OPTIONS] =
    fs->dev->api->seek( fs->dev, opt->magic.length, SEEK_CUR );
    /* FIXME: store the opt->magic.data somewhere! Ownership requires some changes in other code. */
    // FIXME: add whio_dev_size_t_en/decode()
    rc = whio_dev_decode_uint32( fs->dev, &opt->block_size );
    CHECK;
    rc = whefs_dev_id_decode( fs->dev, &opt->block_count );
    CHECK;
    rc = whefs_dev_id_decode( fs->dev, &opt->inode_count );
    CHECK;
    rc = whio_dev_decode_uint16( fs->dev, &opt->filename_length );
    CHECK;
#undef CHECK
    whefs_fs_init_sizes( fs );

    rc = whefs_fs_init_bitsets( fs );
    if( whefs_rc.OK != rc )
    {
	/* we could treat this as non-fatal we we changed how
	   cache-is-on checking is done elsewhere. */
	WHEFS_DBG_ERR("Init of bitsets failed with rc %d!", rc);
	whefs_fs_finalize( fs );
	return rc;
    }
    rc = whefs_fs_hints_read( fs );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Reading of hints failed rc %d!", rc);
	whefs_fs_finalize( fs );
	return rc;
    }
#if WHEFS_LOAD_CACHES_ON_OPEN
    //WHEFS_DBG_CACHE("Pre-loading inode cache.");
    rc = whefs_fs_caches_load( fs );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Error #%d initializing inode/block caches!", rc);
	whefs_fs_finalize( fs );
	return rc;
    }
#endif
    return whefs_rc.OK;
}

int whefs_openfs_dev( whio_dev * restrict dev, whefs_fs ** tgt, bool takeDev )
{
    if( ! dev || !tgt ) return whefs_rc.ArgError;
    whefs_fs * fs = whefs_fs_alloc();
    if( ! fs ) return whefs_rc.AllocError;
    *fs = whefs_fs_empty;
    // FIXME: do a 1-byte write test to see if the device is writeable,
    // or add a parameter to the function defining the write mode.
    fs->flags |= WHEFS_FLAG_ReadWrite; /* we're guessing!!! */
    fs->dev = dev;
    fs->ownsDev = takeDev;
    whefs_fs_check_fileno( fs );
    int rc = whefs_openfs_stage2( fs );
    if( whefs_rc.OK == rc )
    {
        whefs_fs_mmap_connect( fs );
	*tgt = fs;
    }
    return rc;
}

int whefs_openfs( char const * filename, whefs_fs ** tgt, bool writeMode )
{
    // FIXME: refactor this to take a whio_dev and split it into
    // two or three parts.
    if( ! filename || !tgt ) return whefs_rc.ArgError;
    whefs_fs * fs = whefs_fs_alloc();
    if( ! fs ) return whefs_rc.AllocError;
    *fs = whefs_fs_empty;
    fs->flags |= (writeMode ? WHEFS_FLAG_ReadWrite : WHEFS_FLAG_Read);
    if( ! whefs_open_FILE( filename, fs, writeMode, false ) )
    {
	WHEFS_DBG_WARN("Could not open file [%s] in %s mode.",
		       filename, writeMode ? "read/write" : "read-only" );
	whefs_fs_free(fs);
	return whefs_rc.IOError;
    }
    int rc = whefs_openfs_stage2( fs );
    if( whefs_rc.OK == rc )
    {
        whefs_fs_mmap_connect( fs );
	*tgt = fs;
    }
    return rc;
}

void whefs_fs_dump_info( whefs_fs const * restrict fs, FILE * out )
{
    struct SizeOfs
    {
	char const * label;
	size_t size;
    } sizeofs[] = {
#define X(T) {"sizeof(" # T ")",sizeof(T)}
    X(whio_dev),
    X(whio_dev_api),
    X(FILE),
    X(whefs_fs),
    X(whefs_file),
#if 1
    X(whio_stream),
    X(whio_stream_api),
#endif
    X(whbits),
    X(whefs_string),
    X(whefs_inode),
    X(whefs_block),
    X(whefs_hashid),
    X(whefs_hashid_list),
    X(whefs_fs_closer_list),
    {0,0}
    };
#undef X

    fprintf( out,"%s:%d:%s():\n", __FILE__, __LINE__, __func__);

    struct SizeOfs * so = sizeofs;
    fprintf( out, "sizeof() of various internal types:\n");
    for( ;so && so->label; ++so )
    {
	fprintf( out,"\t%s = %"PRIu64"\n", so->label, (uint64_t)so->size );
    }



    fprintf( out,"Various EFS stats:\n" );
    whefs_fs_options const * o = &fs->options;
    fprintf( out,
	     "\ton-disk sizeof whefs_inode = %u (+%"PRIu64" bytes for the name)\n"
	     "\tbits used for node/block IDs: %d\n"
	     "\tblock size: %"WHIO_SIZE_T_PFMT"\n"
	     "\tblock count: %"WHEFS_ID_TYPE_PFMT"\n"
	     "\tmax inode count: %"WHEFS_ID_TYPE_PFMT" (1 is reserved for the root dir entry!)\n"
	     "\tmax filename length: %u (WHEFS_MAX_FILENAME_LENGTH=%u)\n"
	     "\tmagic cookie length: %"PRIu32"\n"
	     "\tContainer size:\n\t\tcalculated =\t\t%"WHIO_SIZE_T_PFMT"\n\t\tdevice-reported =\t%"WHIO_SIZE_T_PFMT"\n",
	     whefs_sizeof_encoded_inode, (uint64_t)whefs_fs_sizeof_name(&fs->options),
	     (int) WHEFS_ID_TYPE_BITS,
	     o->block_size,
	     o->block_count,
	     o->inode_count,
	     o->filename_length, WHEFS_MAX_FILENAME_LENGTH,
	     (uint32_t)o->magic.length,
	     whefs_fs_calculate_size(&fs->options),
	     whio_dev_size(fs->dev)
	     );
#if 1
    fprintf( out, "\tEFS internal table offsets:\n");
#define OFF(X) fprintf(out,"\t\t%s\t= %u\n",# X, fs->offsets[WHEFS_OFF_ ## X])
    OFF(CORE_MAGIC);
    OFF(SIZE);
    OFF(CLIENT_MAGIC);
    OFF(OPTIONS);
    OFF(HINTS);
    OFF(INODE_NAMES);
    OFF(INODES_NO_STR);
    OFF(BLOCKS);
    OFF(EOF);
#undef OFF
#endif

}

int whefs_fs_append_blocks( whefs_fs * restrict fs, whefs_id_type count )
{
    if( !count || !fs || !fs->dev ) return whefs_rc.ArgError;
    if( !whefs_fs_is_rw(fs) )
    {
        return whefs_rc.AccessError;
    }
    /**
       Having an active mmap() makes it impossible to truncate an open
       file because our simplified mmap() proxy device doesn't have a
       proper truncate() implementation. So we remove the proxy (if any)
       before truncating, then reconnect it (if the device supports it)
       afterwards.
    */
    whefs_fs_mmap_disconnect(fs);
    whefs_fs_options * opt = &fs->options;
    const whefs_id_type oldCount = opt->block_count;
    const size_t oldEOF = fs->offsets[WHEFS_OFF_EOF];
    const size_t newEOF = oldEOF + (whefs_fs_sizeof_block(opt) * count);
    int rc = fs->dev->api->truncate( fs->dev, newEOF );
    //WHEFS_DBG("Adding %"WHEFS_ID_TYPE_PFMT" blocks to fs (current count=%"WHEFS_ID_TYPE_PFMT").",count,oldCount);
    if( whio_rc.OK != rc )
    {
        WHEFS_DBG_ERR("Could not truncate fs to %u bytes to add %"WHEFS_ID_TYPE_PFMT" blocks!",newEOF, count);
        fs->dev->api->truncate( fs->dev, oldEOF ); // just to be sure
        whefs_fs_mmap_connect(fs);
        return fs->err = rc;
    }
    fs->offsets[WHEFS_OFF_EOF] = newEOF;
    fs->filesize = newEOF;
    opt->block_count += count;
    // FIXME: error handling!
    // If anything goes wrong here, the EFS *will* be corrupted.
    whefs_fs_write_filesize( fs );
    whefs_mkfs_write_options( fs );
    whefs_fs_init_bitset_blocks( fs ); // will re-alloc the bitset cache.
    whefs_block bl = whefs_block_empty;
    whefs_id_type id;
    for( id = (oldCount+1); id <= opt->block_count; ++id )
    {
        //WHEFS_DBG("Adding block #%"WHEFS_ID_TYPE_PFMT,id);
        bl.id = id;
        rc = whefs_block_wipe( fs, &bl, true, true, false );
        if( whefs_rc.OK != rc ) break;
    }
    whefs_fs_flush( fs );
    whefs_fs_mmap_connect( fs ); // We need to re-mmap() to account for the new size!
    return rc;
}

int whefs_fs_setopt_autoclose_files( whefs_fs * fs, bool on )
{
    if( ! fs ) return whefs_rc.ArgError;
    if( ! on ) fs->flags |= WHEFS_FLAG_FS_NoAutoCloseFiles;
    else fs->flags &= ~WHEFS_FLAG_FS_NoAutoCloseFiles;
    return whefs_rc.OK;
}

int whefs_fs_setopt_hash_cache( whefs_fs * fs, bool on, bool loadNow )
{
    if( ! fs ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    if( on )
    {
        // Reminder: we don't check if it was already on so we
        // can more easily honor loadNow.
        fs->flags |= WHEFS_FLAG_FS_EnableHashCache;
        if( loadNow )
        {
            rc = whefs_inode_hash_cache_load( fs );
        }
    }
    else
    {
        fs->flags &= ~WHEFS_FLAG_FS_EnableHashCache;
        whefs_hashid_list_free( fs->cache.hashes );
        fs->cache.hashes = 0;
    }
    if( whefs_rc.OK != rc )
    {
        whefs_hashid_list_free( fs->cache.hashes );
        if( on ) fs->flags &= ~WHEFS_FLAG_FS_EnableHashCache;
    }
    return rc;
}


#ifdef __cplusplus
} /* extern "C"*/
#endif
