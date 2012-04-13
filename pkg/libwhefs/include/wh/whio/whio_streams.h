#ifndef WANDERINGHORSE_NET_WHIO_STREAMS_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_STREAMS_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/

#include "whio_stream.h"
#include "whio_dev.h"

/*
   This file contains declarations and documentation for the concrete
   whio_stream implementations. The generic stream API is declared in
   whio_stream.h.
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
   Creates a stream object which wraps the given whio_dev object.
   If takeOwnership is true then ownership of dev is passed to the
   new object. dev must outlive the returned object or results
   are undefined.

   Returns 0 if !dev or on an alloc error, otherwise it returns
   a new object which the caller must eventually free using
   str->api->finalize(str). If the stream owns dev then dev will be
   destroyed at that point, too.
*/
whio_stream * whio_stream_for_dev( whio_dev * dev, bool takeOwnership );

/**
   Creates a whio_stream wrapper around the given FILE handle. If fp
   was opened in read mode, it is illegal to use the stream in a write
   context (but this routine cannot check that). Likewise, if it was
   open for write mode, it is illegal to use the stream in a read
   context (again, this code cannot check that).

   The takeOwnership argument determines the ownerhsip of fp: if the
   function succeeds and takeOwnership is true then fp's ownership is
   transfered to the returned object. In all other cases ownership is
   not changed.

   The returned stream object must be destroyed by calling
   stream->destroy(stream). If the stream owns the FILE handle then
   that will close the FILE handle.

   If you want to write to stdout, simply use:

   @code
   whio_stream * out = whio_stream_for_FILE(stdout, false);
   @endcode

   And similar for stdin or stderr.
*/
whio_stream * whio_stream_for_FILE( FILE * fp, bool takeOwnership );

/**
   Works like whio_stream_for_FILE(), except that it accepts a
   filename and a file open mode argument (the same as expected by
   fopen()), and the stream takes over ownership of the underlying
   FILE handle.

   If allocation of the new stream or fopen() fails then 0 is returned.

   For output streams, for mode you will normally want mode "a" (if
   you want to keep the old contents) or "w" (if you want to lose the
   old contents). For input streams, use mode "r". Optionally, you can
   append the letter 'b' to the mode string for platforms which treat
   binary and text streams differently (POSIX platforms don't and
   probably ignore the 'b').

   The returned stream object must be destroyed by calling
   stream->destroy(stream).
*/
whio_stream * whio_stream_for_filename( char const * src, char const * mode );

/**
   Similar to whio_stream_for_FILE() but it takes an existing/open
   file handle number and uses fdopen() to try to open it. On success,
   a new whio_stream object is returned which wraps that FILE
   object. On error (fdopen() or a malloc() fails) 0 is returned.

   See the man page for fdopen() for details of how this might or might not
   behave the exact same as opening a FILE handle.

   The man pages say that the open mode (e.g "r", "r+", "w+", etc.) 
   "must be compatible with" the one used for opening the file
   descriptor in the first place. They do not say what "compatible"
   means, though (e.g. are "w" and "w+" compatible?). Because of this,
   this function may or may not be able to associate a FILE handle
   with the descriptor, as we cannot know the exact flags used to open
   that stream.
*/
whio_stream * whio_stream_for_fileno( int fileno, bool writeMode );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // WANDERINGHORSE_NET_WHIO_STREAMS_H_INCLUDED
