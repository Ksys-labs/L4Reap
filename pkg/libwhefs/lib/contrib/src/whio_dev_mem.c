/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
/************************************************************************
Implementations for whio_dev types:

- one for a dynamically allocated in-memory buffer, with the option to
grow the buffer on demand.

- one for existing memory ranges, with read-only or read-write access.

They comply with the whio_dev interface, and all
implementation-specified behaviours of the interface are documented
along with the factory functions for creating the device objects.

************************************************************************/

#include <wh/whio/whio_devs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* memcpy() */
#include <stdint.h>

/**
   Internal implementation details for the whio_dev memory
   buffer wrapper.
*/
typedef struct whio_dev_membuf_meta
{
    /**
       Size of the buffer (i.e. position of EOF).
    */
    whio_size_t size;
    /**
       Bytes allocated in the buffer. May be larger than size.
    */
    whio_size_t alloced;
    /**
       The buffer itself.
    */
    unsigned char * buffer; /* is unsigned necessary here? */
    /**
       Current position within the buffer.
    */
    whio_size_t pos;
    /**
       If true, the buffer will be grown as needed.
    */
    bool expandable;
    /**
       By how much do we want to expand when we grow?

       values <= 1.0 mean do not expand.

       values >1.0 mean:

       - when expanding, expand by (amount * factor).

       - when shrinking, only release memory if (amount / factor) is
       greater than the amount requested (that is, if we shrunk
       enough).
    */
    float expfactor;
} whio_dev_membuf_meta;


#define WHIO_DEV_MEMBUF_META_INIT { \
    0, /* size */ \
    0, /* alloced */ \
    0, /* buffer */ \
    0, /* pos */ \
    false, /* expandable */ \
    1.5 /* expfactor */ \
    }
/**
   Initialization object for new whio_dev_membuf objects.
*/
static const whio_dev_membuf_meta whio_dev_membuf_meta_empty = WHIO_DEV_MEMBUF_META_INIT;

#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_membuf_meta_alloc_slots object.
*/
whio_dev_membuf_meta_alloc_count = 5
};
static struct
{
    whio_dev_membuf_meta objs[whio_dev_membuf_meta_alloc_count];
    char used[whio_dev_membuf_meta_alloc_count];
} whio_dev_membuf_meta_alloc_slots = { {WHIO_DEV_MEMBUF_META_INIT}, {0} };
#endif

static whio_dev_membuf_meta * whio_dev_membuf_meta_alloc()
{
    whio_dev_membuf_meta * obj = 0;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whio_dev_membuf_meta_alloc_count; ++i )
    {
	if( whio_dev_membuf_meta_alloc_slots.used[i] ) continue;
	whio_dev_membuf_meta_alloc_slots.used[i] = 1;
	whio_dev_membuf_meta_alloc_slots.objs[i] = whio_dev_membuf_meta_empty;
	obj = &whio_dev_membuf_meta_alloc_slots.objs[i];
	break;
    }
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_membuf_meta *) malloc( sizeof(whio_dev_membuf_meta) );
    return obj;
}

static void whio_dev_membuf_meta_free( whio_dev_membuf_meta * obj )
{
    if( ! obj ) return;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_membuf_meta_alloc_slots.objs[0]) ||
	(obj > &whio_dev_membuf_meta_alloc_slots.objs[whio_dev_membuf_meta_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_membuf_meta_alloc_slots.objs[0]);
	whio_dev_membuf_meta_alloc_slots.objs[ndx] = whio_dev_membuf_meta_empty;
	whio_dev_membuf_meta_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_membuf API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a whio_dev_membuf.
 */
#define WHIO_MEMBUF_DECL(RV) whio_dev_membuf_meta * mb = (dev ? (whio_dev_membuf_meta*)dev->impl.data : 0); \
    if( !mb  || ((void const *)&whio_dev_api_membuf != dev->impl.typeID) ) return RV

static whio_size_t whio_dev_membuf_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_MEMBUF_DECL(0); //whio_rc.SizeTError);
    if( ! dest ) return 0; //whio_rc.SizeTError;
    if( mb->pos >= mb->size ) return 0;
    whio_size_t rlen = n;
    if( ((mb->pos + n) >= mb->size )
	|| ((mb->pos + n) < mb->pos)
	)
    {
	rlen = mb->size - mb->pos;
    }
    if( rlen )
    {
	memcpy( dest, mb->buffer + mb->pos, rlen );
	mb->pos += rlen;
    }
    return rlen;
}

