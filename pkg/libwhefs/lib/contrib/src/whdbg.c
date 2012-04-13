
#include <stdlib.h>

#include "whdbg.h"

#ifdef __cplusplus
#define ARG_UNUSED(X)
extern "C" {
#else
#define ARG_UNUSED(X) X
#endif

static int whdbg_flags = WHDBG_DEFAULT;
static FILE * whdbg_stream = 0;

void whdbg_set_stream( FILE * f )
{
    whdbg_stream = f;
}

unsigned int whdbg_get_flags()
{
    return whdbg_flags;
}
unsigned int whdbg_set_flags(unsigned int newflags)
{
    unsigned int x = whdbg_flags;
    whdbg_flags = newflags;
    return x;
}

/**
   "Automatic" arguments for whdbgv() and friends. Names of
   elements must match those defined in WHDBG_SOURCE_INFO_PARAMS_DECL.
*/
#define WHDBG_SOURCE_INFO_PARAMS_PASS _file,_line,_func

struct whdbg_PRIVATE_t
{
    char const * filename;
    char const * funcname;
    unsigned int lineno;
    unsigned int condition;
    char const * condString;
};
struct whdbg_PRIVATE_t whdbg_PRIVATE_init = {0,0,0,0,0};
struct whdbg_PRIVATE_t whdbg_PRIVATE = {0,0,0,0,0};


void whdbgv( unsigned int condition,
		WHDBG_SOURCE_INFO_PARAMS_DECL,
		char const * fmt,
		va_list vargs )
{
#if WHDBG_CONFIG_ENABLE
//whdbg_stream&&
    if(((whdbg_flags & condition) || (WHDBG_ALWAYS == whdbg_flags) || (WHDBG_ALWAYS == condition))
	&& fmt )
    {
	long rc = 0;
#define VAPARGS vappendf_FILE_appender, whdbg_stream
	if( whdbg_PRIVATE.condString )
	{
	    rc = printf( "whdbg(%s):%s:%d:%s():\n\t",whdbg_PRIVATE.condString,WHDBG_SOURCE_INFO_PARAMS_PASS);
	    whdbg_PRIVATE.condString = 0;
	}
	else
	{
	    rc = printf( "whdbg(0x%08x):%s:%d:%s():\n\t",condition,WHDBG_SOURCE_INFO_PARAMS_PASS);
	}
	if( rc >= 0 ) rc = vprintf( fmt, vargs );
    printf("\n");
#undef VAPARGS
    }
#endif // WHDBG_CONFIG_ENABLE
}

void whdbg(  unsigned int condition,
		WHDBG_SOURCE_INFO_PARAMS_DECL,
		char const * fmt,
		... )
{
#if WHDBG_CONFIG_ENABLE
    if( whdbg_stream
	&& ((WHDBG_ALWAYS == whdbg_flags) || (whdbg_flags & condition))
	&& fmt )
    {
	va_list vargs;
	va_start( vargs, fmt );
	whdbgv( condition, WHDBG_SOURCE_INFO_PARAMS_PASS, fmt, vargs );
	va_end(vargs);
    }
#endif
}

static void whdbg_PRIVATE_phase2( char const * fmt, ... )
{
#if WHDBG_CONFIG_ENABLE
	va_list vargs;
	va_start( vargs, fmt );
	whdbgv( whdbg_PRIVATE.condition,
		   whdbg_PRIVATE.filename,
		   whdbg_PRIVATE.lineno,
		   whdbg_PRIVATE.funcname,
		   fmt, vargs );
	va_end(vargs);
	whdbg_PRIVATE = whdbg_PRIVATE_init;
#endif
}

whdbg_PRIVATE_f whdbg_PRIVATE_phase1( unsigned int condition,
					    char const * condString,
					    char const * file,
					    unsigned int line,
					    char const *  funcName )
{
#if WHDBG_CONFIG_ENABLE
    whdbg_PRIVATE.condition = condition;
    whdbg_PRIVATE.condString = condString;
    whdbg_PRIVATE.filename = file;
    whdbg_PRIVATE.lineno = line;
    whdbg_PRIVATE.funcname = funcName;
#endif
    return whdbg_PRIVATE_phase2;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
