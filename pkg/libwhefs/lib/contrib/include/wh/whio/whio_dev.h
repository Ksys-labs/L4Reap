#ifndef WANDERINGHORSE_NET_WHIO_DEV_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_DEV_H_INCLUDED


#include "whio_common.h"
#include <stdio.h> /* FILE */
#include <stdint.h> /* uint32_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* ??? */
#include <stdarg.h> /* va_list */

/*
   This file contains declarations and documentation for the generic
   whio_dev API. The specific iodevice implementations are declared
   in whio_devs.h.
*/

/** @page page_whio_dev whio_dev I/O API

   Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

   License: Public Domain

   whio encapsulates an i/o device API. It originally developed as
   parts of two other libraries, but the parts were found to be generic
   enough (and complementary enough) to fork out into a single library.

   The API provides interfaces for working with random-access devices
   (via the whio_dev interface) and sequential streams (via the
   whio_stream interface).

   Features:

   - Easy to use - modelled after the standard FILE API.

   - Comes with back-end implementations supporting FILE handles,
   dynamic memory buffers (with the option to expand on demand),
   and client-specified memory ranges (similar to mmap()).

   - Utility functions for doing common things, such as finding the
   size of a device, encoding/decoding certain basic types, and doing
   printf-style formatted output to devices.

   - An ioctl-like interface to perform some device-specific operations
   (for those times when genericness just won't do the job).


   Requirements:

   - Some C99 features are used, but those which are are normally
   enabled by compilers even outside of C99 mode. (This of course
   depends on the compiler version.)

   - It unfortunately needs fileno() in one place. fileno() is not
   specified by C89, but is found in most (all?) UNIX environments and
   is specified by POSIX. (It is apparently impossible to truncate
   a file without either the file's name or file handle number, as opposed
   to via a FILE handle.)


   This code has been shown to compile and pass basic sanity checks on:

   - Linux x86/32 with gcc 4.3.x and tcc
   - Linux x86/64 with gcc 4.2.x and tcc
   - Solaris 10 on sparcv9 (v240) with gcc 3.4.3 and SunStudio 12

   Example:
@code
char const * fname = "myfile.out";
whio_dev * dev = whio_dev_for_filename( fname, "r+" );
dev->api->write( dev, "hi, world!\n", 11 ); // 11 = size of the data bytes
dev->api->finalize( dev ); // flush and close device

// Now read it back:
dev = whio_dev_for_filename( fname, "r" );
enum { bufSize = 1024 };
char buf[bufSize]; // input buffer
whio_size_t n = 0;
while( 0 != (n = dev->api->read( dev, buf, bufSize ) ) )
{
    printf("Read %"WHIO_SIZE_T_PFMT" bytes\n", n );
}
dev->api->finalize(dev);
@endcode

Note the use of WHIO_SIZE_T_PFMT instead of "%%u", in the printf call,
so that the code will work correctly with varying sizes of
whio_size_t.
*/

