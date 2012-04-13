#error "You must edit this file as described in the comments, then remove this line."
/**
   This is a skeleton implementation for whio_dev implementations. To use it:

   - Pick a device type name for your device. For example, "mydev".
   - Globally replace IODEV_TYPE in this file with that chosen name.
   - Search through this code for comment lines and do what they say
*/

#include "whio_dev.h"
#include "whio_common.h"

/**
   Internal implementation details for the whio_dev wrapper.

   Put any device-specific details here.
*/
typedef struct whio_dev_IODEV_TYPE_meta
{
    // Put your custom members here.
} whio_dev_IODEV_TYPE_meta;

/** Used for "zeroing out" whio_dev_IODEV_TYPE_meta instances. */
static const whio_dev_FILE whio_dev_IODEV_TYPE_meta_init = {/*fill this out*/};


static whio_size_t whio_dev_IODEV_TYPE_read( whio_dev * dev, void * dest, whio_size_t n )
{
    // add your stuff here
    return 0; // number of bytes real
}

static whio_size_t whio_dev_IODEV_TYPE_write( whio_dev * dev, void const * src, whio_size_t n )
{
    // add your stuff here
    return 0; // number of bytes written
}

static int whio_dev_IODEV_TYPE_error( whio_dev * dev )
{
    // add your stuff here
    return whio_rc.NYIError; // whio_rc.OK on success
}
static int whio_dev_IODEV_TYPE_clear_error( whio_dev * dev )
{
    // add your stuff here
    return whio_rc.NYIError; // whio_rc.OK on success
}
static int whio_dev_IODEV_TYPE_eof( whio_dev * dev )
{
    // add your stuff here
    return whio_rc.NYIError; // 0 == not at EOF
}

static whio_size_t whio_dev_IODEV_TYPE_tell( whio_dev * dev )
{
    // add your stuff here
    return whio_rc.SizeTError; // return current position
}

static whio_size_t whio_dev_IODEV_TYPE_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    // add your stuff here
    return whio_rc.SizeTError; // return current position
}

static int whio_dev_IODEV_TYPE_flush( whio_dev * dev )
{
    // add your stuff here
    return whio_rc.NYIError; // return whio_rc.OK on success
}
static int whio_dev_IODEV_TYPE_trunc( whio_dev * dev, whio_off_t len )
{
    // add your stuff here
    return whio_rc.NYIError; // return whio_rc.OK on success
}

static int whio_dev_IODEV_TYPE_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    // add your stuff here
    return whio_rc.NYIError; // return whio_rc.OK on success
}

static bool whio_dev_IODEV_TYPE_close( whio_dev * dev )
{
    if( ! dev || ! dev->impl.data ) return false;

    // Required by whio_dev_api::close() interface:
    if( dev->client.dtor ) dev->client.dtor( dev->client.data );
    dev->client = whio_dev_client_empty;

    // add your stuff here

    // Cleanup of internal data may optionally be done here or in
    // finalize(), depending on the requirements of the device
    // (e.g. is it re-openable?).    
    return true;
}

static void whio_dev_IODEV_TYPE_finalize( whio_dev * dev )
{
    if( dev )
    {
        // Close the device and free up all resources.
        dev->api->close(dev);
        // Cleanup of internal data may optionally be done in close(),
        // depending on the requirements of the device (e.g. is it
        // re-openable?).
        free( dev->impl.data );
        dev->impl.data = 0;
        whio_dev_free(dev);
    }
}

/**
   The whio_dev_api implementation for IODEV_TYPE.
*/
static const whio_dev_api whio_dev_IODEV_TYPE_api =
    {
    whio_dev_IODEV_TYPE_read,
    whio_dev_IODEV_TYPE_write,
    whio_dev_IODEV_TYPE_close,
    whio_dev_IODEV_TYPE_finalize,
    whio_dev_IODEV_TYPE_error,
    whio_dev_IODEV_TYPE_clear_error,
    whio_dev_IODEV_TYPE_eof,
    whio_dev_IODEV_TYPE_tell,
    whio_dev_IODEV_TYPE_seek,
    whio_dev_IODEV_TYPE_flush,
    whio_dev_IODEV_TYPE_trunc,
    whio_dev_IODEV_TYPE_ioctl
    };


/**
   Initializer object for new IODEV_TYPE instances.
*/
static const whio_dev whio_dev_IODEV_TYPE_init =
    {
    &whio_dev_IODEV_TYPE_api,
    { /* impl */
    0, /* data. Must be-a (whio_dev_FILE*) */
    (void const *)&whio_dev_IODEV_TYPE_meta_init /* typeID */
    }
    };

/**
   Creates an IODEV_TYPE. The arguments are:

   FILL THIS OUT.
*/
whio_dev * whio_dev_IODEV_TYPE_create( ... /* add your arguments here */ )
{
    whio_dev * dev = whio_dev_alloc(); // this is our 'this' object
    if( ! dev ) return 0;

    // Set up our internal metadata:
    typedef whio_dev_IODEV_TYPE_meta MetaType;
    MetaType * meta = (MetaType*)malloc(sizeof(MetaType));
    if( ! meta )
    {
        whio_dev_free(dev);
        return 0;
    }
    *dev = whio_dev_IODEV_TYPE_init;
    *meta = whio_dev_IODEV_TYPE_meta_init;
    dev->impl.data = meta;

    // Initialize dev->impl.meta with any private data here.

    return dev;
}