static whio_size_t whio_dev_membuf_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_MEMBUF_DECL(0);
    if( ! n || !src ) return 0;
    whio_size_t newEnd = mb->pos + n;
    if( newEnd < mb->pos ) return 0; /* overflow! fixme: write as much as we can */
    whio_size_t wlen = n;

    if( newEnd >= mb->size )
    {
	//WHIO_DEBUG("write of %u bytes to pos %u would go out of bounds (%u). Expanding==%d\n",n, mb->pos, mb->size,mb->expandable);
	if( mb->expandable )
	{
	    dev->api->truncate( dev, newEnd );
	    /* ignore possible trunc failure and do a short write. */
	}
	else
	{
	    mb->size = (newEnd > mb->alloced)
		? mb->alloced
		: newEnd;
	}
    }

    if( mb->pos >= mb->size )
    {
	/*
	  We likely got seek()ed out of bounds. Behave like FILE does
	  on my Linux box and resize now...
	*/
	//WHIO_DEBUG("pos(%u) > size(%u). Expanding to %u = %s\n", mb->pos, mb->size, newEnd, mb->expandable ? "yes" : "no");
	if( ! mb->expandable ) return 0;
	//WHIO_DEBUG("Seems we've been truncated from %u to %u\n",mb->size, mb->pos);
	mb->size = newEnd;
    }

    if( (newEnd >= mb->size ) /* will overflow current EOF. */
	//|| (newEnd < mb->pos ) /* overflow in mb->pos+n. */
	)
    {
	/* write as much as we can, to EOF. */
	wlen = mb->size - mb->pos;
    }
    if( wlen )
    {
	memcpy( mb->buffer + mb->pos, src, wlen );
	mb->pos += wlen;
	if( mb->size < mb->pos ) mb->size = mb->pos;
    }
    return wlen;
}

static int whio_dev_membuf_error( whio_dev * dev )
{
    WHIO_MEMBUF_DECL(whio_rc.ArgError);
    return (mb->pos <= mb->size)
	? whio_rc.OK
	: whio_rc.RangeError;
}

static int whio_dev_membuf_clear_error( whio_dev * dev )
{
    return whio_rc.OK;
}

static int whio_dev_membuf_eof( whio_dev * dev )
{
    WHIO_MEMBUF_DECL(whio_rc.ArgError);
    return (mb->pos < mb->size)
	? 0
	: 1;
}

static whio_size_t whio_dev_membuf_tell( whio_dev * dev )
{
    WHIO_MEMBUF_DECL(whio_rc.SizeTError);
    return mb->pos;
}

static whio_size_t whio_dev_membuf_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_MEMBUF_DECL(whio_rc.SizeTError);
    whio_size_t too = mb->pos;
    switch( whence )
    {
      case SEEK_SET:
	  if( pos < 0 ) return whio_rc.SizeTError;
	  too = (whio_size_t)pos;
	  break;
      case SEEK_END:
	  too = mb->size + pos;
	  if( (pos>0) && (too < mb->size) )  /* overflow! */ return whio_rc.SizeTError;
	  else if( (pos<0) && (too > mb->size) )  /* underflow! */ return whio_rc.SizeTError;
	  break;
      case SEEK_CUR:
	  too += pos;
	  if( (pos>0) && (too < mb->pos) )  /* overflow! */ return whio_rc.SizeTError;
	  else if( (pos<0) && (too > mb->pos) )  /* underflow! */ return whio_rc.SizeTError;
	  break;
      default:
	  return whio_rc.SizeTError;
	  break;
    };
    /** We defer any actual expansion until the next write. */
    return (mb->pos = too);
}

static int whio_dev_membuf_flush( whio_dev * dev )
{
    return whio_rc.OK;
}

