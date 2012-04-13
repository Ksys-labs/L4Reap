#ifndef WANDERINGHORSE_NET_WHIO_DEVS_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_DEVS_H_INCLUDED

/*
   This file contains declarations and documentation for the concrete
   whio_dev implementations. The generic iodevice API is declared in
   whio_dev.h.
*/
#include <stdio.h> /* FILE */
#include <stdint.h> /* uint32_t */
#include <stdarg.h> /* va_list */

#include "whio_dev.h"
#ifdef __cplusplus
extern "C" {
#endif


/**
   Creates a whio_dev object which will use the underlying FILE
   handle. On success, ownership of f is defined by the takeOwnership
   argument (see below) and the returned object must eventually be finalized with
   dev->api->finalize(dev).

   For purposes of the whio_dev interface, any parts which have
   implementation-specified behaviour will behave as they do
   for the local FILE-related API. e.g. ftruncate() may or may
   not be able to extend a file, depending on the platform
   and even the underlying filesystem type.

   The takeOwnership argument determines the ownerhsip of f: if the
   function succeeds and takeOwnership is true then f's ownership is
   transfered to the returned object. In all other cases ownership is
   not changed. If the device owns f then closing the device
   will also close f. If the device does not own f then f must
   outlive the returned device.

   Peculiarities of this whio_dev implementation:

   - ioctl() is very limited. The definition of the pseudo-standard
   ioctl() does not allow us to blindly pass on elipse arguments to
   it, so we cannot simply proxy the calls.

   - See the docs for whio_dev_ioctl_FILE_fd for how to fetch the
   underlying file descriptor.

   - The iomode() member will always return -1 because it cannot know
   (without trying to write) if f is writeable.

   @see whio_dev_for_filename()
*/
whio_dev * whio_dev_for_FILE( FILE * f, bool takeOwnership );

/**
   Similar to whio_dev_for_FILE(), but takes a filename and an
   access mode specifier in the same format expected by fopen().
   In addition, the returned object internally uses the lower-level
   read(), write(), lseek(), etc. API instead of fread(), fwrite(),
   fseek(), etc. This is, for some use cases, more performant, and is
   compatible with fcntl()-style locking.

   It is ill advised to mix use of the POSIX file API and the
   lower-level APIs. The returned object uses the lower-level APIs for
   all i/o operations, using the fopen() and fclose() API only for
   opening and closing the file (because fopen()'s mode arguments are
   easier to manage).


   Peculiarities of this whio_dev implementation:

   - Because the lower-level I/O API doesn't have direct equivalents
   of feof(), ferror(), and clearerr(), devices created this way may
   behave slightly differently in some cases (involving error
   reporting) than devices created using whio_dev_for_FILE() (but
   should nonetheless behave as expected).

   - The whio_dev_ioctl_FILE_fd ioctl is supported to fetch the
   underlying file descriptor.

   - It also supports the whio_dev_ioctl_GENERAL_name ioctl() to
   report the name passed to this function. That will only work
   if the name passed to this function is static or otherwise
   stays in scope (and doesn't change addresses) for the life
   of this object.

   - It also supports the whio_dev_ioctl_FCNTL_lock_wait,
   whio_dev_ioctl_FCNTL_lock_nowait, and whio_dev_ioctl_FCNTL_lock_get
   ioctls (from the whio_dev_ioctls enum).

   - The iomode() member will return 0 or 1 depending on mode: If mode
   contains "w" or "+" then it is write mode (iomode() returns a
   positive value). If mode contains "r" and does not have a "+" then
   iomode() returns 0.


   @see whio_dev_for_FILE()
*/
whio_dev * whio_dev_for_filename( char const * fname, char const * mode );

/**
   Equivalent to whio_dev_for_filename(), but takes an opened file
   descriptor and calls fdopen() on it.

   PLEASE read your local man pages for fdopen() regarding caveats in
   the setting of the mode parameter and the close()
   handling. e.g. destroying the returned device will close it, so the
   descriptor should not be used by client code after that. Likewise,
   client code should not close the descriptor as long as the returned
   device is alive. Thus ownership of the handle is effectively passed
   to the returned object, and there is no way to relinquish it.

   My local man pages say:

   @code
   The fdopen() function associates a stream with the existing file
   descriptor, fd.  The mode of the stream (one of the values "r",
   "r+", "w", "w+", "a", "a+") must be compatible with the mode of the
   file descriptor.  The file position indicator of the new stream is
   set to that belonging to fd, and the error and end-of-file
   indicators are cleared.  Modes "w" or "w+" do not cause truncation
   of the file.  The file descriptor is not dup’ed, and will be
   closed when the stream created by fdopen() is closed.  The result
   of applying fdopen() to a shared memory object is undefined.
   @endcode


   The returned device is identical to one returned by
   whio_dev_for_filename(), except that the ioctl()
   whio_dev_ioctl_GENERAL_name will return NULL (but will succeed
   without an error).
*/
whio_dev * whio_dev_for_fileno( int filedescriptor, char const * mode );


/**
   Creates a new whio_dev object which wraps an in-memory buffer. The
   initial memory allocated by the buffer is allocated by this call.
   Whether or not the buffer is allowed to be expanded by write() or
   seek() operations is defined by the remaining parameters.

   The expFactor specifies a growth expansion value, as follows. If
   expFactor is less than 1.0 then the buffer will never be allowed to
   expand more than the original size. If it is equal to or greater
   than 1.0, then it will be made expandable (that is, write() may
   cause it to grow). Every time its size grows, it will grow by a
   factor of expFactor (or to exactly the requested amount, for a
   factor of 1.0). e.g. 1.5 means grow by 1.5 times (a common growth
   factor for dynamic memory allocation). Likewise, when a buffer
   shrinks (via truncate()), it will be reallocated if
   (currentSize/expFactor) is greater than the number of bytes being
   used. For example, if a buffer of 1024 bytes with an expFactor of
   1.5 is being shrunk, it will not release the allocated memory
   unless doing so will drop it below ((1024/1.5)=682) bytes. A very
   large expFactor (more than 2.0) is not disallowed, but may not be
   good for your sanity.

   For purposes of the following text, a membuf device with an
   expFactor of equal to or greater than 1.0 is said to be "growable".

   If the buffer is growable then calling write() when we are at (or
   past) EOF will cause the buffer to try to expand to accommodate.
   If it cannot, or if the buffer is not growable, the write operation
   will write as much as it can fit in existing memory, then return a
   short write result.

   It not enough memory can be allocated for the intitial buffer then
   this function returns 0. On success, the caller owns the returned
   object and must eventually destroy it by calling
   dev->api->finalize(dev).

   The returned object supports all of the whio_dev interface, with
   the caveat that write() calls will not be allowed to expand out of
   bounds if the device is not growable.

   Regardless of the expansion policies, the truncate() member of the
   returned object can be used to expand the buffer. See below for
   more details.

   Peculiarities of this whio_dev implementation:

   - Whether or not the device is growable (as explained above), seeks past
   EOF are always allowed as long as the range is positive. Non-growable
   buffers will not write there, but growable ones will try to expand
   at the next write. Non-growable buffers *can* be expanded by manually
   calling truncate() on them.

   - truncate() ignores the growth policy! That is by design, to allow
   us to (optionally) manually control the growth without allowing
   rogue seek/write combinations to take up all our memory.

   - When truncate() shrinks a growable buffer: memory may or may not
   be immediately released, depending on the magnitude of the
   change. A truncate() to a size of 0 will always release the memory
   immediately.

   - When truncate() shrinks a non-growable buffer: the memory is not
   released at all because the buffer could then not be expanded
   later. When truncating a non-expanding buffer to a smaller size,
   writes() made past new EOF will cause it to expand, but only up to
   the original size given to this function. e.g. if we have a non-growable
   buffer with 1024 bytes, we can truncate() it to 10 bytes, then write
   (expanding the size) up until we reach 1024 bytes, at which point
   we cannot write any more.

   - seek() will only fail if the offset would cause the position
   counter to overflow its numeric range or would set it before the
   start of the buffer. seek() does not change the object's size, and
   writing after an out of bounds seek will cause the object to grow
   starting at that new position (if it is growable) or it will fail
   (if the buffer is not growable or cannot grow).

   - flush() is a harmless no-op.

   - ioctl(): all of the ictl operations listed in the whio_dev_ioctls
   enum and marked with whio_dev_ioctl_mask_BUFFER are supported, as
   documented in that enum. The whio_dev_ioctl_GENERAL_size ioctl
   is also supported (from the whio_dev_ioctls enum), but it returns
   the size to the virtual EOF, whereas whio_dev_ioctl_BUFFER_size
   returns the allocated size of the buffer.

   - iomode() always returns a positive value as long as its "this"
   argument is valid.
*/
whio_dev * whio_dev_for_membuf( whio_size_t size, float expFactor );

/**
   Creates a new read/write whio_dev wrapper for an existing memory
   range. For read-only access, use whio_dev_for_memmap_ro() instead.

   The arguments:

   - mem = the read/write memory the device will wrap. Ownership is
   not changed.  May not be 0. If mem's address changes during the
   lifetime of the returned object (e.g. via a realloc), results are
   undefined an almost certainly ruinous. If you want this device to
   free the memory when it is destroyed, set dev->client.data to the
   mem parameter and set dev->client.dtor to an appropriate destructor
   function (free() would suffice for memory allocated via alloc()).

   - size = the size of the mem buffer. It may not be 0. It is the
   caller's responsibility to ensure that the buffer is at least that
   long. This object will not allow i/o operations outside of that
   bound. It is good practice to ensure that the memory is zeroed out
   before passing it here, to avoid unpredictable artifacts.

   On success a new whio_dev is returned. On error (invalid
   arguments or allocation error), 0 is returned.

   Peculiarities of the memmap whio_dev wrapper:

   - trunc() operations will work, but only up to the size passed to
   this function.  That is, one can "shrink" the device by truncating
   it, then grow it back, but never larger than the original size.

   - seek() accepts past-EOF ranges, but will return
   whio_rc.SizeTError on a numeric over/underflow. Writing past EOF
   range will of course not be allowed.

   - write() on a read-only memory buffer returns 0, as opposed to
   whio_rc.SizeTError.

   - Supports the ioctl()s whio_dev_ioctl_BUFFER_size, which returns
   the allocated size of the buffer (as passed to the factory
   function), and whio_dev_ioctl_GENERAL_size, which returns the
   position of the virtual EOF. It is not yet clear if we can support
   whio_dev_ioctl_BUFFER_uchar_ptr without violating constness of
   read-only buffers.

   - iomode() always returns a positive value as long as its "this"
   argument is valid.


   @see whio_dev_for_memmap_ro()
   @see whio_dev_for_membuf()
*/
whio_dev * whio_dev_for_memmap_rw( void * mem, whio_size_t size );

/**
   This is nearly identical to whio_dev_for_memmap_rw() except that it
   creates a read-only device and ownership of mem is not changed by
   calling this function.

   In addition to the description for whio_dev objects returned from
   whio_dev_for_memmap_rw(), these notes apply:

   - iomode() always returns 0 unless its "this" argument is invalid,
   in which case it returns a negative value.


   @see whio_dev_for_memmap_rw()
   @see whio_dev_for_membuf()
*/
whio_dev * whio_dev_for_memmap_ro( const void * mem, whio_size_t size );

/**
   This object is the api member used by whio_dev instances returned by
   whio_dev_for_memmap_rw() and whio_dev_for_memmap_ro(). It is in the public
   interface because there are some interesting use-cases where we want
   to override parts of the API to do custom handling.

   The address of this object is also used as the whio_dev::typeID value
   for memmap devices.
*/
extern const whio_dev_api whio_dev_api_memmap;

/**
   This object is the api member used by whio_dev instances returned
   by whio_dev_for_membuf() . It is in the public interface because
   there are some interesting use-cases where we want to override
   parts of the API to do custom handling.

   The address of this object is also used as the whio_dev::typeID value
   for membuf devices.
*/
extern const whio_dev_api whio_dev_api_membuf;


/**
   Creates a new whio_dev object which acts as a proxy for a specified
   range of another device (that is, the object acts as a
   "subdevice"). All read/write/seek/tell operations on the returned
   object act on a range which is relative to the parent object. The
   object will not allow read/write operations outside of the given
   range.

   The arguments are:

   - parent = the parent i/o device. It must outlive the returned
   object, and ownership of parent is not changed by calling this
   function.

   - lowerBound is the logical begining-of-file for the returned
   device, relative to the coordinates of the parent.

   - upperBound is the logical "hard" EOF for the returned device -
   writes will not be allowed past this point. If 0 is passed it is
   treated as "unlimited". upperBound must be 0 or greater than
   lowerBound.

   The bounds are not checked when the subdevice is created, as some
   devices will allow one to write past EOF to extend it. If the
   bounds are not legal at read/write time then the error will be triggered
   there.  If parent is ever sized such that the given bounds are
   not longer legal, read/write errors may occur.

   Subdevices can be used to partition a larger underlying device into
   smaller pieces. By writing to/reading from the subdevices, one can
   be assured that each logical block remains separate and that any
   operation in one block will not affect any other subdevices which
   have their own blocks (barring any bugs in this code or overlapping
   byte rangees). Results are of course unpredictable if multiple
   devices map overlapping ranges in the parent device (then again,
   maybe that's what you want, e.g. as a communication channel).

   Most operations are proxied directly through the parent device,
   with some offsets added to accomodate for the bounds defined, so
   the returned object will inherit any peculiarities of the parent
   implementation (e.g. flush() requirements).

   On success a new whio_dev object is returned, which must eventually be
   destroyed by calling dev->api->finalize(dev).

   On error 0 is returned.

   Peculiarities of this whio_dev implementation:

   - read() and write() will reposition the parent cursor to the
   subdevice's position before reading/writing and re-set it when
   complete. Thus other code using the parent device need not worry
   about the subdevice hosing the cursor position. reads/writes do of
   course track the internal cursor used by the subdevice, so (for
   example) one need not manually reposition the cursor when doing
   sequential reads or writes. Likewise, seek() and tell() work independently
   of the parent device (with one exception, described below).

   - The behaviour of seek(dev,n,SEEK_END) depends partially on
   upperBound. If upperBound==0 (i.e. "no bounds") then SEEK_END will
   use the EOF of the parent device, not the subdevice. In any case,
   the value returned from tell() or seek() will be relative to the
   subdevice.

   - truncate() does not function; it simply returns
   whio_rc.UnsupportedError.  It would be feasible to handle
   truncating like the memmap whio_dev does - allow shrinking and
   growing back up to the original maximum size, but no further than
   that. This might be added at some point, but it doesn't currently
   seem to be useful enough to warrant the effort.

   - Some calls (e.g. flush(), clear_error()) are simply passed on to the
   parent device. Though it is not technically/pedantically correct to
   do so in all cases.

   - ioctl(): this type accepts ioctls described in the
   whio_dev_ioctls enum having the name prefix
   whio_dev_ioctl_SUBDEV_. See those docs for details.

   - iomode() returns the same as the parent device does.

   - It is theoretically possible to nest subdevices, and it may be
   even useful in some cases, but i haven't tried it.

   Example:


   @code
    char const * fname = "subdev.iodev";
    whio_dev * parent = whio_dev_for_filename( fname, "w+" );
    assert(parent);
    parent->api->write( parent, "!", 1 );
    parent->api->seek( parent, 99, SEEK_SET );
    parent->api->write( parent, "!", 1 );
    // parent is now 100 bytes long.
    whio_dev * sub = whio_dev_subdev_create( parent, 10, 43 );
    assert(sub);
    MARKER("Subdevice in place for file [%s]...\n", fname );

    size_t i = 0;
    whio_size_t szrc = 0;
    for( ; i < 5; ++i )
    {
        szrc = sub->api->write( sub, "0123456789", 10 );
        MARKER("Write length = %u\n", szrc );
        if( szrc != 10 ) break;
    }
    sub->api->finalize(sub);
    parent->api->finalize(parent);
    // Now bytes in the inclusive range 10..42 (not 43 - that's sub's
    // hard EOF) will be filled with whatever was written to sub.
   @endcode
*/
whio_dev * whio_dev_subdev_create( whio_dev * parent, whio_size_t lowerBound, whio_size_t upperBound );



/**
   This routine only works on devices created with
   whio_dev_subdev_create() (or equivalent). It re-sets the lower and
   upper bounds of the subdevice (as described in
   whio_dev_subdev_create()) and re-sets the cursor to the new lower bound.

   This routine does no bounds checking on the parent device.

   On success, whio_rc.OK is returned, otherwise:

   - whio_rc.ArgError = !dev or upperBound is not 0 but is less than lowerBound.

   - whio_rc.TypeError = dev's type-id does not match (it is not a subdev device).

   - whio_rc.InternalError = dev is not mapped to a parent device (this theoretically
   cannot happen unless client code muddles with dev->impl.data).

   @see whio_dev_subdev_create()
*/
int whio_dev_subdev_rebound( whio_dev * dev, whio_size_t lowerBound, whio_size_t upperBound );

/** @struct whio_blockdev

   whio_blockdev is a helper intended to assist in
   partitioning of a larger i/o device. It is intended to
   be used in conjunction with a "master" i/o device
   which has logical partitions made up of fixed-sized
   records. Instead of providing a low-level i/o API, its
   API works at the "record" level, where each record
   is a block of a size specified when the object
   is initialized.

   whio_blockdev objects are initialized via whio_blockdev_setup()
   and cleaned up (depending on how they were created) using
   whio_blockdev_cleanup() or whio_blockdev_finalize().

   Though this type's internals are publically viewable,
   they should not be used by client code. Use the
   whio_blockdev family of functions instead.

   @see whio_blockdev_alloc()
   whio_blockdev_setup()
   whio_blockdev_cleanup()
   whio_blockdev_free()
   whio_blockdev_in_range()
   whio_blockdev_write()
   whio_blockdev_read()
   whio_blockdev_wipe()
*/
typedef struct whio_blockdev
{
    /**
       Info about the blocks this object manages.
    */
    struct blocks
    {
	/**
	   Size of each block. MUST NOT be changed after setting up
	   this object, and doing so may lead to errors.
	*/
	whio_size_t size;
	/**
	   Number of blocks. MUST NOT be changed after setting up
	   this object, and doing so may lead to errors.
	*/
	whio_size_t count;
	/**
	   Must be null or valid memory at least 'size' bytes
	   long. When a block is "wiped", this memory is copied over
	   that block.

	   The contents may be changed after setting up this object,
	   so long as the address stays valid (or is changed to
	   accommodate) and stays at least 'size' bytes long.
	*/
	void const * prototype;
    } blocks;
    /**
       Implementation details which the client should neither touch
       nor look at.
    */
    struct impl
    {
	/**
	   This object's i/o device. It is created via
	   whio_blockdev_setup() and cleaned up by
	   whio_blockdev_cleanup(). It is a whio_dev subdevice
	   which fences i/o operations on the parent device
	   to the range defined by whio_blockdev_setup().
	*/
	whio_dev * fence;
    } impl;
} whio_blockdev;

/**
   A whio_blockdev object for cases where static initialization is necessary
   (e.g. member whio_blockdev objects).
*/
#define whio_blockdev_empty_m {\
	{ /* blocks */ \
	    0 /*size*/,\
	    0 /*count*/,\
	    0 /*prototype*/\
	},\
	{ /* impl */ \
	    0 /*fence*/ \
	}\
    }

