/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif


#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#include <wh/whio/whio_common.h>

const whio_rc_t whio_rc =
    {
    0, /* OK */
    /* -1 is reserved for SizeTError */
    -2, /* ArgError */
    -3, /* IOError */
    -4, /* AllocError */
    -5, /* InternalError */
    -6, /* RangeError */
    // -7 can be reused
    -8, /* AccessError */
    -9, /* ConsistencyError */
    -10, /* NYIError */
    -11, /* UnsupportedError */
    -12, /* TypeError */
    (whio_size_t)-1 /* SizeTError */
    };


void whio_noop_printf(char const * fmt, ...)
{
}
