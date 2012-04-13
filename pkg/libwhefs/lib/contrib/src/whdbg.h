#ifndef WANDERINGHORSE_NET_WHDBG_H_INCLUDED
#define WANDERINGHORSE_NET_WHDBG_H_INCLUDED 1

/** @page whdbg_page_main whdbg: Debugging Library for C

@section whdbg_sec_about About whdbg

Authors: Stephan Beal (http://wanderinghorse.net/home/stephan/)

License: the core code is in the public domain

Example usage:
@code
// in main():
whdbg_set_stream( stderr );
whdbg_set_flags( whdbg_get_flags() | WDBG_INFO );

// ... in an arbitrary function ...
int x = 42;
WHDBG(WHDBG_INFO)("x=%d", x);
@endcode

That will send something like the following to the specified debug stream:

@verbatim
whdbg(WHDBG_INFO):myfile.c:392:my_function():
    x=42
@endverbatim

The flags passed to WHDBG() and whdbg_set_flags(), and returned by
whdbg_get_flags(), need not come from the predefined WHDBG_xxx flags -
it they can be an arbitrary bitmask. The exceptions are that the value
0 is reserves for "never output" and WHDBG_ALWAYS is reserved for
"always output".

If this library is compiled with WHDBG_CONFIG_ENABLE set to a false
value then debugging is disabled in a way such that all calls to
WHDBG() "should" be optimized out by the compiler.

*/

#include <stdarg.h>
#include <stdio.h> /* FILE */

