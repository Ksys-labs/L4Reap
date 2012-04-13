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

#include <stdlib.h>
#include <stdio.h>

#include <wh/whio/whio_devs.h>

/**
   Internal implementation details for the whio_dev subdev wrapper.
*/
typedef struct whio_dev_subdev_meta
{
    /**
       Underlying FILE handle. Owned by this
       object.
    */
    whio_dev * dev;
    /**
       Lower bound of device, relative to parent BOF.
    */
    whio_size_t lower;
    /**
       Upper bound of device, relative to parent BOF. Use 0
       for "no bound".
    */
    whio_size_t upper;
    /**
       Current cursor pos, relative to parent BOF.
    */
    whio_size_t pos;
} whio_dev_subdev_meta;

/**
   Initialization object for whio_dev_subdev objects. Also used as
   whio_dev::typeID for such objects.
*/
#define WHIO_DEV_SUBDEV_META_INIT { \
    0, /* dev */ \
    0, /* lower */ \
    0, /* upper */ \
    0 /* pos */ \
    }
static const whio_dev_subdev_meta whio_dev_subdev_meta_empty = WHIO_DEV_SUBDEV_META_INIT;

#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_subdev_meta_alloc_slots object.
*/
whio_dev_subdev_meta_alloc_count = 4
};
static struct
{
    whio_dev_subdev_meta objs[whio_dev_subdev_meta_alloc_count];
    char used[whio_dev_subdev_meta_alloc_count];
} whio_dev_subdev_meta_alloc_slots = { {WHIO_DEV_SUBDEV_META_INIT}, {0} };
#endif

whio_dev_subdev_meta * whio_dev_subdev_meta_alloc()
{
    whio_dev_subdev_meta * obj = 0;
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = 0;
    for( ; i < whio_dev_subdev_meta_alloc_count; ++i )
    {
	if( whio_dev_subdev_meta_alloc_slots.used[i] ) continue;
	whio_dev_subdev_meta_alloc_slots.used[i] = 1;
	whio_dev_subdev_meta_alloc_slots.objs[i] = whio_dev_subdev_meta_empty;
	obj = &whio_dev_subdev_meta_alloc_slots.objs[i];
	//WHIO_DEBUG("Allocated device #%u @0x%p\n", i, (void const *)obj );
	break;
    }
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_subdev_meta *) malloc( sizeof(whio_dev_subdev_meta) );
    return obj;
}

void whio_dev_subdev_meta_free( whio_dev_subdev_meta * obj )
{
#if WHIO_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_subdev_meta_alloc_slots.objs[0]) ||
	(obj > &whio_dev_subdev_meta_alloc_slots.objs[whio_dev_subdev_meta_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_subdev_meta_alloc_slots.objs[0]);
	if( 0 )
	{
	    WHIO_DEBUG("Address range = 0x%p to 0x%p, ndx=%u\n",
		       (void const *)&whio_dev_subdev_meta_alloc_slots.objs[0],
		       (void const *)&whio_dev_subdev_meta_alloc_slots.objs[whio_dev_subdev_meta_alloc_count-1],
		       ndx
		       );
	    WHIO_DEBUG("Freeing object @0x%p from static pool index %u (@0x%p)\n",
		       (void const *)obj,
		       ndx,
		       (void const *)&whio_dev_subdev_meta_alloc_slots.objs[ndx] );
	}

	whio_dev_subdev_meta_alloc_slots.objs[ndx] = whio_dev_subdev_meta_empty;
	whio_dev_subdev_meta_alloc_slots.used[ndx] = 0;
	return;
    }
#else
    free(obj);
#endif /* WHIO_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_subdev API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a whio_dev_subdev.
 */
#define WHIO_subdev_DECL(RV) whio_dev_subdev_meta * sub = (dev ? (whio_dev_subdev_meta*)dev->impl.data : 0); \
    if( !sub || ((void const *)&whio_dev_subdev_meta_empty != dev->impl.typeID) || (!sub->dev)) return RV


static whio_size_t whio_dev_subdev_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_subdev_DECL(whio_rc.SizeTError);
    if( (sub->pos < sub->lower) || (sub->pos >= sub->upper ) ) return 0;
    const whio_size_t opos = sub->dev->api->tell( sub->dev );
    if( whio_rc.SizeTError == opos ) return 0;
    if( sub->pos != sub->dev->api->seek( sub->dev, sub->pos, SEEK_SET ) ) return 0;
    whio_size_t rend = sub->pos + n;
    if( rend > sub->upper )
    {
	rend = sub->upper;
    }
    whio_size_t rlen = rend - sub->pos;
    whio_size_t rc = 0;
    if( rlen )
    {
	rc = sub->dev->api->read( sub->dev, dest, rlen );
    }
    sub->pos += rc;
    sub->dev->api->seek( sub->dev, opos, SEEK_SET );
    return rc;
}

