#ifndef WANDERINGHORSE_NET_WHEFS_HASH_H_INCLUDED
#define WANDERINGHORSE_NET_WHEFS_HASH_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain


  Hashing-related types and routines for whefs.
*/

#include <wh/whefs/whefs_config.h> // whefs_id_type, stdint types

#ifdef __cplusplus
extern "C" {
#endif

/**
   The integral type whefs uses to store hash values.

   Reminder to self: if we'll add support for duplicate hash codes
   to the inode names hash cache then we can drop this to
   uint16_t and potentially save significant amounts of memory.
   The difference is: with uint32_t we can cache 128 inode hashes
   per 1k of memory. With uint16_t we can cache 170 hashes per
   1k of memory.
*/
typedef uint32_t whefs_hashval_type;
/**
   printf format specifier for use with whefs_hashval_type.
*/
#define WHEFS_HASHVAL_TYPE_PFMT "08"PRIxLEAST32
/**
   scanf format specifier for use with whefs_hashval_type.
*/
#define WHEFS_HASHVAL_TYPE_SFMT SCNu32

/**
   A container for mapping abstract hash values to abstract
   whefs-related object IDs. The primary intention is a name-hash
   cache for file name lookups, but in theory it can be used with any
   whefs types which have a unique ID of whefs_id_type type.
*/
struct whefs_hashid
{
    /** Client-defined hash code. */
    whefs_hashval_type hash;

    /** Unique ID of client-defined hashed object. */
    whefs_id_type id;

    /**
       Routines which visit this object via search results should increment this number.
       We may use it later for dropping least-used items from the list.

       Changing this to uint32_t actually costs 6 bytes on my box, as the sizeof()
       comes out to 12 (if whefs_id_type is uint16_t as well). With uint16_t i
       get a sizeof(8).

       Note that we can easily overflow with a 16-bit counter, but for
       the purposes we're using this number for that won't hurt us (it
       may cause the entry to get dropped if we do LRU shaving, but
       that's it). The extra bytes for uint32_t aren't worth it here.
    */
    uint16_t hits;
};
typedef struct whefs_hashid whefs_hashid;
/**
   Empty initializer object for whefs_hashid.
*/
#define whefs_hashid_empty_m {0/*hash*/,0/*id*/,0/*hits*/}
/**
   Empty initializer object for whefs_hashid.
*/
extern const whefs_hashid whefs_hashid_empty;


/** @struct whefs_hashid_list

   Holds a list of whefs_hashid objects. It is a special-purpose
   hashtable with a focus on making as few calls as necessary
   to malloc() while still keeping memory costs reasonable.
   It is not a full-fledged hashtable - all items are stored in
   a contiguous array. Thus inserting, removing, etc., can be
   expensive, except for appending to the end, which is O(1)
   as long as the list has fewer items in use than are actually
   allocated.

   Currently this is used to map inode name hashes to inode IDs
   to speed up lookups by name.
 */
struct whefs_hashid_list
{
    /** Number of items allocated. */
    whefs_id_type alloced;
    /** Real number of entries. */
    whefs_id_type count;
    /** A hint to the allocator to say we'll never need more than this
        many entries. If set to 0 it is ignored, which may cause some
        over-allocation in the worst case.
     */
    whefs_id_type maxAlloc;
    /** The list of items. */
    whefs_hashid * list;
    /** Functions which unsort a list set this to false. whefs_hashid_list_sort() sets it to true. */
    bool isSorted;
    /** Internal optimization hack to avoid auto-sorting when we're inserting many items in a loop. */
    bool skipAutoSort;
};
typedef struct whefs_hashid_list whefs_hashid_list;
/** Empty initializer object. */
#define whefs_hashid_list_empty_m {0U/*alloced*/,0U/*count*/,0U/*maxAlloc*/,0/*list*/,false/*isSorted*/,false/*skipAutoSort*/}
/** Empty initializer object. */
extern const whefs_hashid_list whefs_hashid_list_empty;

/**
   Sorts the entries in li by hash value. It is not specified whether
   the sort is stable in relation to items with the same hash values.

   In this implementation, sorting may actually change the size of the
   list. whefs_hashid_list_wipe_index() only marks data for erasure. Any items
   marked as such will be filtered out of the list as part of the sorting
   process.

   Returns whefs_rc.OK on success. The only error condition is (!li),
   then it returns whefs_rc.ArgError.
*/
int whefs_hashid_list_sort( whefs_hashid_list * li );

/**
   Allocates the specified number of entries in a whefs_hashid_list.

   If *tgt is 0 then this function allocates a new instance and assigns
   tgt to it, otherwise it assumes *tgt is a valid object and it re-uses any
   parts of that object which it can. It reserves toAlloc places in (*tgt)->list.

   On success, whefs_rc.OK is returned and tgt will contain at least
   the given number of entries, else some other value is returned.
   On success, tgt will contain at least toAlloc entries, but the exact
   number is unspecified unless toAlloc is 0, in which case all entries
   will be freed but tgt will still be valid (but tgt->list will be 0).
   Newly-created entries will be initialized to empty values.
 
   tgt may not be 0, but *tgt may be 0. If *tgt is not 0 then it must point
   to a valid object (specifically, it may not be an uninitialized object).
 
   The tgt object must eventually be freed using whefs_hashid_list_free().

   If (*tgt)->count is less than toAlloc then entries will be lopped
   off and (*tgt)->count will be set to the value of toAlloc.
   Otherwise (*tgt)->count will not be modified (or will be 0 if this
   function allocates the object).

   If (*tgt)->alloced is greater than or equal to toAlloc then this
   function has no side-effects and returns success.
*/
int whefs_hashid_list_alloc( whefs_hashid_list ** tgt, whefs_id_type toAlloc );

/**
   Frees a list created by whefs_hashid_list_alloc(), including all of
   its entries. After calling this, tgt is an invalid object.
*/
void whefs_hashid_list_free( whefs_hashid_list * tgt );

/**
   Appends a copy of val to the end of tgt, expanding tgt if necessary. On success
   whefs_rc.OK is returned, otherwise:

   - whefs_rc.AllocError if allocation fails.
   - whefs_rc.ArgError if !tgt or !val.

   If you have removed items from the list, it is more memory-efficient to
   sort it before adding any new items. This will cost a sort but avoid
   a re-alloc if at least as many items are removed as were added.

   The current usage of the whefs_hashid_list class does not account
   for duplicate hash keys, so ensure that you don't add dupes.
*/
int whefs_hashid_list_add( whefs_hashid_list * tgt, whefs_hashid const * restrict val );

/**
   The item at the given index is zeroed out, which is the internal
   equivalent of erasure. During the next sort, any zeroed items
   will be removed from the list. This function, to simplify the
   implementation and speed up certain fs operations, does not
   actually modify the structure of the list or change li->count. The
   change in size is deferred until the next sort. When that happens,
   the zeroed items get sorted to the start of the list, then get
   pruned.

   A side effect of the mark-for-removal approach is that this routine
   does not invalidate any other indexes by shifting items around.

   On success whefs_rc.OK is returned. On error one of the following is returned:

   If !lf then whefs_rc.ArgError is returned. If ndx is not less than li->count,
   whefs_rc.RangeError is returned.
*/
int whefs_hashid_list_wipe_index( whefs_hashid_list * li, whefs_id_type ndx );

/**
   Searches src for an item matching the given hash value. On success
   it returns the index of the left-most item with that hash value. On
   error, or no result found, it returns whefs_rc.IDTypeEnd.

   Search results are undefined if src is not sorted by ascending hash
   value.

   Technically speaking, the search algorithm used by this function is
   unspecified, but the current implementation uses two different
   approaches to accomodate usage patterns in whefs_fs::cache.nameHashes.

   IFF src->isSorted then a binary search is used, with the addition
   that it returns the left-most match if the hash key has
   duplicates. The left-most behaviour is to allow the client to check
   if each matching item *really* matches the object from which the
   given hash was derived. e.g. if two different strings have a hash
   collision, we need to compare the input string with the string
   referred to by src->list[search_result_index].id in order to ensure
   that a match has been found. If there is a mismatch we could then
   check succeeding items in the list until we hit the end of the list
   or an item with a differing hash value.

   IFF !src->isSorted then we fall back to a linear search. The alternative
   would be to sort the list here automatically if needed, but that could
   invalidate data held by loops if this was called in a loop.
*/
whefs_id_type whefs_hashid_list_index_of( whefs_hashid_list const * src, whefs_hashval_type hash );

/**
   Inserts count empty slots in li before pos, expanding the list if
   necessary.  Existing entries starting at pos are shifted count
   items to the right.

   If !li then whefs_rc.ArgError is returned. If pos is greater than
   li->count, or if !li->count, then whefs_rc.RangeError is returned.
   On success, whefs_rc.OK is returned and li is updated accordingly.
   The caller is responsible for populating the new entries and
   re-sorting the list.

*/
int whefs_hashid_list_add_slots( whefs_hashid_list * li, whefs_id_type pos, whefs_id_type count );

/**
   Generates a hash code for the given null-terminated string.
   If str is null then 0 is returned. The exact hash routine
   is not specified.
*/
whefs_hashval_type whefs_hash_cstring( char const * str );

/**
   Returns the amount of memory allocated to li, including all
   items.
*/
size_t whefs_hashid_list_sizeof( whefs_hashid_list const * li );


/**
   Removes the least-visited items from li using the simple heuristic
   of sorting by visit count, lopping off the bottom half, and
   re-sorting.  li need not be sorted before calling this. The
   returned list will be sorted and will likely have fewer items in
   it. Performance is effectively O(N), where N = li->count, plus the
   cost the of underlying (unspecified) sort.
*/
int whefs_hashid_list_chomp_lv( whefs_hashid_list * li );

#if 0

/**
   UNTESTED/EXPERIMENTAL!
*/
typedef int (*whefs_hashid_list_search_cmp)( whefs_hashval_type, whefs_id_type id, void const * );
/**
   UNTESTED/EXPERIMENTAL!

   Searches for an item, as for whefs_hashid_list_index_of(). The hash
   argument is the hash to search for. If a match is found, it calls
   cmp(hash,search_result.id,cmdData). If that function returns 0,
   search_result.id is returned. If cmp returns non-0 then the next
   item in the list with the same hash key is checked. If it has the same
   hashcode, cmp() is called. This process is repeated until the list
   is exhausted or it encounters an item where (hash!=item.hash).

   If no item is found, whefs_rc.IDTypeEnd is returned.
*/
whefs_id_type whefs_hashid_list_search( whefs_hashid_list const * src,
                                        whefs_hashval_type hash,
                                        whefs_hashid_list_search_cmp cmp,
                                        void const * cmpData
                                        );
#endif

#ifdef __cplusplus
} /* extern "C"*/
#endif

#endif // WANDERINGHORSE_NET_WHEFS_HASH_H_INCLUDED