static int whio_dev_membuf_trunc( whio_dev * dev, whio_off_t _len )
{
    WHIO_MEMBUF_DECL(whio_rc.ArgError);
    if( _len < 0 ) return whio_rc.RangeError;
    whio_size_t ulen = (whio_size_t)_len;
    if( 0 == ulen )
    {
#if 1 /* arguable. Hmmm. */
	if( mb->expandable )
	{
	    /**
	       We only do this for expanding buffers because otherwise
	       we could no longer write to the buffer (as we can't expand
	       it).
	    */
	    free( mb->buffer );
	    mb->buffer = 0;
	    mb->alloced = 0;
	}
#endif
	mb->size = 0;
	return 0;
    }
    if( !mb->alloced || (ulen > mb->alloced) )
    { /* try to grow */
	whio_size_t alen = ulen;
	if( mb->expandable )
	{ /* see how much to expand by. */
	    alen = (whio_size_t)(mb->alloced * (mb->expfactor+0.01));
	    /* ^^^ that +0.01 kludge is to work around (100*1.8)==179 and (100*1.9)==189 */
	    if( alen < ulen ) alen = ulen;
	}
	void * b = realloc( mb->buffer, alen );
	if( ! b ) return whio_rc.AllocError;
	mb->buffer = b;
	//WHIO_DEBUG("Grew buffer from %u to %u bytes\n", mb->alloced, alen);
	if( mb->alloced < alen )
	{   /* clean up new memory to avoid RAM artifacts. */
	    memset( WHIO_VOID_PTR_ADD(b,mb->alloced), 0, alen - mb->alloced );
	}
	mb->alloced = alen;
	mb->size = ulen;
	return whio_rc.OK;
    }
    if( mb->expandable && (mb->alloced > ulen) )
    {	/**
	   Try to shrink...

	   We only do this for expanding buffers because otherwise
	   we could no longer write to the buffer (as we can't expand
	   it).
	*/
	//const whio_size_t oldAlloc = mb->alloced;
	//WHIO_DEBUG("oldAlloc=%u mb->alloced=%u ulen=%u\n",oldAlloc,mb->alloced,ulen);
	whio_size_t alen = ulen;
	if( alen < (mb->alloced/mb->expfactor) )
	{
	    void * b = realloc( mb->buffer, alen );
	    if( b )
	    {
		mb->buffer = b;
		mb->alloced = alen;
		//WHIO_DEBUG("Shrunk buffer from %u to %u bytes\n", oldAlloc, mb->alloced);
	    }
	    /* ignore realloc failure if we're shrinking - just keep the old block. */
	}
    }
    mb->size = ulen;
    return whio_rc.OK;
}

short whio_dev_membuf_iomode( whio_dev * dev )
{
    WHIO_MEMBUF_DECL(-1);
    return 1;
}

static int whio_dev_membuf_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    int rc = whio_rc.UnsupportedError;
    WHIO_MEMBUF_DECL(rc);
    whio_size_t * x = 0;
    unsigned char const ** cp = 0;
    switch( arg )
    {
      case whio_dev_ioctl_BUFFER_uchar_ptr:
          cp = va_arg(vargs,unsigned char const **);
          if( cp )
          {
              rc = whio_rc.OK;
              *cp = mb->buffer;
          }
          else
          {
              rc = whio_rc.ArgError;
          }
	  break;
      case whio_dev_ioctl_GENERAL_size:
          x = va_arg(vargs,whio_size_t*);
          if( x )
          {
              rc = whio_rc.OK;
              *x = mb->size;
          }
          else
          {
              rc = whio_rc.ArgError;
          }
	  break;
      case whio_dev_ioctl_BUFFER_size:
          x = va_arg(vargs,whio_size_t*);
          if( x )
          {
              rc = whio_rc.OK;
              *x = mb->alloced;
          }
          else
          {
              rc = whio_rc.ArgError;
          }
	  break;
      default:
          break;
    };
    return rc;
}

static bool whio_dev_membuf_close( whio_dev * dev );
static void whio_dev_membuf_finalize( whio_dev * dev );
    
const whio_dev_api whio_dev_api_membuf =
    {
    whio_dev_membuf_read,
    whio_dev_membuf_write,
    whio_dev_membuf_close,
    whio_dev_membuf_finalize,
    whio_dev_membuf_error,
    whio_dev_membuf_clear_error,
    whio_dev_membuf_eof,
    whio_dev_membuf_tell,
    whio_dev_membuf_seek,
    whio_dev_membuf_flush,
    whio_dev_membuf_trunc,
    whio_dev_membuf_ioctl,
    whio_dev_membuf_iomode
    };

#define WHIO_DEV_MEMBUF_INIT { \
    &whio_dev_api_membuf, \
    { /* impl */ \
    0, /* data. Must be-a (whio_dev_membuf*) */ \
    (void const *)&whio_dev_api_membuf /* typeID */ \
    } }

static const whio_dev whio_dev_membuf_empty = WHIO_DEV_MEMBUF_INIT;

