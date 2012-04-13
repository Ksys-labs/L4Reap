#ifndef WANDERINGHORSE_NET_WHARGV_H_INCLUDED
#define WANDERINGHORSE_NET_WHARGV_H_INCLUDED

#include <stddef.h>
#include <stdio.h>

/** @page page_whargv_main whargv: argv parser


   Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

   License: Public Domain

   Yes, what the world needs now is another command line arguments
   parser! Here's my 30th (or so) attempt at one (but the first
   implemented in C, AFAIR).


   Example:
@code
int main( int argc, char const ** argv )
{
    whargv_entry ae;
    int i = 1;
    int arc = 1;
    for( ; (i < argc) && (arc>0); i += arc )
    {
        arc = whargv_entry_parse( argc-i, argv + i, &ae );
        if( arc < 1 ) break; // error.
        // ... do something with the ae argument ...
        // e.g.
        if( ae.is_help ) { show_help(); return 1; }
    }
    return 0;
}
@endcode

    Optionally, call whargv_global_parse() and access the arguments array via
    the whargv_global object.

*/
#ifdef __cplusplus
extern "C" {
#endif

enum whargv_constants {
/**
   Maximum size of key strings for whargv_entry objects,
   not including the trailing null byte.
*/
whargv_entry_max_key_len = 32
};

/**
   Type for holding information about a single command-line argument.
   These objects are populated the by whargv_entry_parse() routine.
*/
typedef struct whargv_entry
{
    /**
       The key of the argument, minus any leading '-' or '--'.
    */
    char key[whargv_entry_max_key_len + 1];
    /**
       The value of the entry. This normally points back to static
       memory from the argv array passed to whargv_entry_parse(),
       which typically comes (in turn) from main().
    */
    char const * val;
    /**
       Indicates that the argument is a --long-style flag.
    */
    char is_long;
    /**
       Indicates that the argument is not a flag (does not start with
       a dash). It may be a file name, command name, or similar.
    */
    char is_nonflag;
    /**
       If the argument is a single dash, this value is set to 1. If it
       is a double-dash, this is set to 2.
     */
    char is_dash;
    /**
       Indicates that the argument has a numeric key, e.g. --42
       or ---42 (for -42). If the number is explicitly signed as
       negative, this flag will be set to a value less than 0,
       else a number greater than 0.
    */
    char is_numflag;
    /**
       Indicates that the argument is one of -? or --help.
    */
    char is_help;
} whargv_entry;


/**
   Parses a single command-line argument.

   On success, the number of arguments parsed is returned (should be
   1, unless this function is extended to support the form (--arg
   VALUE)). If any arguments are 0 then -1 is returned. If no argument
   can be parsed, 0 is returned. On error, the e object is left
   in an undefined state. On success, the number of items consumed
   from argv (at most argc) is returned. Currently only a single
   token can be consumed, so the return value will be 1 on success,
   but for the sake of future code, don't assume 1.

   The values returned via the e object have the following lifetimes:

   e->val, if not null, points directly to contents in argv, so it has
   that lifetime.  e->key is a copy of the original key, owned by
   e. Each call to this routine will zero out the key, so when looping
   over a single whargv_entry object, keep in mind to copy the key if
   needed!

   The argc parameter is currently ignored - only a single argv entry
   is read. The exception is if !argc, in which case this function
   does nothing and returns 0.

   The contents of the e object on a successfull parse depend on the
   format of the argv argument, as shown below. Note that because of
   how the strings are stored, keys use an empty string to denote
   empty, whereas values may use an empty string or NULL, depending on
   the specifics of the case. (Doing it this way allows us to avoid a
   dynamic copy of the keys while allowing values with arbitrary
   lengths.)

   - -f: e->key="f", e->val=0

   - -fVALUE: e->key="f", e->val="VALUE"

   - -f=VALUE: same as above

   - --long-flag: e->key="long-flag", e->val=NULL

   - --long-flag=VALUE: e->key="long-flag", e->val="VALUE"

   - non-flag-argument: e->key="",e->val will be the argument string, e->is_nonflag=non-zero

   - -- (double-dash): e->key="", e->val=NULL, e->is_dash=2

   - - (single dash): e->key="", e->val=NULL, e->is_dash=1

   - --N, where N is an (optionally signed) integer: e->key="[+-]N" with leading sign (if
   given), e->val="[-]N" without a leading '+', e->is_numflag=-1 if the number
   is signed negative, else 1.

   - -? or --help: e->key="?" or "help", e->is_help=non-zero

   Argument keys have a maximum length of whargv_entry_max_key_len. Any
   keys over that length will be silently truncated, and e->val will
   likely poluted with the truncated part of the key.


   TODOs (maybe):

   - -f VALUE

   - --flag VALUE

   - -f [EOF or FLAG]: e->val = "1" or "true"

   - --flag [EOF or FLAG]: same as above


   Once (err... IF) these are implemented, the return value may be
   greated than 1 on success.
*/

int whargv_entry_parse( int argc, char const ** argv, whargv_entry * e );

/**
   Parses all arguments in argv into the whargv_global object. Returns
   the number of items parsed (the same value as whargv_global.argc).

   This routine is neither thread-safe nor reantrant. Ideally, it
   should not be called more than once during the lifetime of the
   application.

   Memory allocated by this function is released via an atexit()
   handler, so the whargv_global object should not be referenced
   post-main().
 */
size_t whargv_global_parse( int argc, char const ** argv );

/**
   See the whargv_global object.
 */
typedef struct whargv_global_t
{
    /**
       An array of arguments parsed (and owned) by whargv_global_parse().
    */
    whargv_entry * argv;
    /**
       The number of items in argv.
    */
    size_t argc;
} whargv_global_t;

/**
   whargv_global_parse() stores its results here.
 */
extern whargv_global_t whargv_global;




#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHARGV_H_INCLUDED */
