#include "whbits.h"
#include <stddef.h> /* size_t on my box */
#include <string.h> /* memset() */
#include <stdlib.h> /* malloc()/free() */

const whbits whbits_init_obj = WHBITS_INIT;

int whbits_init( whbits * b, whbits_count_t bitCount, unsigned char initialState )
{
    if( ! bitCount || !b ) return -1;
    const size_t lenB = (bitCount / 8) + ((bitCount%8) ? 1 : 0);
    if( b->sz_alloc >= lenB )
    { /* re-use the memory */
        //memset( b->bytes+b->sz_bytes, initialState, b->sz_bytes - lenB );
        if( b->sz_bytes == lenB )
        {
            b->sz_bits = bitCount;
            return 0;
        }
        else if( b->sz_bytes < lenB )
        {
            memset( b->bytes+b->sz_bytes, initialState, b->sz_alloc - b->sz_bytes );
            b->sz_bytes = lenB;
        }
        else
        {
            memset( b->bytes+lenB, initialState, b->sz_alloc - lenB );
            b->sz_bytes = lenB;
        }
        b->sz_bits = bitCount;
        return 0;
    }
    whbits old = *b;
    unsigned char * x = realloc( b->bytes, lenB );
    if( ! x )
    {
        return -1;
    }
    b->bytes = x;
    b->sz_alloc = lenB;
    b->sz_bytes = lenB;
    b->sz_bits = bitCount;
    memset( b->bytes + old.sz_alloc, initialState, b->sz_alloc - old.sz_alloc );
    return 0;
}

void whbits_free_bits( whbits * b )
{
    if( b )
    {
	if( b->bytes )
	{
	    free(b->bytes);
	}
	*b = whbits_init_obj;
    }
}

size_t whbits_size_bits( whbits const * b )
{
    return b ? b->sz_bits : 0;
}

size_t whbits_size_bytes( whbits const * b )
{
    return b ? b->sz_bytes : 0;
}

char whbits_set( whbits* b, whbits_count_t bitNum )
{
    if( ! b || (bitNum  > b->sz_bits) ) return 0;
    WHBITS_SET( b, bitNum );
    return 1;
}

char whbits_unset( whbits * b, whbits_count_t bitNum )
{
    if( ! b || (bitNum  > b->sz_bits) ) return 0;
    WHBITS_UNSET( b, bitNum);
    return 1;
}

char whbits_toggle( whbits const * b, whbits_count_t bitNum )
{
    if( ! b || (bitNum  > b->sz_bits) ) return -1;
    return (char) WHBITS_TOGGLE(b,bitNum);
}

char whbits_get( whbits const * b, whbits_count_t bitNum )
{
    if( ! b || (bitNum  > b->sz_bits) ) return 0;
    return WHBITS_GET(b,bitNum);
}