static bool whio_dev_membuf_close( whio_dev * dev )
{
    if( dev )
    {
	if( dev->client.dtor ) dev->client.dtor( dev->client.data );
	dev->client = whio_client_data_empty;
	whio_dev_membuf_meta * f = (whio_dev_membuf_meta*)dev->impl.data;
	if( f )
	{
	    dev->impl.data = 0;
	    free(f->buffer);
	    //*f = whio_dev_membuf_meta_empty;
	    //free(f);
	    whio_dev_membuf_meta_free( f );
	    return true;
	}
        *dev = whio_dev_membuf_empty;
    }
    return false;
}

static void whio_dev_membuf_finalize( whio_dev * dev )
{
    if( dev )
    {
	dev->api->close(dev);
	whio_dev_free(dev);
    }
}
#undef WHIO_MEMBUF_DECL

whio_dev * whio_dev_for_membuf( whio_size_t size, float expFactor )
{
    whio_dev * dev = whio_dev_alloc();
    if( ! dev ) return 0;
    *dev = whio_dev_membuf_empty;
    whio_dev_membuf_meta * mb = whio_dev_membuf_meta_alloc();
    if( !mb )
    {
	whio_dev_free(dev);
	return 0;
    }
    *mb = whio_dev_membuf_meta_empty;
    dev->impl.data = mb;
    //mb->alloc_policy = expandable ? whio_dev_membuf_alloc_policy_133 : 0;
    mb->expandable = (expFactor >= 1.0);
    mb->expfactor = expFactor;
    int rc = dev->api->truncate( dev, size );
    if( whio_rc.OK != rc )
    {
	dev->api->finalize( dev );
	dev = 0;
    }
    //WHIO_DEBUG( "membuf @%p, buffer @%p: size=%u\n", (void const *)dev, (void const *)mb->buffer, mb->size );
    return dev;
}


/**
   Internal implementation details for the whio_dev memmap wrapper. It
   wraps a client-supplied memory range in the whio_dev interface.
*/
typedef struct whio_dev_memmap
{
    /**
       Size of the buffer (i.e. position of EOF).
    */
    whio_size_t size;
    /**
       The maximum size of the buffer. This starts out the same as
       size, but truncate() can shrink the effective size of the
       buffer.  We keep the largest size so that we can re-truncate()
       after a truncate() shrinks the range (but we can't grow larger
       than this value).
    */
    whio_size_t maxsize;

    /**
       Current position within the buffer.
    */
    whio_size_t pos;

    /**
       The memory buffer itself, in read/write form.
       For read-only streams this is 0.
    */
    void * rw;

    /**
       For read/write streams, this is the same as rw but const.  For
       read-only streams it is the start of the wrapped memory range.
    */
    void const * ro;
} whio_dev_memmap;

/**
   Initialization object for new whio_dev_memmap objects.
*/
#define WHIO_DEV_MEMMAP_INIT { \
    0, /* size */ \
    0, /* maxsize */ \
    0, /* pos */ \
    0, /* rw */ \
    0 /* ro */ \
    }

static const whio_dev_memmap whio_dev_memmap_empty = WHIO_DEV_MEMMAP_INIT;


#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_memmap_alloc_slots object.
*/
whio_dev_memmap_alloc_count = 5
};
static struct
{
    whio_dev_memmap objs[whio_dev_memmap_alloc_count];
    char used[whio_dev_memmap_alloc_count];
} whio_dev_memmap_alloc_slots = { {WHIO_DEV_MEMMAP_INIT}, {0} };
#endif

static whio_dev_memmap * whio_dev_memmap_alloc()
{
    whio_dev_memmap * obj = 0;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whio_dev_memmap_alloc_count; ++i )
    {
	if( whio_dev_memmap_alloc_slots.used[i] ) continue;
	whio_dev_memmap_alloc_slots.used[i] = 1;
	whio_dev_memmap_alloc_slots.objs[i] = whio_dev_memmap_empty;
	obj = &whio_dev_memmap_alloc_slots.objs[i];
	break;
    }
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_memmap *) malloc( sizeof(whio_dev_memmap) );
    return obj;
}