#ifdef __cplusplus
extern "C" {
#endif



/** @def WHDBG_CONFIG_ENABLE

   If WHDBG_CONFIG_ENABLE is set to a true value then whdbg()
   and friends are activated. If it is a false value then whdbg()
   (and friends) become no-ops, and calls to WHDBG() resolve to
   complete no-ops (no function calls) and should be optimized by the
   compiler out altogether.
*/
#if !defined(WHDBG_CONFIG_ENABLE)
#if !defined(NDEBUG)
#  define WHDBG_CONFIG_ENABLE 1
#else
#  define WHDBG_CONFIG_ENABLE 0
#endif
#endif

/** @enum whdbg_flags

The whdbg_flags enum contains a bitmask of logging/debugging flags,
for use with whdbg() and friends.


FIXME: clearly define message categories and IDs. The current system
is not at all extendible from client code.
*/
enum whdbg_flags {

/**
   Never log any debug message.
*/
WHDBG_NEVER = 0x00000000,

/**
   General error mask.
*/
WHDBG_ERROR = 0x10000000,

/**
   Log allocation events. This is inherently
   dangerous, as logging can cause an alloc.
*/
WHDBG_ALLOC_ERR = WHDBG_ERROR | 0x01,

/**
   Log error messages.
*/
WHDBG_IO_ERROR = WHDBG_ERROR | 0x02,

/**
   General warning mask.
*/
WHDBG_WARNING = 0x20000000,
WHDBG_NYI =    WHDBG_WARNING | 0x01,

WHDBG_INFO = 0x40000000,

/**
   "For your information..." or "To whom it may concern..."
*/
WHDBG_FYI = WHDBG_INFO,
/**

   Log for allocation actions.
*/
WHDBG_ALLOC = WHDBG_INFO | 0x0001,

/**
   Log deallocation events.
*/
WHDBG_DEALLOC =  WHDBG_INFO | 0x0002,

/**
   FIXME markers
 */
WHDBG_FIXME =  WHDBG_INFO | 0x04,

/**
   For apps with a "verbose" flag.
*/
WHDBG_VERBOSE =  0x00010000,
/**
   Factory-related messages (e.g. registration).
 */
WHDBG_FACTORY =  0x00020000,

/**
   Log any memory-related events.
*/
WHDBG_MEMORY = WHDBG_DEALLOC | WHDBG_ALLOC_ERR | WHDBG_ALLOC,

/**
   This range is reserved for client application use, as are
   variables/enum entries named WHDBG_APP_xxx.
*/
WHDBG_LAST = 0x01000000,

/**
   Default debug level.
*/
WHDBG_DEFAULT = WHDBG_WARNING | WHDBG_ERROR,

/**
   Log anything except WHDBG_NEVER.
*/
WHDBG_ALWAYS = 0x7fffffff
};



/** @internal

   Internal typedef. Part of the WHDBG macro definition, and not
   to be used outside of that context. Used like fprintf(), with
   the stream being that set via whdbg_set_stream(). If no stream
   is set, no output is generated.
*/
typedef void (*whdbg_PRIVATE_f)( char const * fmt, ... );

/** @internal

   Internal function. Part of the WHDBG macro definition, and not
   to be used outside of that context.

   This routine is not thread-safe, and there may be collisions in the
   logger if multiple threads use this API at once. In a best
   case you'd get wrong file location information. In a worst case,
   a crash.
*/
whdbg_PRIVATE_f whdbg_PRIVATE_phase1( unsigned int condition,
					    char const * condString,
					    char const * file,
					    unsigned int line,
					    char const * funcName );

/**
   WHDBG() is a much simplified way of calling whdbg(). It is used like this:

   @code
   WHDBG(WHDBG_WHATEVER|WHDBG_WHATEVER_ELSE)("num=%d val=%s",num,val);
   @endcode

   Note the extra set of parenthesis.

   As a log message it will use the string form of the flags instead
   of the numeric form.

   If WHDBG_CONFIG_ENABLE is set to a false value this macro evaluates
   to an if(false) block and "should" be compiled out by compilers.

*/
#define WHDBG(COND) if(WHDBG_CONFIG_ENABLE) whdbg_PRIVATE_phase1( COND, # COND, __FILE__, __LINE__, __func__ )

/**
   WHDBG_FLAGS() is a simpler way to pass the required arguments to
   whdbg(). Instead of:

   @code
   whdbg(WHDBG_WARNING,WHDBG_SOURCE_INFO,"...",...);
   @endcode

   Try:

   @code
   whdbg(WHDBG_FLAGS(WHDBG_WARNING),"...",...);
   @endcode

   Or a bit friendlier:

   @code
   WHDBG(WHDBG_WARNING)("...", ...);
   @endcode

*/
#define WHDBG_FLAGS(CONDITION) (CONDITION),WHDBG_SOURCE_INFO

/**
   Convenience macro to log allocations. Takes a size (unsigned int) argument.
*/
#define WHDBGME_ALLOC(SIZE) WHDBG(WHDBG_ALLOC)("alloc of %u bytes",(SIZE))

/**
   Convenience macro to log allocations. Takes a typename argument.
*/
#define WHDBGME_ALLOCT(TYPE) WHDBG(WHDBG_ALLOC)("alloc of %s object: %u bytes",# TYPE, sizeof(TYPE))

/**
   Convenience macro to log allocations. Takes a typename argument followed by
   a pointer to an object of that type.
*/
#define WHDBGME_ALLOCTP(TYPE,PTR) WHDBG(WHDBG_ALLOC)("alloc of %s object @%p: %u bytes",# TYPE, (PTR), sizeof(TYPE))

/**
   De-alloc form of WHDBGME_ALLOC.
*/
#define WHDBGME_DEALLOC(SIZE) WHDBG(WHDBG_DEALLOC)("dealloc of %u bytes",(SIZE))

/**
   De-alloc form of WHDBGME_ALLOCT.
*/
#define WHDBGME_DEALLOCT(TYPE) WHDBG(WHDBG_DEALLOC)("dealloc of %s object: %u bytes",# TYPE, sizeof(TYPE))

/**
   De-alloc form of WHDBGME_ALLOCTP.
*/
#define WHDBGME_DEALLOCTP(TYPE,PTR) WHDBG(WHDBG_DEALLOC)("dealloc of %s object @%p: %u bytes",# TYPE, (PTR), sizeof(TYPE))

/**
   Gets the current debug mask. See the whdbg_flags enum for
   the ones provided byt his library.

   @see whdbg_set_flags()
*/
unsigned int whdbg_get_flags();

/**
   Sets the current debug mask and returns the previous one. See the
   whdbg_flags enum for details.

   The flags passed to WHDBG() and whdbg_set_flags(), and returned by
   whdbg_get_flags(), need not come from the predefined WHDBG_xxx
   flags - it they can be an arbitrary bitmask. The exceptions are
   that the value 0 is reserves for "never output" and WHDBG_ALWAYS is
   reserved for "always output".

    @see whdbg_get_flags()
*/
unsigned int whdbg_set_flags(unsigned int newflags);

/**
   Sets the debug output stream to f. Setting it to 0 completely
   disables debug output sent via whdbg() and friends.

   Does not transfer ownership of f, and f must outlive all
   debugging output or be replaced before being closed.
*/
void whdbg_set_stream( FILE * f );

/** @def WHDBG_SOURCE_INFO
   This macro should always be pass as the 2nd argument
   to whdbg() and whdbgv().
*/
#define WHDBG_SOURCE_INFO __FILE__,__LINE__,__func__

/** @def WHDBG_SOURCE_INFO_PARAMS_DECL
   For use by whdbgv() and friends.
*/
#define WHDBG_SOURCE_INFO_PARAMS_DECL char const *_file, unsigned int _line, char const *_func

/**
   If (condition & whdbg_get_flags()) and a debug stream has been
   set via whdbg_set_stream(), then the formatted string, followed
   by a newline character is sent to the debug channel, otherwise
   nothing happens.

   Always pass the macro WHDBG_SOURCE_INFO as the second argument.

   As a special case, if either condition or whdbg_get_flags() are
   equal to WHDBG_ALWAYS then the message is always sent regardless
   of the current debug mask (provided there's a stream to send it
   to).

   The whdbg API internally uses whdbg() extensively to send
   messages.  This function isn't *really* for client-side use, but it
   interesting for debuggering purposes and for add-on whdbg code (like
   the i/o support), so its part of the public API.
*/
void whdbgv( unsigned int condition,
		WHDBG_SOURCE_INFO_PARAMS_DECL,
		char const * fmt,
		va_list vargs );
/**
   Identical to whdbgv() but takes an ellipses list instead of
   a va_list.
*/
void whdbg( unsigned int condition,
	       WHDBG_SOURCE_INFO_PARAMS_DECL,
	       char const * fmt,
		... );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHDBG_H_INCLUDED */