/**
   Empty initialization object.
*/
extern const whio_blockdev whio_blockdev_empty;


/**
   Allocates and returns a new whio_blockdev, which the caller owns,
   or 0 on OOM.  Because this function might be configured to use a
   memory source other than malloc, the object must be destroyed using
   whio_blockdev_free() instead of free().

   @see whio_blockdev_free()
*/
whio_blockdev * whio_blockdev_alloc();

/**
   Initializes the given whio_blockdev object, which must have been
   allocated using whio_blockdev_alloc() or created on the stack and
   initialized using whio_blockdev_empty or whio_blockdev_empty_m.

   bdev will use parent_store as its storage device, but will only
   access the device range [parent_offset,(block_size *
   block_count)). None of the parameters may be 0 except for
   parent_offset and prototype. If prototype is not null then it must
   be valid memory at least block_size bytes long. When a block is
   "wiped" (see whio_blockdev_wipe()), this prototype object is
   written to it.

   The parent_store object must outlive bdev. Performing any i/o on
   bdev after the parent i/o device is invalidated will lead to
   undefined results.

   On success, a call to whio_blockdev_cleanup() or
   whio_blockdev_free() must eventually be made for bdev to free up
   the internally allocated resources. See those functions for details
   on which to use.

   If bdev is passed to this function multiple times without a
   corresponding call to whio_blockdev_cleanup(), it will leak
   resources.

   Returns whio_rc.OK on success, some other value on error:

   - whio_rc.AllocError if allocation of a subdevice fails.

   - whio_rc.ArgError if any of bdev, parent_store, block_size, or
   block_count are 0.
*/
int whio_blockdev_setup( whio_blockdev * bdev, whio_dev * parent_store, whio_size_t parent_offset,
			 whio_size_t block_size, whio_size_t block_count, void const * prototype );