static void whio_dev_memmap_free( whio_dev_memmap * obj )
{
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_memmap_alloc_slots.objs[0]) ||
	(obj > &whio_dev_memmap_alloc_slots.objs[whio_dev_memmap_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_memmap_alloc_slots.objs[0]);
	whio_dev_memmap_alloc_slots.objs[ndx] = whio_dev_memmap_empty;
	whio_dev_memmap_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_memmap API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a
   whio_dev_memmap. Declares the local variable (whio_dev_memmap * mb)
   - the internal device data. On error (null argument or type
   mismatch) it calls (return RV).
 */
#define WHIO_MEMMAP_DECL(RV) whio_dev_memmap * mb = (dev ? (whio_dev_memmap*)dev->impl.data : 0); \
    if( !mb  || ((void const *)&whio_dev_api_memmap != dev->impl.typeID) ) return RV

static whio_size_t whio_dev_memmap_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_MEMMAP_DECL(0); //whio_rc.SizeTError);
    if( ! dest || !mb->ro ) return 0; //whio_rc.SizeTError;
    if( mb->pos >= mb->size ) return 0;
    whio_size_t rlen = n;
    if( ((mb->pos + n) >= mb->size )
	|| ((mb->pos + n) < mb->pos)
	)
    {
	rlen = mb->size - mb->pos;
    }
    if( rlen )
    {
	memcpy( dest, WHIO_VOID_CPTR_ADD(mb->ro,mb->pos), rlen );
	mb->pos += rlen;
    }
    return rlen;
}

static whio_size_t whio_dev_memmap_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_MEMMAP_DECL(0);
    if( ! n || !src || !mb->rw ) return 0;
    if( mb->pos >= mb->size )
    {
	//WHIO_DEBUG("write would go out of bounds.\n");
	return 0;
    }
    const whio_size_t newEnd = mb->pos + n;
    whio_size_t wlen = n;
    if( (newEnd >= mb->size ) /* would overflow EOF. */
	|| (newEnd < mb->pos ) /* overflow in mb->pos+n. */
	)
    {
	/* write as much as we can, to EOF. */
	wlen = mb->size - mb->pos;
    }
    if( wlen )
    {
	memcpy( WHIO_VOID_PTR_ADD(mb->rw,mb->pos), src, wlen );
	mb->pos += wlen;
    }
    return wlen;
}

static int whio_dev_memmap_error( whio_dev * dev )
{
    WHIO_MEMMAP_DECL(whio_rc.ArgError);
    return (mb->ro && (mb->pos <= mb->size))
	? whio_rc.OK
	: whio_rc.RangeError;
}

static int whio_dev_memmap_clear_error( whio_dev * dev )
{
    return whio_rc.OK;
}


static int whio_dev_memmap_eof( whio_dev * dev )
{
    WHIO_MEMMAP_DECL(whio_rc.ArgError);
    return (mb->pos < mb->size)
	? 0
	: 1;
}

static whio_size_t whio_dev_memmap_tell( whio_dev * dev )
{
    WHIO_MEMMAP_DECL(whio_rc.SizeTError);
#if 0
    return (mb->pos <= mb->size )
	? mb->pos
	: whio_rc.SizeTError;
#else
    return mb->pos;
#endif
}

static whio_size_t whio_dev_memmap_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_MEMMAP_DECL(whio_rc.SizeTError);
    whio_size_t too = mb->pos;
    switch( whence )
    {
      case SEEK_SET:
	  if( pos < 0 ) return whio_rc.SizeTError;
	  too = (whio_size_t)pos;
	  break;
      case SEEK_END:
	  too = mb->size + pos;
	  if( (pos>0) && (too < mb->size) )  /* overflow! */ return whio_rc.SizeTError;
	  else if( (pos<0) && (too > mb->size) )  /* underflow! */ return whio_rc.SizeTError;
	  break;
      case SEEK_CUR:
	  too += pos;
	  if( (pos>0) && (too < mb->pos) )  /* overflow! */ return whio_rc.SizeTError;
	  else if( (pos<0) && (too > mb->pos) )  /* underflow! */ return whio_rc.SizeTError;
	  break;
      default:
	  return whio_rc.SizeTError;
	  break;
    };
    return (mb->pos = too);
}

static int whio_dev_memmap_flush( whio_dev * dev )
{
    return whio_rc.OK;
}

static int whio_dev_memmap_trunc( whio_dev * dev, whio_off_t _len )
{
    WHIO_MEMMAP_DECL(whio_rc.ArgError);
    if( _len < 0 ) return whio_rc.RangeError;
    whio_size_t ulen = (whio_size_t)_len;
    if( ulen > mb->maxsize )
    {
	return whio_rc.RangeError;
    }
    mb->size = ulen;
    return whio_rc.OK;
}

short whio_dev_memmap_iomode( whio_dev * dev )
{
    WHIO_MEMMAP_DECL(-1);
    return (NULL == mb->rw) ? 0 : 1;
}

