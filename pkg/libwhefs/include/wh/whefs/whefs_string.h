#if !defined(WANDERINGHORSE_NET_WHEFS_STRING_H_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_STRING_H_INCLUDED 1
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/

/** @typedef uint16_t whefs_string_size_t
   
whefs_string_size_t represents the maximum length of strings in
whefs. In practice 8 bits is probably enough, but until whefs gets
support for directories, longer strings may sometimes be needed.
*/
typedef uint16_t whefs_string_size_t;

/** @struct whefs_string

   whefs_string is a type for storing strings. Routines in the whefs
   library which perform string operations may use this type, as
   opposed to a simple string, because it allows us to re-use the
   string's bytes at certain times instead of using malloc for every
   string.

   One way to free the memory owned by these objects is
   whefs_string_clear(). Alternately, one may pass the string member
   to free() (assuming it was created using malloc()), then set the
   length and alloced members to 0. If the object is part of a chain,
   the caller must free each object in the chain.

   If the maximum size of a set of strings is known in advance it is
   possible to avoid any allocation by using this approach:

   @code
   enum { bufSize = MaxPossibleSizeOfStrings + 1 }; // +1 for the trailing NULL
   char buf[bufSize];
   memset( buf, 0, bufSize );
   whefs_string s = whefs_string_empty;
   s.string = buf;
   s.alloced = bufSize;
   whefs_string_copy_cstring( &s, sourceString );
   assert( (s.string == buf) );
   @endcode

   Use this approach ONLY when working with strings with a known fixed
   upper length, or else the whefs_string API might try to realloc()
   the static buffer and will likely segfault in doing so. e.g. in
   whefs we know the maximum length of filenames at compile time, and
   can optimize out some calls to malloc() by using stack-allocated
   buffers.
*/
struct whefs_string
{
    /**
       The string data for this object. The memory must be freed
       by whomever owns this object.
     */
    char * string;
    /**
       The length of the string member.
    */
    whefs_string_size_t length;
    /**
       Number of bytes allocated for the string member.
    */
    whefs_string_size_t alloced;
    /**
       Pointer to the next string in the list. Used by
       routines which return multiple strings.
    */
    struct whefs_string * next;
};
typedef struct whefs_string whefs_string;

/**
   Empty whefs_string initialization macro for places where static
   initialization is required (e.g. member whefs_string objects).
*/
#define whefs_string_empty_m { 0, 0, 0, 0 }

/**
   Empty initialization object.
*/
extern const whefs_string whefs_string_empty;

/**
   Copies the null-terminated string str into tgt. Neither src nor tgt
   may be null, but src may be a length-zero string.

   If tgt->alloced is greater than str's length then tgt's allocated
   memory is re-used for the copy, otherwise tgt->allocated is adjusted
   to fit using realloc().

   On success, whefs_rc.OK is returned and tgt is modified. On error,
   some other value is returned and tgt is not modified. tgt->next is
   never modified by this routine.

   Example usage:

   @code
   whefs_string str = whefs_string_empty; // Important! See below!
   whefs_string_copy_cstring( &str, "Hi, world!!" );
   ...
   whefs_string_copy_cstring( &str, "Bye, world!" );
   // ^^^^ Re-uses str->string's memory
   ...
   whefs_string_clear( &str, false );
   @endcode

   BE CAREFUL not to pass a pointer to an uninitialized whefs_string
   objects. If the members of the object have uninitialized values
   this function may attempt to malloc() a huge amount of memory, use
   an uninitialized string, or otherwise misbehave (been there, done
   that!). Use the whefs_string_empty object to initialize new
   whefs_string objects, as shown in the example above.

*/
int whefs_string_copy_cstring( whefs_string * tgt, char const * str );

/**
   Clears tgt's contents, freeing up any used memory, but does not
   destroy tgt.

   If clearChain is true then tgt->next is recursively cleared as
   well.
*/
int whefs_string_clear( whefs_string * tgt, bool clearChain );

/**
   Similar to whefs_string_clear(), but also frees tgt. If clearChain
   is true then tgt->next is recursively finalized as well.

   Passing a tgt of null will result in a harmless non-OK return
   value.

   NEVER pass this an object which was allocated on the stack,
   as free() (or some equivalent) will be called on it.

*/
int whefs_string_finalize( whefs_string * tgt, bool clearChain );


/**
   Allocates and zero-initializes a whefs_string. It must eventually be
   passed to whefs_string_finalize() to free the memory.
*/
whefs_string * whefs_string_alloc();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // WANDERINGHORSE_NET_WHEFS_STRING_H_INCLUDED