/**
   Works similarly to whio_blockdev_setup(), but it uses the parent
   device directly instead of an explicit subdevice, and does not
   place an upper limit on the number of blocks it may write.

   parent may be any API-compliant device, including a subdevice.
   parent must outlive bdev. All i/o on the parent device starts at
   offset 0 and happens in blocks of block_size.

   whio_blockdev_in_range() will always return true for a blockdev
   initialized this way.

   See whio_blockdev_setup() for the allocation and cleanup
   requirements of bdev.

   Returns whio_rc.OK on success or whio_rc.ArgError if
   !bdev, !parent, or !block_size.

   This routine does not allocate any memory.
   
   @see whio_blockdev_free()
*/
int whio_blockdev_setup2( whio_blockdev * bdev, whio_dev * parent, whio_size_t block_size, void const * prototype );

/**
   Cleans up internal memory owned by bdev but does not free bdev
   itself. After this, bdev may be passed to whio_blockdev_setup() to
   re-initialize it if needed.

   Returns true on success, false on error.

   If bdev is a default-initialized object then this function will
   likely attempt to free memory from an undefined memory region,
   leading to undefined behaviour.

   @see whio_blockdev_free()
   @see whio_blockdev_setup()
*/
bool whio_blockdev_cleanup( whio_blockdev * bdev );

