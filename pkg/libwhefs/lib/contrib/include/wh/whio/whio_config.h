#ifndef WANDERINGHORSE_NET_WHIO_CONFIG_H_INCLUDED
#define WANDERINGHORSE_NET_WHIO_CONFIG_H_INCLUDED 1
/*
  Common configuration options for libwhio. Parts of the
  library can be changed/configured by editing this file.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/

#include <stdint.h> /* uintXX_t */
#include <inttypes.h> /* PRIuXX macros */
//#include <unistd.h> /* ONLY for off_t. We need a better way to do this. */
#include <sys/types.h> /* off_t on Linux */

#ifndef __cplusplus
/* Try to find a usable bool, or make one up ... */
#  ifndef WHIO_HAVE_STDBOOL
#    define WHIO_HAVE_STDBOOL 1
#  endif
#  if defined(WHIO_HAVE_STDBOOL) && !(WHIO_HAVE_STDBOOL)
#    if !defined(bool)
#      define bool char
#    endif
#    if !defined(true)
#      define true 1
#    endif
#    if !defined(false)
#      define false 0
#    endif
#  else /* aha! stdbool.h! C99. */
#    include <stdbool.h>
#  endif /* WHIO_HAVE_STDBOOL */
#endif /* __cplusplus */


#ifdef __cplusplus
#if !defined(restrict)
#  define restrict
#endif
extern "C" {
#endif

#if !defined(WHIO_DEBUG_ENABLED)
#  define WHIO_DEBUG_ENABLED 0
#endif

/** @def WHIO_DEBUG

  WHIO_DEBUG is a printf-like macro which is used for internal debugging
  of the whio library. If compiled with WHIO_DEBUG_ENABLED set to 0 then
  all debuggering output is disabled.
*/
#if WHIO_DEBUG_ENABLED
#  include <stdio.h> // printf()
#  define WHIO_DEBUG if(WHIO_DEBUG_ENABLED) printf("WHIO_DEBUG: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(WHIO_DEBUG_ENABLED) printf
#else
    extern void whio_noop_printf(char const * fmt, ...);
#  define WHIO_DEBUG if(0) whio_noop_printf
#endif


/** @def WHIO_CONFIG_ENABLE_STATIC_MALLOC
   Changing this only has an effect when building this library
   or when building extensions which want to follow these
   conventions...

   If WHIO_CONFIG_ENABLE_STATIC_MALLOC is true then certain operations
   which are normally done via malloc() and free() are instead
   done first via a static list of pre-allocated objects and then
   (if the list is all used up) fall back malloc()/free().
   This i not strictly thread-safe, but for some use cases this isn't
   significant.

   When using this we might not actually save much dynamic memory
   (e.g.  whio_dev is only 12 bytes per object on 32-bit platforms),
   but we potentially save many calls to malloc(). That depends on the
   application, of course, but this idea was implemented for libwhefs,
   where keeping mallocs down is a goal and we create an unusually
   high number of whio_dev objects. In that lib, we were able to cut a
   typical use case from 19 whio-related mallocs down to 1 (and that
   one happend in fopen(), beneath the whio_dev FILE handler, so we
   couldn't avoid it).

   Note that this approach to allocation is inherently not
   thread-safe, so if you need to create/destroy whio_dev objects from
   multiple threads, do not build with this option turned on. If you
   only create and destroy whio_dev objects within a single thread,
   this option can potentially save many calls to malloc() (and should
   perform much better).
*/
#if !defined(WHIO_CONFIG_ENABLE_STATIC_MALLOC)
#  define WHIO_CONFIG_ENABLE_STATIC_MALLOC 0
#endif

#if defined(WHIO_SIZE_T_BITS)
# error "WHIO_SIZE_T_BITS must not be defined before including this file! Edit this file instead!"
#endif

/** @def WHIO_SIZE_T_BITS

    WHIO_SIZE_T_BITS defines the number of bits used by whio's primary
    unsigned interger type. This is configurable so that certain
    client code (*cough* libwhefs *cough*) can use whio without having
    to fudge certain numeric types.
*/
#define WHIO_SIZE_T_BITS 32

/** @def WHIO_SIZE_T_PFMT

    Is is a printf-style specifier, minus the '%' prefix, for
    use with whio_size_t arguments. It can be used like this:

    @code
    whio_size_t x = 42;
    printf("The value of x is %"WHIO_SIZE_T_PFMT".", x );
    @endcode

    Using this constant ensures that the printf-style commands
    work when whio_size_t is of varying sizes.

    @see WHIO_SIZE_T_SFMT
*/

/** @def WHIO_SIZE_T_SFMT

WHIO_SIZE_T_SFMT is the scanf counterpart of WHIO_SIZE_T_PFMT.

@see WHIO_SIZE_T_PFMT
*/

/** typedef some_unsigned_int_type_which_is_WHIO_SIZE_T_BITS_long whio_size_t

whio_size_t is a configurable unsigned integer type specifying the
ranges used by this library. Its exact type depends on the value of
WHIO_SIZE_T_BITS: it will be uintXX_t, where XX is the value of
WHIO_SIZE_T_BITS (8, 16, 32, or 64).

We use a fixed-size numeric type, instead of relying on a standard type
with an unspecified size (e.g. size_t) to help avoid nasty surprises when
porting to machines with different size_t sizes.
*/

#if WHIO_SIZE_T_BITS == 8
#  warning "You're insane! You're just ASKING for overflows!"
#  define WHIO_SIZE_T_PFMT PRIu8
#  define WHIO_SIZE_T_SFMT SCNu8
    typedef uint8_t whio_size_t;
    typedef int16_t whio_off_t;
#elif WHIO_SIZE_T_BITS == 16
#  define WHIO_SIZE_T_PFMT PRIu16
#  define WHIO_SIZE_T_SFMT SCNu16
    typedef uint16_t whio_size_t;
    typedef int32_t whio_off_t;
#elif WHIO_SIZE_T_BITS == 32
#  define WHIO_SIZE_T_PFMT PRIu32
#  define WHIO_SIZE_T_SFMT SCNu32
    typedef uint32_t whio_size_t;
    typedef int64_t whio_off_t;
#elif WHIO_SIZE_T_BITS == 64
#  define WHIO_SIZE_T_PFMT PRIu64
#  define WHIO_SIZE_T_SFMT SCNu64
    typedef uint64_t whio_size_t;
    typedef uint64_t whio_off_t;
#else
#  error "WHIO_SIZE_T_BITS must be one of: 8, 16, 32, 64"
#endif

/** @def WHIO_VOID_PTR_ADD()
   WHIO_VOID_PTR_ADD() is a workaround for gcc's -pedantic mode
   and other compilers which warn when void pointers are used
   in addition.
*/
#  define WHIO_VOID_PTR_ADD(VP,PLUS) ((void*)((unsigned char *)(VP)+(PLUS)))
/** @def WHIO_VOID_CPTR_ADD()
   Equivalent to WHIO_VOID_PTR_ADD() but applies to a const void
   pointer.
*/
#  define WHIO_VOID_CPTR_ADD(VP,PLUS) ((void const*)((unsigned char const *)(VP)+(PLUS)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHIO_CONFIG_H_INCLUDED */
