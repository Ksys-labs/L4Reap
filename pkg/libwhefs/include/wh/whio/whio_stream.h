#ifndef WANDERINGHORSE_NET_WHIO_STREAM_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_STREAM_H_INCLUDED 1

/*
   This file contains declarations and documentation for the generic
   whio_stream API. The specific stream implementations are declared
   in whio_streams.h.
*/

/** @page page_whio_stream whio_stream C Stream API

  The whio_stream API provides an abstract interface for sequential
  streams which are either read-only or write-only. In practice, this
  type of stream is often the only type an application has access to
  for certain operations (as opposed to a full-fledged i/o device with
  random access, as modelled by the whio_dev API). This API is similar
  to that of whio_dev, but is somewhat smaller (because sequential
  streams have fewer requirements than random-access streams do).

   Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

   License: Public Domain

 */
#include <stdarg.h> /* va_list */

#include "whio_common.h"
#ifdef __cplusplus
extern "C" {
#endif

struct whio_stream;
/** @struct whio_stream_api

   whio_stream_api defines the "member functions" of the whio_stream
   class. It is an abstract interface for sequential streams.

   @see whio_stream
*/
struct whio_stream_api
{
    /**
       read() must read (at most) count bytes from its underlying
       source and write them to dest. It may read more than count
       (e.g. buffering) but must not write more than count bytes to
       dest, nor may it actually consume more bytes than that.

       It must return the number of bytes read, or 0 on EOF or error.
    */
    whio_size_t (*read)( struct whio_stream * self, void * dest, whio_size_t count );

    /**
       write() tries to write count bytes from src to its underlying
       destination. Returns the number of bytes written, 0 if it cannot
       write, or a number smaller than count on error.       
    */
    whio_size_t (*write)( struct whio_stream * self, void const * src, whio_size_t count );

    /**
       Close must close the stream, such that further reads or writes will
       fail. It must also free any resources owned by the instance, but must
       not free the self object.

       The interface requires that finalize() call close(), so normally
       client code does not need to call this. It is provided to allow
       for stack-allocated stream objects which otherwise could not
       be cleaned up uniformly.

       If dev->client.dtor is not null then this routine must call
       that function and pass it dev->client.data. If it is null then
       dev->client.data must not be modified (the lack of a destructor
       function is a signal that the client owns the object).

       This function should returned false if !self or if the stream is
       not opened.
    */
    bool (*close)( struct whio_stream * self );

    /**
       finalize() must call close() and then free the self object.
       After finalize() returns, self is not a valid
       object.

       The proper way to destroy a whio_stream object is:

       @code
       theStream->api->finalize(theStream);
       @endcode

       Implementations of this function must ensure that they meet
       that expectation.

       DO NOT call this on a stack-allocated object - use close()
       instead (which is provided primarily for stack-allocated
       objects).
    */
    void (*finalize)( struct whio_stream * self );

    /**
       Flushes the write buffer (for write streams). On success it
       must return whio_rc.OK. On error it returns an
       implementation-defined non-zero value.
    */
    int (*flush)( struct whio_stream * self );

    /**
       isgood() returns whether or not self is in a valid use state.
       It should return true on eof, as eof is not strictly an error.
       To report EOF it should return 0 from the read()
       implementation.
    */
    bool (*isgood)( struct whio_stream * self );

    /**
       This function must return a positive number if self is writable,
       0 if it is read-only, and a negative number if the device
       cannot report this information or if the given argument is null
       or otherwise invalid.
    */
    short (*iomode)( struct whio_stream * dev );
};

typedef struct whio_stream_api whio_stream_api;

/** @struct whio_stream

   whio_stream is an abstract interface for sequential streams. There
   is no default implementation - custom implementations must be
   provided which can handle specific stream types, e.g. FILE handles,
   an in-memory buffer, or a socket.

   The proper way to create any stream instance is from a factory
   function. The function may take any arguments necessary for
   constructing a new stream (or connecting to an existing one). For
   example, to create a stream for a FILE handle we might do:

   @code
   whio_stream * theStream = whio_stream_for_FILE(stdin,false);
   @endcode

   The public API of a stream object is accessed like:

   @code
   theStream->api->write( theStream, ... );
   @endcode

   (The first parameter as the equivalent of a "this" pointer,
   so we can get at instance-specific data.)

   For an explanation of why the "extra" api member exists, see the
   documentation for the whio_dev interface, which uses this same
   technique.

   The proper way to destroy a whio_stream object is:

   @code
   theStream->api->finalize(theStream);
   @endcode

   Implementations are responsible for properly implementing the
   finalize() member. Ownership of the underlying native stream is
   defined by the factory function which creates the stream.

   For examples of creating concrete implementations of this type,
   see the files whio_stream_FILE.c and whio_stream_dev.c.
*/
struct whio_stream
{
    /**
       Holds all "member functions" of this interface.  It is never
       legal for api to be NULL, and if a device with a NULL api
       member is used with the whio API then a segfault will certainly
       quickly result.
    */
    const whio_stream_api * api;
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
};
/**
   Convenience typedef.
*/
typedef struct whio_stream whio_stream;

/**
   Empty initialization object for whio_streams.
*/
extern const whio_stream whio_stream_empty;

/**
   An initializer macro for use in whio_stream subclass struct
   initializers.

   The arguments correspond to the five member functions,
   followed by the type ID (which may be set to 0 if factory
   code will later set it to a valid value).
*/
#define whio_stream_empty_m(Read,Write,Destroy,Flush,IsGood,TypeID)	\
    { { Read, Write, Destroy, Flush, IsGood, { 0, TypeID } }

/**
   Equivalent to whio_dev_writefv() except that it takes a whio_stream
   object instead of a whio_dev.
*/
whio_size_t whio_stream_writefv(whio_stream * stream, char const * fmt, va_list args );

/**
   Equivalent to whio_stream_writefv() except that it takes an (...) 
   elipses list instead of a va_list.
*/
whio_size_t whio_stream_writef( whio_stream * stream, char const * fmt, ... );

/**
   Convenience function to read the next character from a whio_stream. If tgt
   is not 0 then it is assigned to the value of the character.
   Returns true if it reads a character, else false.

   Example:

   @code
   char x = 0;
   if( whio_stream_getchar( myStream, &x ) ) {  ... }
   @endcode
*/
bool whio_stream_getchar( whio_stream * stream, char * tgt );

/**
   Copies all data from istr to ostr, stopping only when
   istr->api->read() returns fewer bytes than requested. On success
   whio_rc.OK is returned, on error some other value.  On error this
   function unfortunately cannot report whether the failure was at the
   read or write level.

   The data is copied in chunks of some unspecified static size (hint: a few kb).
*/
int whio_stream_copy( whio_stream * restrict istr, whio_stream * restrict ostr );


/**
   Consumes stream to the first \\n character.  It appends that data, minus the newline,
   to dest. Returns the number of characters appended to dest, or 0 at EOF or on a read
   error.

   Note that the stream is consumed and the trailing newline character
   (if any) is effectively lost.
*/
//whio_size_t whio_stream_readln_membuf(whio_stream * stream, struct memblob * dest );

/**
   Functionally identical to whio_stream_readln_membuf() except that the
   line is returned as a null-termined string which the caller must
   clean up using free(). On error or EOF 0 is returned.
*/
//char * whio_stream_readln(whio_stream * stream);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // WANDERINGHORSE_NET_WHIO_STREAM_H_INCLUDED
