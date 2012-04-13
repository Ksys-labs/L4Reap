#ifndef WANDERINGHORSE_NET_WHIO_COMMON_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_COMMON_H_INCLUDED
/*
  Common API declarations for the whio API.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/

#include "whio_config.h"
//#include <stdio.h>
//#include <unistd.h> /* off_t on Linux? */
//#include <sys/types.h> /* off_t on Linux? */

#include <stdint.h> /* uint32_t */
#include <stdarg.h> /* va_list */



#ifdef __cplusplus
extern "C" {
#endif

/** @struct whio_rc_t

   Functions in the api which return an int almost always return a
   value from the whio_rc object. All members of this type must have
   a non-zero value, except for the OK member, which must be 0. The
   values and signedness for all error values is unspecified except
   for SizeTError, which is defined as -1.
*/
typedef struct whio_rc_t
{
    /**
       The non-error value, always equals 0.
    */
    int OK;

    /**
       Error in argument handling (e.g. unexpected arg type, count,
       etc.).
     */
    int ArgError;

    /**
       Read or write error.
    */
    int IOError;

    /**
       Memory allocation error.
    */
    int AllocError;

    /**
       An internal error in the API.
    */
    int InternalError;

    /**
       An out-of-range error. e.g. wrong size or value.
    */
    int RangeError;

    /**
       A requested resource could not be accessed, or write
       permissions are required but denied.
    */
    int AccessError;

    /**
       A data consistency error (or a bug makes the data look
       corrupted even though it is not).
    */
    int ConsistencyError;

    /**
       Indicates that the called routine is not yet implemented.
    */
    int NYIError;

    /**
       Indicates that the requested option or operation is not
       supported.
    */
    int UnsupportedError;

    /**
       Indicates some form of type mismatch or an unexpected type.
    */
    int TypeError;

    /**
       This is equivalent to (whio_size_t)-1, and is used by routines which
       need an error value for a whio_size_t object.
    */
    whio_size_t SizeTError;
} whio_rc_t;

/** @var whio_rc
   A shared instance of whio_rc_t which contains the "official" values
   of the common error codes for the whio API.
*/
extern const whio_rc_t whio_rc;

/** @struct whio_client_data

whio_client_data is an abstraction for tying client-specific data to a
whio_dev or whio_stream object. The data is not used by the public
whio_dev/whio_stream API with one exception: when
whio_dev_api::close() or whio_stream_api::close() is called, the
implementation must clean up this data IFF the dtor member is not
0. For example:

  @code
  if( my->client.dtor ) {
    my->client.dtor( my->client.data );
    my->client = whio_client_data_empty; // zero it out
  }
  @endcode
   
*/
struct whio_client_data
{
    /**
       Arbitrary data associated with an i/o device or stream.

       This data is for sole use by whio_dev and whio_stream clients,
       with one important exception: if dtor is not 0 then device
       implementations take that as a hint to destroy this object
       using that function.

       The object pointed to by client.data should not do any i/o on
       this stream or any stream/device during its destructor. Since
       client.data can point to arbitrary objects, it is impossible
       for this API to ensure that they are destroyed in a kosher
       order.  Thus it is the client's responsibility to use
       client.data wisely and carefully. A good use would be for a
       client-side buffer, e.g.  to implement buffered readahead. A
       bad use would be using it to store links to other i/o devices,
       as the destructor would presumably then close or flush them and
       they might not be live at that point. Device implementors should
       use whio_impl_data to store "more private" data.
    */
    void * data;
    /**
       If this function is not 0 then whio_dev implementations
       MUST call this function, passing the data member to it,
       in their cleanup routines. If it is 0 then they must
       ignore the data member.
    */
    void (*dtor)( void * );
};
typedef struct whio_client_data whio_client_data;
/**
   Static initializer for whio_client_data objects.
*/
#define whio_client_data_empty_m {0/*data*/,0/*dtor*/}

/**
   An empty whio_client_data object for use in initialization
   of whio_client_data objects.
*/
extern const whio_client_data whio_client_data_empty;

/**
   Holds private implementation details for whio_dev instances. Each
   instance may (and in practice always does) store instance-specific
   data here. These data are for use only by the functions related to
   this implementation and must be cleaned up via
   dev->api->close(dev).
*/
struct whio_impl_data
{
    /**
       data is SOLELY for use by concrete implementations of
       whio_stream and whio_dev, and not by clients of those types.
       
       This field can be used to store private data required by the
       implementation functions. Each instance may have its own
       information (which should be cleaned up via the close() member
       function, assuming the object owns the data).

       This data should be freed in the owning object's close()
       routine.
    */
    void * data;

    /**
       A type identifier for use solely by whio_dev and whio_stream
       implementations, not client code. If the implementation uses
       this (it is an optional component), it must be set by the
       whio_dev/whio_stream initialization routine (typically a
       factory function).

       This mechanism works by assigning some arbitrary opaque value
       to all instances of a specific whio_dev implementation. The
       implementation funcs can then use that to ensure that they are
       passed the correct type. The typeID need not be public, but may
       be so if it should be used by third parties to confirm that
       whio_dev/whio_stream objects passed to them are of the proper
       type.
    */
    void const * typeID;
};
/**
   Static initializer for whio_impl_data objects.
*/
#define whio_impl_data_empty_m {0/*data*/,0/*dtor*/}
typedef struct whio_impl_data whio_impl_data;
/**
   Empty initializer object for whio_impl_data.
*/
extern const whio_impl_data whio_impl_data_empty;

/**
   Tries to convert an fopen()-compatible mode string to a number
   compatible with whio_dev::iomode() and whio_stream::iomode().

   Returns a positive number of mode appears to be writeable,
   0 if it appears to be read-only, and a negative value if it
   cannot determine the mode.

   This function is intended for use with whio_dev/whio_stream
   factories which use an fopen()-like mode string.
*/
short whio_mode_to_iomode( char const * mode );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHIO_COMMON_H_INCLUDED */