static int whio_dev_memmap_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    int rc = whio_rc.UnsupportedError;
    WHIO_MEMMAP_DECL(rc);
    whio_size_t * x = 0;
    switch( arg )
    {
      case whio_dev_ioctl_GENERAL_size:
          x = va_arg(vargs,whio_size_t*);
          if( x )
          {
              rc = whio_rc.OK;
              *x = mb->size;
          }
          else
          {
              rc = whio_rc.ArgError;
          }
	  break;
      case whio_dev_ioctl_BUFFER_size:
          x = va_arg(vargs,whio_size_t*);
          if( x )
          {
              rc = whio_rc.OK;
              *x = mb->maxsize;
          }
          else
          {
              rc = whio_rc.ArgError;
          }
	  break;
      default:
          break;
    };
    return rc;
}

static bool whio_dev_memmap_close( whio_dev * dev )
{
    if( dev )
    {
	if( dev->client.dtor ) dev->client.dtor( dev->client.data );
	dev->client = whio_client_data_empty;
	whio_dev_memmap * f = (whio_dev_memmap*)dev->impl.data;
	if( f )
	{
	    dev->impl.data = 0;
	    *f = whio_dev_memmap_empty;
	    whio_dev_memmap_free(f);
	    return true;
	}
    }
    return false;
}

static void whio_dev_memmap_finalize( whio_dev * dev )
{
    if( dev )
    {
	dev->api->close( dev );
	whio_dev_free(dev);
    }
}
#undef WHIO_MEMMAP_DECL

const whio_dev_api whio_dev_api_memmap =
    {
    whio_dev_memmap_read,
    whio_dev_memmap_write,
    whio_dev_memmap_close,
    whio_dev_memmap_finalize,
    whio_dev_memmap_error,
    whio_dev_memmap_clear_error,
    whio_dev_memmap_eof,
    whio_dev_memmap_tell,
    whio_dev_memmap_seek,
    whio_dev_memmap_flush,
    whio_dev_memmap_trunc,
    whio_dev_memmap_ioctl,
    whio_dev_memmap_iomode
    };

static const whio_dev whio_dev_memmap_dev_empty =
    {
    &whio_dev_api_memmap,
    { /* impl */
    0, /* data. Must be-a (whio_dev_memmap*) */
    (void const *)&whio_dev_api_memmap /* typeID */
    }
    };

/**
   Creates a new whio_dev wrapper for an existing memory range. The arguments:

   - rw = the read/write memory the device will wrap. Ownership is not changed.
   May be 0, but only if ro is not 0.

   - ro = the read-only memory the device will wrap. Ownership is not changed.
   May be 0, but only if rw is not 0.

   - size = the size of the rw or ro buffer. It is the caller's
   responsibility to ensure that the buffer is at least that
   long. This object will not allow i/o operations outside of that
   bound.

   If both ro and rw are not 0 then they must have the same address. If rw is 0 then
   the device is read-only, and any write operations will fail.

   On success a new whio_dev is returned. On error (invalid arguments,
   alloc error), 0 is returned.

   See whio_dev_for_memmap_rw() and whio_dev_for_memmap_ro() for more details
   about the returned object.
*/
static whio_dev * whio_dev_for_memmap( void * rw, void const * ro, whio_size_t size )
{
    if( (!rw && !ro) || ! size ) return 0;
    if( rw && ro )
    {
	if( ro != rw ) return 0;
    }
    whio_dev * dev = whio_dev_alloc();
    if( ! dev ) return 0;
    *dev = whio_dev_memmap_dev_empty;
    whio_dev_memmap * mb = whio_dev_memmap_alloc();
    if( !mb )
    {
	whio_dev_free(dev);
	return 0;
    }
    *mb = whio_dev_memmap_empty;
    dev->impl.data = mb;
    mb->size = mb->maxsize = size;
    mb->rw = rw;
    mb->ro = ro ? ro : rw;
    //WHIO_DEBUG( "memmap @%p, buffer @%p: size=%u\n", (void const *)dev, (void const *)mb->ro, mb->size );
    return dev;
}

whio_dev * whio_dev_for_memmap_rw( void * mem, whio_size_t size )
{
    return whio_dev_for_memmap( mem, mem, size );
}

whio_dev * whio_dev_for_memmap_ro( const void * mem, whio_size_t size )
{
    return whio_dev_for_memmap( 0, mem, size );
}

