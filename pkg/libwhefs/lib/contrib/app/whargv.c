/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/

#include "whargv.h"
#include <stdlib.h> /* free(), calloc() */
#include <string.h> /* strlen() */
#include <ctype.h> /* isdigit() */

#if 1
#include <stdio.h>
#define MARKER if(1) printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(1) printf
#else
static void bogo_printf(char const * fmt, ...) {}
#define MARKER if(0) bogo_printf
#endif

const whargv_entry whargv_entry_init =
    {
    {0}, /* key */
    0, /* val */
    0, /* is_long */
    0, /* is_nonflag */
    0, /* is_dash */
    0, /* is_numflag */
    0  /* is_help */
    };


whargv_global_t whargv_global = {0,0};


/**
   Cleans up the whargv_global object. It is called
   by whargv_global_parse().
*/
static void whargv_global_atexit()
{
    if(0) whargv_global_atexit(); /* kludge to work around static-not-referenced warning. */
    if( whargv_global.argv )
    {
	free(whargv_global.argv);
	whargv_global.argv = 0;
	whargv_global.argc = 0;
    }
}

int whargv_entry_parse( int argc, char const ** argv, whargv_entry * e )
{
    if( ! argc || !argv || !e ) return -1;
    //MARKER("argc=%d argv[0]=[%s]\n",argc,argv[0]);
    *e = whargv_entry_init;
    char const * x = argv[0];
    const size_t slen = (x && x[0]) ? strlen( x ) : 0;
    if( ! slen ) return 0;
    if( '-' != x[0] )
    { /* non-flag argument */
	e->val = x;
	e->is_nonflag = 1;
	return 1;
    }
    if( slen < 2 )
    {
	e->is_dash = 1;
	return 1; /* a single, lonely dash */
    }
    int rc = 1;
    char const * keypos = x + 1;
    if( ! *keypos ) return 0;
    if( '-' == *keypos )
    { /* flag in the form --flag */
	++keypos;
	e->is_long = 1;
	if( 2 == slen )
	{ /* the infamous double-dash */
	    e->is_dash = 2;
	    return 1;
	}
    }
    if( ! e->is_long )
    { /* short arg in the form -A[=][VALUE] */
	e->key[0] = *(keypos++);
	e->key[1] = 0;
	if( *keypos == '=' )
	{
	    ++keypos;
	}
	e->val = *keypos ? keypos : 0;
	//MARKER("short key=%c val=[%s]\n",e->key[0], e->val );
	goto whargv_entry_parse_check_num;
    }
    /* Long arg in the form --key[=[VALUE]] */
    size_t i = 0;
    for( ; keypos && *keypos
	     && (i < whargv_entry_max_key_len);
	 ++keypos, ++i )
    {
	if( *keypos == '=' )
	{
	    e->val = *(keypos+1) ? (keypos+1) : 0;
	    break;
	}
	e->key[i] = *keypos;
    }
    e->key[i] = 0;
    whargv_entry_parse_check_num:
    if( e->is_long && *e->key )
    { /* check for --[+-]INTEGER flag */
	x = e->key;
	char sign = 0;
	if( x && ((*x == '-') || (*x == '+')) )
	{
	    sign = *x;
	    ++x;
	}
	size_t got = 0;
	for( ; x && isdigit(*x); ++x, ++got )
	{
	}
	if( got )
	{
	    e->is_numflag = -1;
	    e->val = e->key;
	    if( '+' == sign )
	    { /* kludge to strip plus sign. */
		++e->val;
		e->is_numflag = 1;
	    }
	}
    }
    if( *e->key )
    {
	if( ( 0 == strcmp( e->key, "help" ) )
	    || (0 == strcmp( e->key, "?" ))
	    )
	{
	    e->is_help = 1;
	}
    }
    return rc;
}

size_t whargv_global_parse( int argc, char const ** argv )
{
    if( ! whargv_global.argv )
    {
	atexit( whargv_global_atexit );
    }
    else
    { /* clean up! */
	whargv_global_atexit();
    }
    whargv_global.argc = 0;
    whargv_global.argv = (whargv_entry *)calloc( argc+1, sizeof(whargv_entry) );
    if( ! whargv_global.argv ) return 1;
    int i = 0;
    int arc = 1;
    for( ; (i < argc) && (arc>0); i += arc, ++whargv_global.argc )
    {
	arc = whargv_entry_parse( argc-i, argv + i, &whargv_global.argv[i] );
	if( arc < 1 ) break;
    }
    return whargv_global.argc;
}