/**
   Destroys bdev and any internal memory it owns. ONLY pass this
   object created using whio_blockdev_alloc(). DO NOT pass this an
   object which was created on the stack, as that will lead to a
   segfault. For stack-allocated objects use whio_blockdev_cleanup()
   instead.

   @see whio_blockdev_cleanup()
   @see whio_blockdev_alloc()
*/
void whio_blockdev_free( whio_blockdev * bdev );

/**
   Returns true if id is a valid block ID for bdev, else false.

   If bdev was initialized with whio_blockdev_setup2() then this
   function will always return true, with the assumption that the
   underlying device can indeed be grown if needed.
*/
bool whio_blockdev_in_range( whio_blockdev const * bdev, whio_size_t id );

/**
   Writes the contents of src to bdev at the underlying device
   position corresponding to the given block id.  On success it
   returns whio_rc.OK. It returns an error code if bdev or src are
   null, id is not in bounds, or on an i/o error. src must be valid
   memory at least bdev->blocks.size bytes long.


   Error conditions:

   - If !bdev or !src: whio_rc.ArgError is returned.

   - If whio_blockdev_in_range(bdev,id) returns false,
   whio_rc.RangeError is returned.
  
   - If writing fails, whio_rc.IOError is returned.
*/
int whio_blockdev_write( whio_blockdev * bdev, whio_size_t id, void const * src );

/**
   Reads bdev->blocks.size bytes of memory from the block with the
   given id from bdev and copies it to dest. dest must be valid memory
   at least bdev->blocks.size bytes long. Returns whio_rc.OK on success.
*/
int whio_blockdev_read( whio_blockdev * bdev, whio_size_t id, void * dest );

/**
   This is equivalent to whio_blockdev_wipe(bdev,id,bdev->impl.prototype).
*/
int whio_blockdev_wipe( whio_blockdev * bdev, whio_size_t id );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHIO_DEVS_H_INCLUDED */