static whio_size_t whio_dev_subdev_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_subdev_DECL(0);
    if( (sub->pos < sub->lower) || (sub->pos >= sub->upper ) ) return 0;
    const whio_size_t opos = sub->dev->api->tell( sub->dev );
    if( whio_rc.SizeTError == opos ) return 0;
    if( sub->pos != sub->dev->api->seek( sub->dev, sub->pos, SEEK_SET ) ) return 0;
    whio_size_t rend = sub->pos + n;
    if( rend > sub->upper )
    {
	rend = sub->upper;
    }
    whio_size_t wlen = rend - sub->pos;
    whio_size_t rc = 0;
    if( wlen )
    {
	rc = sub->dev->api->write( sub->dev, src, wlen );
    }
    sub->pos += rc;
    sub->dev->api->seek( sub->dev, opos, SEEK_SET );
    return rc;
}

static int whio_dev_subdev_error( whio_dev * dev )
{
    WHIO_subdev_DECL(whio_rc.ArgError);
    return sub->dev->api->error( sub->dev );
}

static int whio_dev_subdev_clear_error( whio_dev * dev )
{
    WHIO_subdev_DECL(whio_rc.ArgError);
    return sub->dev->api->clear_error( sub->dev );
}

static int whio_dev_subdev_eof( whio_dev * dev )
{
    WHIO_subdev_DECL(whio_rc.ArgError);
    return sub->pos >= sub->upper;
}

static whio_size_t whio_dev_subdev_tell( whio_dev * dev )
{
    WHIO_subdev_DECL(whio_rc.SizeTError);
    if( sub->pos < sub->lower ) return whio_rc.SizeTError;
    else return sub->pos - sub->lower;
}

static whio_size_t whio_dev_subdev_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_subdev_DECL(whio_rc.SizeTError);
    whio_size_t too = sub->pos;
    whio_size_t ppos = sub->dev->api->tell( sub->dev );
    whio_size_t top = sub->upper;
#define OVERFLOW return whio_rc.SizeTError
#define UNDERFLOW return whio_rc.SizeTError
    switch( whence )
    {
      case SEEK_SET:
	  if( pos < 0 ) return whio_rc.SizeTError;
	  too = sub->lower + (whio_size_t)pos;
	  if( too < sub->lower ) OVERFLOW;
	  break;
      case SEEK_END:
	  /* This reeks of special-case handling... */
	  if( ! sub->upper )
	  {
	      top = sub->dev->api->seek( sub->dev, 0, SEEK_END );
	      sub->dev->api->seek( sub->dev, ppos, SEEK_SET );
	      if( whio_rc.SizeTError == top ) return top;
	  }
	  else
	  {
	      top = sub->upper;
	  }
	  too = top + pos;
	  if( (pos < 0) && (too > sub->upper ) ) UNDERFLOW;
	  else if( (pos>0) && (too < sub->upper) ) OVERFLOW;
	  break;
      case SEEK_CUR:
	  too += pos;
	  if( (pos < 0) && (too > sub->pos ) ) UNDERFLOW;
	  else if( (pos > 0) && (too < sub->pos) ) OVERFLOW;
	  break;
      default:
	  return whio_rc.SizeTError;
	  break;
    };
#undef OVERFLOW
#undef UNDERFLOW
    const whio_size_t upos = sub->dev->api->seek( sub->dev, (sub->pos = too), SEEK_SET );
    return (upos == sub->pos)
	? (sub->pos - sub->lower)
	: whio_rc.SizeTError;
}

static int whio_dev_subdev_flush( whio_dev * dev )
{
    WHIO_subdev_DECL(whio_rc.ArgError);
    return sub->dev ? sub->dev->api->flush( sub->dev ) : whio_rc.ArgError;
}

