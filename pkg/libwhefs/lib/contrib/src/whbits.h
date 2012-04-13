#ifndef WANDERINGHORSE_NET_WHBITS_H_INCLUDED
#define WANDERINGHORSE_NET_WHBITS_H_INCLUDED 1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int whbits_count_t;
/**
   A type for managing bitsets.
*/
typedef struct whbits
{
    /** Number of bytes in use by the bitset. */
    whbits_count_t sz_bytes;
    /** Number of bits in the bitset. */
    whbits_count_t sz_bits;
    /**
       Number of bytes allocated.
    */
    whbits_count_t sz_alloc;
    /** the bytes of the bitset. */
    unsigned char * bytes;
} whbits;
/**
   Static initializer for whbits objects.
*/
#define WHBITS_INIT { 0, 0, 0, 0 }
/**
   Empty initializer for whbits objects.
*/
extern const whbits whbits_init_obj;

/**
   Sets up a new bitset array for the given number of bits. Each byte
   of the array will get the value initialState.

   tgt must be a valid object. If it has not previously been used as a
   parameter to this function then it must have been initialized by
   copying the whbits_init_obj over it (or explicitly setting all values
   to zero). Passing in an object with uninitialized values will lead to
   Grief.

   If tgt contains any bytes and the new bitCount is smaller than the old one,
   it re-uses the old memory. If tgt->bytes is not null but is not large
   enough to hold the new bits, it is expanded and only the new bytes
   are set to the initialState.

   On success, 0 is returned and tgt is populated with a bitset
   (potentially allocated using malloc()). The object may be freely
   copied after that, but ONE those copies must eventually be passed
   to whbits_free_bits() to release the memory.
*/
int whbits_init( whbits * tgt, whbits_count_t bitCount, unsigned char initialState );

/**
   Frees any memory associated with b and clears b's state, but does
   not free() b (which may have been stack-allocated).
*/
void whbits_free_bits( whbits * b );

#define WHBITS_BYTEFOR(B,BIT) ((B)->bytes[ BIT / 8 ])
#define WHBITS_SET(B,BIT) ((WHBITS_BYTEFOR(B,BIT) |= (0x01 << (BIT%8))),0x01)
#define WHBITS_UNSET(B,BIT) ((WHBITS_BYTEFOR(B,BIT) &= ~(0x01 << (BIT%8))),0x00)
#define WHBITS_GET(B,BIT) ((WHBITS_BYTEFOR(B,BIT) & (0x01 << (BIT%8))) ? 0x01 : 0x00)
#define WHBITS_TOGGLE(B,BIT) (WHBITS_GET(B,BIT) ? (WHBITS_UNSET(B,BIT)) : (WHBITS_SET(B,BIT)))


/**
   Sets bit number bitNum in b and returns non-zero on success.
*/
char whbits_set( whbits* b, whbits_count_t bitNum );

/**
   Unsets bit number bitNum in b and returns non-zero on success.
*/
char whbits_unset( whbits * b, whbits_count_t bitNum );

/**
   Gets the value of the bit number bitNum in b.
*/
char whbits_get( whbits const * b, whbits_count_t bitNum );


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHBITS_H_INCLUDED */
