#if !defined(WANDERINGHORSE_NET_WHEFS_CACHE_H_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_CACHE_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/

/**
   This file contains declarations for some of the whefs
   private/internal caching API.

*/
#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_string.h>
#include <wh/whio/whio_devs.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
   Generates a hashcode for the given name and identifies the inode
   with the given id with that hashcode. On success it returns whefs_rc.OK.

   Error cases:

   - If fs or name are null, or !*name: whefs_rc.ArgError

   - Allocation of the cache fails: whefs_rc.AllocError

   - An entry already exists for the same hash code: whefs_rc.InternalError. If
   this ever actually happens, i'll try other hashes or build a better hashtable.


   This routine is much faster if the cache is sorted before calling
   this.  If you're going to add lots of items at once, do so with the
   lower-level cache list API, then sort it, then mark is as unsorted (or sort it).
*/
int whefs_inode_hash_cache( whefs_fs * fs, whefs_id_type id, char const * name );

/**
   If a cached entry is found with the same hashcode as name, the id
   of that inode is returned, else 0.
*/
whefs_id_type whefs_inode_hash_cache_search_id(whefs_fs * fs, char const * name );
/**
   If a cached entry is found with the same hashcode as name, the index
   of that entry in the cache is returned, else whefs_rc.IDTypeEnd.
*/
whefs_id_type whefs_inode_hash_cache_search_ndx(whefs_fs * fs, char const * name );

/**
   Searches for a cache entry with the same hash as name. If it finds
   it, it removes it. The cache becomes unsorted by this.
*/
void whefs_inode_name_uncache(whefs_fs * fs, char const * name );

/**
   Iterates over all inodes and caches the name entries for all
   in-used inodes. This is normally automatically called when an FS is
   opened (unless an internal compile-time flag disables that).
*/
int whefs_inode_hash_cache_load( whefs_fs * fs );

/**
   Sorts the name cache, if it is loaded. It shouldn't strictly be
   in the public API, but it may need to be.
*/
void whefs_inode_hash_cache_sort(whefs_fs * fs );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHEFS_CACHE_H_INCLUDED */