short whio_dev_subdev_iomode( whio_dev * dev )
{
    WHIO_subdev_DECL(-1);
    return sub->dev->api->iomode( sub->dev );
}

static int whio_dev_subdev_trunc( whio_dev * dev, whio_off_t len )
{
    return whio_rc.UnsupportedError;
    //WHIO_subdev_DECL(whio_rc.ArgError);
    //return sub->dev->api->truncate( sub->dev, len );
}

static int whio_dev_subdev_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    WHIO_subdev_DECL(whio_rc.ArgError);
    int rc = whio_rc.UnsupportedError;
    whio_size_t * sz = 0;
    switch( arg )
    {
      case whio_dev_ioctl_SUBDEV_parent_dev:
	  rc = whio_rc.OK;
	  *(va_arg(vargs,whio_dev**)) = sub->dev;
	  break;
      case whio_dev_ioctl_SUBDEV_bounds_get:
	  rc = whio_rc.OK;
	  sz = (va_arg(vargs,whio_size_t*));
	  if( sz ) *sz = sub->lower;
	  sz = (va_arg(vargs,whio_size_t*));
	  if( sz ) *sz = sub->upper;
	  break;
      default:
	  return sub->dev->api->ioctl( sub->dev, arg, vargs );
	  break;
    };
    return rc;
}

static bool whio_dev_subdev_close( whio_dev * dev )
{
    WHIO_subdev_DECL(false);
    dev->api->flush(dev);
    if( dev->client.dtor ) dev->client.dtor( dev->client.data );
    dev->client = whio_client_data_empty;
    *sub = whio_dev_subdev_meta_empty;
    whio_dev_subdev_meta_free( sub );
    dev->impl = whio_impl_data_empty;
    return true;
}

static void whio_dev_subdev_finalize( whio_dev * dev )
{
    if( dev )
    {
	dev->api->close(dev);
	whio_dev_free(dev);
    }
}

#undef WHIO_subdev_DECL

static const whio_dev_api whio_dev_api_subdev =
    {
    whio_dev_subdev_read,
    whio_dev_subdev_write,
    whio_dev_subdev_close,
    whio_dev_subdev_finalize,
    whio_dev_subdev_error,
    whio_dev_subdev_clear_error,
    whio_dev_subdev_eof,
    whio_dev_subdev_tell,
    whio_dev_subdev_seek,
    whio_dev_subdev_flush,
    whio_dev_subdev_trunc,
    whio_dev_subdev_ioctl,
    whio_dev_subdev_iomode
    };

static const whio_dev whio_dev_subdev_empty =
    {
    &whio_dev_api_subdev,
    { /* impl */
    0, /* implData. Must be-a (whio_dev_subdev_meta*) */
    (void const *)&whio_dev_subdev_meta_empty /* typeID */
    }
    };

whio_dev * whio_dev_subdev_create( whio_dev * parent, whio_size_t lowerBound, whio_size_t upperBound )
{
    if( ! parent || (upperBound && (upperBound <= lowerBound)) ) return 0;
    whio_dev * dev = whio_dev_alloc();
    if( ! dev ) return 0;
    whio_dev_subdev_meta * meta = whio_dev_subdev_meta_alloc();
    if( ! meta )
    {
	whio_dev_free(dev);
	return 0;
    }
    *dev = whio_dev_subdev_empty;
    *meta = whio_dev_subdev_meta_empty;
    dev->impl.data = meta;
    meta->dev = parent;
    meta->lower = lowerBound;
    meta->pos = lowerBound;
    meta->upper = upperBound;
    return dev;
}

int whio_dev_subdev_rebound( whio_dev * dev, whio_size_t lowerBound, whio_size_t upperBound )
{
    if( !dev || (upperBound && (upperBound <= lowerBound)) ) return whio_rc.ArgError;
    if( (void const *)&whio_dev_subdev_meta_empty != dev->impl.typeID ) return whio_rc.TypeError;
    whio_dev_subdev_meta * sub = (whio_dev_subdev_meta*)dev->impl.data;
    if( ! sub || !sub->dev ) return whio_rc.InternalError;
    sub->lower = sub->pos = lowerBound;
    sub->upper = upperBound;
    return whio_rc.OK;
}
