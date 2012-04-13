/**
   whefs_string implementations.
*/
#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_string.h>
#include <stdlib.h> // malloc()/free()
#include <string.h> // memset()

const whefs_string whefs_string_empty = whefs_string_empty_m;

whefs_string * whefs_string_alloc()
{
    whefs_string * x = (whefs_string*)malloc(sizeof(whefs_string));
    if( x ) *x = whefs_string_empty;
    return x;
}


int whefs_string_copy_cstring( whefs_string * tgt, char const * str )
{
    if( ! str || !tgt ) return whefs_rc.ArgError;
    size_t slen = 0;
    {
	char const * x = str;
	for( ; x && *x; ++x, ++slen ) {}
    }
    if( ! slen )
    {
        tgt->length = 0;
        if( tgt->string /* && tgt->alloced */)
        {
            //tgt->string[0] = 0;
            memset( tgt->string, 0, tgt->alloced );
        }
        return 0;
    }
    if( tgt->alloced > slen )
    {
	memset( tgt->string + slen, 0, tgt->alloced - slen );
	memcpy( tgt->string, str, slen );
	tgt->length = slen;
	return whefs_rc.OK;
    }
    if( (slen+1) >= UINT16_MAX ) return whefs_rc.RangeError;
    const whefs_string_size_t alen =
        //slen + 1
        slen * 1.5 // FIXME: cap at WHEFS_MAX_FILENAME_LENGTH
        ;
    if( alen < slen )
    { /* overflow! */
	return whefs_rc.RangeError;
    }
    char * xp = (char *) realloc( tgt->string, alen );
    if( ! xp ) return whefs_rc.AllocError;
    tgt->string = xp;
    tgt->alloced = alen;
    tgt->length = slen;
    memset( xp + slen, 0, alen - slen );
    memcpy( tgt->string, str, slen );
    return whefs_rc.OK;
}

int whefs_string_clear( whefs_string * tgt, bool clearChain )
{
    if( ! tgt ) return whefs_rc.ArgError;
    while( tgt )
    {
	whefs_string * n = tgt->next;
        if( tgt->alloced && tgt->string ) free( tgt->string );
	*tgt = whefs_string_empty;
	if( ! clearChain ) break;
	tgt = n;
    }
    return whefs_rc.OK;
}

int whefs_string_finalize( whefs_string * tgt, bool clearChain )
{
    if( ! tgt ) return whefs_rc.ArgError;
    while( tgt )
    {
	whefs_string * n = tgt->next;
	free( tgt->string );
	*tgt = whefs_string_empty;
	free(tgt);
	if( ! clearChain ) break;
	tgt = n;
    }
    return whefs_rc.OK;
}