#include "whio_common.h"
#ifdef __cplusplus
extern "C" {
#endif


struct whio_dev;

/**
   whio_dev_api defines the functions for the whio_dev interface.
   Set the documentation for whio_dev for why this was abstracted
   into a separate class from whio_dev.

   Each whio_dev object has an associated whio_dev_api object. In
   practice, all instances of a specific class of whio_dev share a
   single instance of this class as their 'api' member.

   This type is not intended to be instantiated and used by client
   code.
*/
struct whio_dev_api
{
    /**
       Reads, at most, n bytes from the underlying device and writes
       them to dest. The read number of bytes is returned, or 0 at
       EOF. A short read normally indicates EOF was reached, but can
       also be the result of an i/o error.

       On a non-i/o error (e.g. !dev or !dest), implementations should
       return 0.
    */
    whio_size_t (*read)( struct whio_dev * dev, void * dest, whio_size_t n );

    /**
       Writes n bytes from src to dev, returning the number of
       bytes written. On error, the return value will differ from
       n.

       On a non-i/o error (e.g. !dev or !dest), implementations should
       return 0.
    */
    whio_size_t (*write)( struct whio_dev * dev, void const * src, whio_size_t n );

    /**
       Must close the i/o device, such that reading and writing are
       no longer possible. It is not generically possible to re-open
       a closed device.

       This routine should also deallocate any resources associated
       with the device (but not dev itself). If this routine does not
       (i.e., it defers that to finalize()) then stack-allocated
       whio_dev objects cannot be cleaned up uniformly.

       If dev->client.dtor is not null then this routine must call
       that function and pass it dev->client.data. If it is null then
       dev->client.data must not be modified (the lack of a destructor
       function is a signal that the client owns the object).

       This interface requires that close() must be called from the
       finalize() member and that close() must flush the device (if
       needed), so client code normally does not need to call
       close(). It is provided so that whio_dev objects which are
       created on the stack (which would be unusual but possibly
       useful for some cases) can be properly cleaned up.

       Ownership of the underlying native device is defined by the
       factory function (or similar) which creates the whio_dev
       wrapper.

       This function must return true if it could close the device
       or false if it cannot (e.g. if !dev or it was not opened).
     */
    bool (*close)( struct whio_dev * dev );

    /**
       Must call close() and then free dev. After calling this, dev is
       no longer a valid object.  If passed null, the results are
       undefined (but implementations are incouraged to ignore it,
       possibly emiting a debugging message).
    */
    void (*finalize)( struct whio_dev * dev );

    /**
       Returns 0 if the device is in a good state, else it returns
       an implementation-defined non-zero value. Implementations
       are encourage to use the symbolic values from whio_rc_t,
       but some implementations may be able to return exact
       error codes from the underlying device (e.g. a file handler
       could return a value from ferror() or an errno value related
       to it).
    */
    int (*error)( struct whio_dev * dev );

    /**
       Should clear any error state and return whio_rc.OK on success.
       If this function returns an error (an implementation-defined
       non-zero value), the error state should be assumed
       unrecoverable and the device should not be used.

       For compatibility with the standard FILE API (in particular,
       clearerr()), this routine should be used to clear any
       end-of-file flag (if necessary), though that is not all
       implementations have such a flag to clear.
    */
    int (*clear_error)( struct whio_dev * dev );

    /**
       Returns an implementation-defined non-zero value at eof or 0 at
       not-eof.
     */
    int (*eof)( struct whio_dev * dev );

    /** @deprecated
       Returns the current position of the device, or
       whio_rc.SizeTError on error. An an example of an error, calling
       truncate() to truncate a device may leave its cursor out of
       bounds, which may be considered an error by the underlying
       implementation (though some allow arbitrary placement but may
       fail to be able to read/write from/to the given position).

       On a non-i/o error (e.g. !dev), whio_rc.SizeTError is returned.

       This function can be removed from the API, as the equivalent
       can be done with seek(dev,0,SEEK_CUR). It may disappear
       someday.
    */
    whio_size_t (*tell)( struct whio_dev * dev );

    /**
       Sets the current position within the stream, following the same
       semantics as fseek() except that:

       - the return value is the new position.

       - If the seek would move the cursor out of bounds, it MAY be
       kept in bounds instead (i.e. at 0 or EOF). This is
       implementation-dependent (e.g. an in-memory back-end may not be
       able to grow). As a general rule, seek() is unbounded, and a
       validity check on the position is deferred until the next read
       or write.

       - Implementations MAY return whio_rc.SizeTError for the
       following cases:

       - !dev
       - whence==SEEK_SET and pos is negative.
       - whence is not one of SEEK_SET, SEEK_CUR, or SEEK_END.
       - seek fails in some other way.

       Alternately, implementations may rely on (largely undocumented)
       behaviour of the underlying implementation. e.g. fseek() does
       not document what happens on SEEK_SET with a negative value, but
       we can hope that it fails in that case.
    */
    whio_size_t (*seek)( struct whio_dev * dev, whio_off_t pos, int whence );

    /**
       Flushes the underying stream (not all streams support/need
       this). On success it must return whio_rc.OK. On error it
       returns an implementation-defined non-zero value. Devices
       which do not use flushing should return 0.
    */
    int (*flush)( struct whio_dev * dev );

    /**
       Changes the size of the device to the new size. On success,
       whio_rc.OK is returned, otherwise an implementation-specified
       non-0 value is returned.

       Whether or not it is possible to increase the size of a device
       using this function is unfortunately implementation-specified.
       See the Linux man pages for ftruncate() for interesting
       details.

       The underlying cursor position must not be changed by calling
       this function.
    */
    int (*truncate)( struct whio_dev * dev, whio_off_t size );

    /**
       ioctl() is a "back door" to give access to certain
       implementation-specific features. Devices need not support
       this, in which case they should return
       whio_rc.UnsupportedError. The second argument specifies the
       requested device-specific operation, which will normally be
       some common enum or macro value. The remaining arguments depend
       100% on the operation, and must be specified in the
       documentation for that operation.

       If the requested operation is supported then whio_rc.OK should
       be returned on success, else some error value OTHER than
       whio_rc.UnsupportedError should be returned. When passing on an
       underlying error code, there may be a code collision with
       whio_rc.UnsupportedError - that is unfortunate but unavoidable
       and we won't lose any sleep over it.

       To pass elipsis-style arguments, use whio_dev_ioctl() (and see
       that function's docs for why this one takes a va_list).
    */
    int (*ioctl)( struct whio_dev * dev, int operation, va_list args );

    /**
       This function must return a positive number if dev is writable,
       0 if it is read-only, and a negative number if the device
       cannot report this information or if the given argument is null
       or otherwise invalid.
    */
    short (*iomode)( struct whio_dev * dev );
};
typedef struct whio_dev_api whio_dev_api;



/**
   whio_dev is an interface for communicating with an underlying
   random access data store. It is modelled after the
   POSIX-standard FILE API, and it "should" be able to act as a
   wrapper for any stream type for which we can implement the
   appropriate operations. The most significant limitation is that the
   underlying device type must support random read/write access (if it
   does not, not all operations of this type will be meaningful).

   There is no "default" whio_dev implementation - each underlying
   data store needs a different implementation. This type defines the
   conventions of the interface as specifically as possible, however.

   The member functions of this struct are abstracted into the
   whio_dev_api class (via the 'api' member of this class). The
   primary reason for this is because all instances of a certain class
   of whio_devices, in practice, share a single set of implementation
   functions. By referencing the functions this way we save
   (sizeof(whio_dev_api) - sizeof(whio_dev_api*)) bytes on each instance
   of whio_dev, and in its place we have a single shared (and static)
   instance of the implementation's API object.

   Thread safety: it is never legal to use any given whio_dev instance
   from more than one thread at a single time, and doing so will
   almost certainly corrupt the internal state of the stream. e.g. its
   read/write position may be moved by another thread, causing a read
   or write to go somewhere other than desired. It is in theory not
   problematic for multiple threads to share one whio_dev instance as
   long as access to the device is strictly serialized via a
   marshaller and device positioning operations are implemented taking
   that into account.
*/
typedef struct whio_dev
{
    /**
       Holds all "member functions" of this interface.  It is never
       legal for api to be NULL, and if a device with a NULL api
       member is used with the whio API then a segfault will certainly
       quickly result.
    */
    whio_dev_api const * api;

    /**
       Holds instance-specific, implementation-dependent
       information. Not for use by client code. The
       implementation-specific close() method should free up this
       memory.
    */
    whio_impl_data impl;

    /**
       This data is for sole use by whio_dev clients, with one
       important exception: see the docs for whio_client_data for
       details.
    */
    whio_client_data client;
} whio_dev;


/**
   Works like malloc(), but beware...

   Creates an empty, non-functional whio_dev object and returns it.
   The object must be populated by the caller and must eventually
   be destroyed by calling whio_dev_free() AFTER the private parts
   of the object are cleaned up (see whio_dev_free() for details).

   Clients of whio_dev objects SHOULD NOT use this routine - it is
   intended for use by whio_dev factory/initialization routines for
   reasons explained below.

   This function is not required to be thread safe nor reentrant, and
   a given implementation may use static data if needed. If it
   dynamically allocates memory for its internal use then it "should"
   clean up that memory via an atexit() handler or custom cleanup
   mechanism beyond the scope of this interface.

   The default implementation simply returns the result of calling
   malloc(sizeof(whio_dev)).

   A side effect of the undefined allocation rules is that devices
   returned from this function may not be valid if used after
   main() exits (e.g. via an atexit() handler) because the underlying
   allocation mechanism might have been cleaned up already.

   Why this function exists:

   As part of a related project i am looking to avoid using malloc()
   when necessary. For that project i would like to provide my own
   allocation pool for whio_dev objects (which are allocated
   relatively often in that framework). So, to that end, the i/o
   device implementations which come with whio use this routine to
   allocate their objects.

   @see whio_dev_free()
*/
whio_dev * whio_dev_alloc();

/**
   BIG FAT HAIRY WARNING:

   - This function must ONLY be passed objects allocated via
   whio_dev_alloc().

   - This function DOES NOT call dev->api->finalize(dev) or
   dev->api->close(dev). Instead, this function is intended to be
   called from one of those routines after the device's private data
   has been cleaned up.

   This function effectively passes the given object to free(), but
   the implementation is free to use a different alloc/dealloc
   method. In any case, clients should treat dev as invalid after this
   call.

   @see whio_dev_alloc()
*/
void whio_dev_free( whio_dev * dev );

/**
   This function is a workaround for the whio_dev::ioctl() interface.

   The public ioctl() function on Linux looks like:

   @code
   int ioctl( int descriptor, int operation, ... );
   @endcode

   But if we give whio_dev::ioctl() that interface, as opposed to
   passing a va_list instead of elipsis, it becomes problematic to
   pass on those arguments to the underlying ioctl() function (or do
   equivalent operations). So we provide the elipsis form here and the
   va_list form in the whio_dev::ioctl() interface.
*/
int whio_dev_ioctl( whio_dev * dev, int operation, ... );

/**
   Returns the size of the given device, or whio_rc.SizeTError if !dev
   or if the device returns that code to signify that it is not
   seekable. The size is calculated by seek()ing to the end of the
   device and using that offset. Thus the device must of course
   support seeking. The device is positioned to its pre-call position
   before the function returns.
*/
whio_size_t whio_dev_size( whio_dev * dev );

/**
   Equivalent to dev->api->set( dev, 0, SEEK_SET ), but returns whio_rc.OK
   on success.
*/
int whio_dev_rewind( whio_dev * dev );

/**
   Equivalent to dev->api->write( dev, data, n ).
*/
whio_size_t whio_dev_write( whio_dev * dev, void const * data, whio_size_t n );

/**
   Positions dev to pos and then writes n bytes from data to it. May return
   either whio_rc.SizeTError (if the seek fails) or the result of calling
   dev->api->write( dev, data, n );
*/
whio_size_t whio_dev_writeat( whio_dev * dev, whio_size_t pos, void const * data, whio_size_t n );

whio_size_t whio_dev_readat( whio_dev * dev, whio_size_t pos, void * data, whio_size_t n );

/**
   Copies all of src, from the beginning to EOF, to dest, starting
   at dest's current position. Returns whio_rc.OK on success.
*/
int whio_dev_copy( whio_dev * src, whio_dev * dest );

/**
   Similar to printf() and friends, this writes a formatted string
   to the given output device. Returns the number of bytes written.
   In the general case it is not possible to know if a given conversion
   fails, so it is not possible to say how many bytes *should* have
   been written.

   The formatted data is not copied, but is instead sent (via proxy
   functions) to the destination device directly, so it may be
   arbitrarily large without any copy penalty. Nonetheless, as with
   most formatted writing, this routine has an extremely high overhead
   compared to unformatted writes.
*/
size_t whio_dev_writefv( whio_dev * dev, const char *fmt, va_list ap );

/**
   Equivalent to whio_dev_writefv() except that it takes an elipsis list
   instead of a va_list.
*/
size_t whio_dev_writef( whio_dev * dev, const char *fmt, ... );

/**
   This enum holds masks for general categories of whio_dev ioctl
   operations.  By convention, the top byte (of 4 bytes) of ioctl
   operation values is reserved for these masks. ioctl values should
   not use the top byte and should instead mask their value against
   one of these (this assists in documentation, by making it clear
   which category an ioctl command belongs to).
*/
enum whio_dev_ioctl_categories {

/**
   General-purpose ioctls which may be supported by arbitrary
   devices.
*/
whio_dev_ioctl_mask_GENERAL = 0x01000000,

/**
   ioctl's for FILE devices should have this mask.
*/
whio_dev_ioctl_mask_FILE =   0x02000000,

/**
   ioctl's for in-memory device buffers should have this mask.
*/
whio_dev_ioctl_mask_BUFFER = 0x04000000,

/**
   ioctl's for sub-device operations should have this mask.
*/
whio_dev_ioctl_mask_SUBDEV = 0x10000000,

/**
   ioctl's for locking-related operations should have this mask.
*/
whio_dev_ioctl_mask_LOCKING = 0x20000000,

/**
   ioctl's compatile with fcntl() should have this mask.
*/
whio_dev_ioctl_mask_FCNTL = 0x40000000

};


/**
   Equivalent to dev->api->read(...). If !dev, 0 is returned.
 */
whio_size_t whio_dev_read( whio_dev * dev, void * dest, whio_size_t n );

/**
   Equivalent to dev->api->write(...). If !dev, 0 is returned.
 */
whio_size_t whio_dev_write( whio_dev * dev, void const * src, whio_size_t n );

/**
   Equivalent to dev->api->eof(...). If !dev, whio_rc.ArgError  is returned.
 */
int whio_dev_eof( whio_dev * dev );

/**
   Equivalent to dev->api->tell(...). If !dev, whio_rc.SizeTError is returned.
 */
whio_size_t whio_dev_tell( whio_dev * dev );

/**
   Equivalent to dev->api->seek(...). If !dev, whio_rc.SizeTError is returned.
 */
whio_size_t whio_dev_seek( whio_dev * dev, whio_off_t pos, int whence );

/**
   Equivalent to dev->api->flush(...). If !dev, whio_rc.ArgError  is returned.
 */
int whio_dev_flush( whio_dev * dev );

/**
   Equivalent to dev->api->truncate(...). If !dev, whio_rc.ArgError is returned.
 */
int whio_dev_truncate( whio_dev * dev, whio_off_t size );

/**
   Equivalent to dev->api->finalize(...). If !dev, this function does nothing.
 */
void whio_dev_finalize( whio_dev * dev );

/**
   Equivalent to dev->api->close(...). If !dev, this function returns false.
 */
bool whio_dev_close( whio_dev * dev );


/**
   Result type for whio_dev_fetch(). It is a workaround to enable some
   auto-generated script bindings to use the read() API.  See
   whio_dev_fetch() for details.
*/
typedef struct whio_fetch_result
{
    /**
       malloc()'d memory will be placed here by whio_dev_fetch() and
       whio_dev_fetch_r().  It must be freed by calling
       whio_dev_fetch_free() (if this object is heap-allocated), or by
       passing the data member to free() (if this object is
       stack-allocated).

       It is (char *), as opposed to (void*) or (unsigned char *)
       because SWIG likes it that way.
    */
    char * data;
    /**
       Number of bytes requested in the corresponding
       whio_dev_fetch().
    */
    whio_size_t requested;
    /**
       Number of bytes actually read in the corresponding
       whio_dev_fetch().
    */
    whio_size_t read;
    /**
       Number of bytes allocated to the data member.
     */
    whio_size_t alloced;
} whio_fetch_result;

/**
   This function is a workaround to allow us to use SWIG to
   generate scriptable functions for the whio_dev::read()
   interface. It works like so:

   - it allocates a new whio_fetch_result object.
   - it allocates n bytes of memory.
   - it calls dev->api->read() to fill the memory.

   The returned object contains referneces to the allocated memory,
   the size of the request (the n parameter), the number of bytes
   actually read, and the amount of memory allocated in the obj->data
   member.

   The returned object must be passed to whio_dev_fetch_free(), or
   free(obj->data) and free(obj) must be called (in that order!).

   @see whio_dev_fetch_r()
   @see whio_dev_fetch_free()
   @see whio_dev_fetch_free_data()
*/
whio_fetch_result * whio_dev_fetch( whio_dev * dev, whio_size_t n );

/**
   Similar to whio_dev_fetch(), but the results are written to the tgt
   object (which may not be 0) and the memory buffer of that object is
   re-used if possible (or reallocated, if n is bigger than
   tgt->alloced). On success, whio_rc.OK is returned and tgt is updated.
   On error, tgt may still be updated, and still needs to be freed
   as explained in whio_dev_fetch(), and non-whio_rc.OK is returned:

   - whio_rc.ArgError = !dev or !tgt

   - whio_rc.AllocError = (re)allocation of the memory() buffer
   failed.  tgt->data might still be valid (and need to be freed), but
   could not be expanded to n bytes.

   If tgt comes from C code (as opposed to SWIG-generated script
   bindings) and is a stack-allocated object (not via malloc()), then
   the caller MUST NOT call whio_dev_fetch_free() and must instead
   call free(tgt->data) to free any allocated memory.

   PS: the '_r' suffix on this function name is for "re-use".

   @see whio_dev_fetch()
   @see whio_dev_fetch_free()
   @see whio_dev_fetch_free_data()
*/
int whio_dev_fetch_r( whio_dev * dev, whio_size_t n, whio_fetch_result * tgt );

/**
   Calls free(r->data) then free(r). DO NOT pass this a stack-allocated
   object. To clean up such an object, simply call free(obj->data)
   or call whio_dev_fetch_free_data().

   @see whio_dev_fetch()
   @see whio_dev_fetch_r()
   @see whio_dev_fetch_free_data()
*/
int whio_dev_fetch_free( whio_fetch_result * r );

/**
   Similar to whio_dev_fetch_free(), except that r->data i freed and
   r->alloced set to 0, but r is not deleted. Thus this is legal
   for stack-allocated objects, whereas whio_dev_fetch_free() is not.

   When using the fetch API from scripted code, one needs to be
   careful how the whio_fetch_results are cleaned up. Here's an
   example (in Python) which demonstrates the two ways to handle it:

   @code
import whio
fname = 'bogo.out';
f = whio.whio_dev_for_filename( fname, "w" )
sz = whio.whio_dev_write( f, "Hi, world!", 10 )
whio.whio_dev_finalize(f)
f = whio.whio_dev_for_filename( fname, "r" )
if f is None:
    raise Exception("Could not open "+fname+" for reading!")
# Approach #1: use whio_dev_fetch() to allocate an object:
frc = whio.whio_dev_fetch(f, 5);
if frc is None:
    raise Exception("Fetch failed!")
print('sizes:', frc.requested, frc.read, frc.alloced);
print('data:',frc.data);
# Now clean up the object:
whio.whio_dev_fetch_free(frc);
# Or there's a second approach:
frc = whio.whio_fetch_result(); # allocated new object
rc = whio.whio_dev_fetch_r( f, 30, frc );
rc = whio.whio_dev_fetch_free_data(frc); # note the different cleanup function!
print('fetch_free rc=',rc);
whio.whio_dev_finalize(f)
   @endcode

*/
int whio_dev_fetch_free_data( whio_fetch_result * r );

/** @enum whio_dev_ioctls
  This is the collection of "known" ioctl values for use as the second
  argument to whio_dev::ioctl(). The documentation for each entry
  explains what the third and subsuquent arguments to ioctl() must be.
  Type mismatches will lead to undefined results, very possibly
  crashes or memory corruption.

  This enum is updated as new whio_dev implementations are created or
  new ioctl commands are added.

  Here's an example which works with whio_dev objects created
  via whio_dev_for_memmap_XXX() and whio_dev_membuf(), to reveal
  the size of their associated memory buffer:

  @code
  whio_size_t sz = 0;
  int rc = whio_dev_ioctl(dev, whio_dev_ioctl_BUFFER_size, &sz );
  if( whio_rc.OK == rc ) {
    printf("Buffer size = %u\n", sz );
  }
  @endcode
*/
enum whio_dev_ioctls {

/** @var whio_dev_ioctl_GENERAL_size
   whio_dev_file() finds a device's size by calling seek()
   to get to the EOF. While this is generic, it potentially
   requires i/o. Some devices must record their length and
   therefor have constant-time access to it. Such devices should
   support this ioctl.

   The third argument to whio_dev::ioctl() MUST be a pointer
   to a whio_size_t.
*/
whio_dev_ioctl_GENERAL_size = whio_dev_ioctl_mask_GENERAL | 0x01,

/** @var whio_dev_ioctl_GENERAL_name
   Some devices may be able to report a name associated with the
   device. The second parameter must be a (char const **). The pointer
   will be assigned to the name of the device. The string's ownership
   is not generically defined, but will typically be owned by the
   one who opened the device or by the device itself (if it copies the
   name it was opened with). In any case, if the caller wants to keep
   the name, he should copy it immediately after fetching it.
*/
whio_dev_ioctl_GENERAL_name = whio_dev_ioctl_mask_GENERAL | 0x02,

/** @var whio_dev_ioctl_BUFFER_size

   Some whio_dev implementations use a buffer. Some of those can also
   report the buffer size using this ioctl.

   The 3rd parameter to the ioctl call MUST be a pointer to a whio_size_t
   object, in which the size of the buffer is stored.

   The exact meaning of the buffer is implementation dependent. e.g
   the FILE wrapper uses no buffer (though the underlying
   implementation probably does), but the membuf and memmap
   implementations do (but use them in slightly different ways).

   A growable membuf implementation returns the current allocated size
   of the buffer (which may be larger than its reported device size
   and may be changed by write or truncate operations). A non-growable
   membuf will return the fixed allocated size, which does not change
   during the life of the device.

   The memmap device wrapper will return the size of the memory range
   pointed to by the device. This number does not change during the
   life of the device.
*/
whio_dev_ioctl_BUFFER_size = whio_dev_ioctl_mask_BUFFER | 0x01,

/**
   Devices which store an internal memory buffer *might* want to expose it,
   for performance/access reasons, to the client. The argument to this ioctl
   must be a (unsigned char const **), which will be set to the start of the
   buffer's address. However, a memory buffer might be reallocated and the
   address invalidated, so it should not be stored.

   Example:

   @code
   unsigned char const * buf = 0;
   int rc = whio_dev_ioctl( dev, whio_dev_ioctl_BUFFER_uchar_ptr, &buf );
   if( whio_rc.OK == rc )
   {
       ... Use buf. It is valid until the next write() or truncate() on dev...
       ... It MIGHT be valid longer but it might be moved through reallocation...
   }
   @endcode
*/
whio_dev_ioctl_BUFFER_uchar_ptr = whio_dev_ioctl_mask_BUFFER | 0x02,

/** @var whio_dev_ioctl_FILE_fd

   A FILE-based whio_dev can use this ioctl to return the underlying
   file descriptor number. The third argument to ioctl() MUST be a
   pointer to a signed integer, in which the file descriptor is
   written.
*/
whio_dev_ioctl_FILE_fd = whio_dev_ioctl_mask_FILE | 0x01,

/** @var whio_dev_ioctl_SUBDEV_parent_dev

   Sub-device whio_dev devices interpret this as "return the parent device
   pointer through the third argument", where the third argument must
   be a (whio_dev**).
*/
whio_dev_ioctl_SUBDEV_parent_dev = whio_dev_ioctl_mask_SUBDEV | 0x01,

/** @var whio_dev_ioctl_SUBDEV_bounds_get

   Sub-device whio_dev devices interpret this as "return the
   lower/upper bounds range, relative to the parent device, through
   the third and fourth arguments, respectively", where the third and
   fourth arguments must be a (whio_size_t*). Either may be NULL, in which
   case that argument is ignored.
*/
whio_dev_ioctl_SUBDEV_bounds_get = whio_dev_ioctl_mask_SUBDEV | 0x02,

/** @var whio_dev_ioctl_FCNTL_lock_nowait

   Devices which support locking via fcntl() (or semantically
   compatible) may support this ioctl (and the related ones)
   
   The third argument to the ioctl() call MUST be a non-const (struct
   flock *) which describes the lock/unlock operation.

   whio_dev_ioctl_FCNTL_lock requests a fcntl() non-blocking lock
   (F_SETLK).
*/
whio_dev_ioctl_FCNTL_lock_nowait = whio_dev_ioctl_mask_FCNTL | 0x01,

/** @var whio_dev_ioctl_FCNTL_lock_wait

   whio_dev_ioctl_FCNTL_lock_wait requests a fcntl() blocking lock
   (F_SETLKW).

   See whio_dev_ioctl_FCNTL_lock for more details.
*/
whio_dev_ioctl_FCNTL_lock_wait = whio_dev_ioctl_mask_FCNTL | 0x02,

/** @var whio_dev_ioctl_FCNTL_lock_get

   whio_dev_ioctl_FCNTL_lock_get requests an fcntl() F_GETLK
   operation.

   See whio_dev_ioctl_FCNTL_lock for more details.
*/
whio_dev_ioctl_FCNTL_lock_get = whio_dev_ioctl_mask_FCNTL | 0x04

};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHIO_DEV_H_INCLUDED */
