/**
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain

  This file contiains most of the name-cache-related functionality.
*/

#include "whefs_details.c"
#include "whefs_cache.h"
#include <stdlib.h> // free()
#include <string.h> // memset()
#include <assert.h>

int whefs_inode_hash_cache_load( whefs_fs * fs )
{
    if( ! WHEFS_FS_HASH_CACHE_IS_ENABLED(fs) ) return whefs_rc.OK;
    if( ! fs->cache.hashes )
    {
        whefs_id_type toAlloc = 16 /* arbitrary */;
        if( toAlloc > fs->options.inode_count ) toAlloc = fs->options.inode_count;
        whefs_hashid_list_alloc( &fs->cache.hashes, toAlloc );
    }
    if( ! fs->cache.hashes ) return whefs_rc.AllocError;
    else if( !fs->cache.hashes->maxAlloc )
    {
        fs->cache.hashes->maxAlloc = fs->options.inode_count;
    }

    //whefs_hashid h = whefs_hashid_empty;
    whefs_string name = whefs_string_empty;
    enum { bufSize = WHEFS_MAX_FILENAME_LENGTH+1 };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );
    // ensure that whefs_inode_name_get() won't malloc():
    name.string = (char *)buf;
    name.alloced = bufSize;
    name.length = 0;
    int rc = 0;
    whefs_id_type i;
    whefs_id_type count = 0;
    fs->cache.hashes->skipAutoSort = true; // optimization to avoid extra sorting via whefs_inode_name_get()
    for( i = fs->options.inode_count; i >=1 ; --i )
    {
        /**
           Maintenance reminder:

           We do this loop in reverse order as an efficiency hack for
           whefs_string_cache. The trick is: the whefs_string_cache's
           internal buffer grows only as the number of used inodes
           does (it's size is a function of the highest used inode
           ID). If we insert from low to high it may realloc many
           times. If we insert from high to low, we're guaranteed to
           need only one malloc/realloc on it.
        */
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
        if( fs->bits.i_loaded )
	{
            if( ! WHEFS_ICACHE_IS_USED(fs,i) )
            {
                continue;
            }
	}
#endif // WHEFS_CONFIG_ENABLE_BITSET_CACHE
        rc = whefs_inode_name_get( fs, i, &name ); // this caches the name hash
        assert( (name.string == (char *)buf) && "Internal memory management foo-foo." );
        if( whefs_rc.OK != rc ) break;
        ++count;
    }
    fs->cache.hashes->skipAutoSort = false;
    whefs_hashid_list_sort(fs->cache.hashes);
    WHEFS_DBG_CACHE("Loaded all used inodes into cache with %"WHEFS_ID_TYPE_PFMT" name(s).",count);
    return rc;
}
int whefs_inode_hash_cache_chomp_lv( whefs_fs * fs )
{
    if( ! fs ) return whefs_rc.ArgError;
    if( ! fs->cache.hashes || !fs->cache.hashes->count ) return whefs_rc.OK;
    return whefs_hashid_list_chomp_lv( fs->cache.hashes );
}

whefs_id_type whefs_inode_hash_cache_search_ndx(whefs_fs * fs, char const * name )
{
    if( ! WHEFS_FS_HASH_CACHE_IS_ENABLED(fs) ) return whefs_rc.IDTypeEnd;
    if( fs->cache.hashes && !fs->cache.hashes->isSorted )
    {
        WHEFS_DBG_CACHE("Achtung: auto-sorting dirty name cache before search starts.");
        whefs_inode_hash_cache_sort(fs);
    }
#if 0
    return ( ! fs->cache.hashes )
        ? whefs_rc.IDTypeEnd
        : whefs_hashid_list_index_of( fs->cache.hashes, fs->cache.hashfunc(name) );
#else
    if( ! fs->cache.hashes )
    {
        WHEFS_DBG_CACHE("Cache check failed (cache is empty) for name [%s].",name);
        return whefs_rc.IDTypeEnd;
    }
    else
    {
        whefs_id_type const rc = whefs_hashid_list_index_of( fs->cache.hashes, fs->cache.hashfunc(name) );
        WHEFS_DBG_CACHE("Cache %s for name [%s].",((rc==whefs_rc.IDTypeEnd) ? "miss" : "hit"), name);
        return rc;
    }
#endif    
}

whefs_id_type whefs_inode_hash_cache_search_id(whefs_fs * fs, char const * name )
{
    if( ! WHEFS_FS_HASH_CACHE_IS_ENABLED(fs) ) return 0;
    if( ! fs->cache.hashes ) return 0;
    whefs_id_type n = whefs_inode_hash_cache_search_ndx( fs, name );
    WHEFS_DBG_CACHE("Cache %s for name [%s].",((n==whefs_rc.IDTypeEnd) ? "miss" : "hit"), name);
    return ( n == whefs_rc.IDTypeEnd )
        ? 0
        : fs->cache.hashes->list[n].id;
}



void whefs_inode_hash_cache_sort(whefs_fs * fs )
{
    if( fs->cache.hashes && ! fs->cache.hashes->isSorted )
    {
        whefs_hashid_list_sort( fs->cache.hashes );
    }
}


void whefs_inode_name_uncache(whefs_fs * fs, char const * name )
{
    if( !fs->cache.hashes || ! name || !*name  ) return;
    const whefs_id_type ndx = whefs_inode_hash_cache_search_ndx( fs, name );
    if( whefs_rc.IDTypeEnd != ndx )
    {
        fs->cache.hashes->list[ndx] = whefs_hashid_empty;
        fs->cache.hashes->isSorted = false;
    }
}

int whefs_inode_hash_cache( whefs_fs * fs, whefs_id_type id, char const * name )
{
    if( ! WHEFS_FS_HASH_CACHE_IS_ENABLED(fs) )
    {
        return whefs_rc.OK;
    }
    if( ! fs || !name || !*name ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    if( ! fs->cache.hashes )
    {
        const whefs_id_type max = fs->options.inode_count;
        whefs_id_type dflt = 100/sizeof(whefs_hashid) /*for lack of a better default value.*/;
        if( dflt > max ) dflt = max;
        rc = whefs_hashid_list_alloc( &fs->cache.hashes, dflt );
        if( fs->cache.hashes )
        {
            fs->cache.hashes->maxAlloc = max;
        }
    }
    if( whefs_rc.OK != rc ) return rc;
    whefs_hashval_type h = fs->cache.hashfunc( name );
#if 1
    const whefs_id_type ndx = whefs_hashid_list_index_of( fs->cache.hashes, h );
    if( whefs_rc.IDTypeEnd != ndx )
    {
        if(0) WHEFS_DBG("CHECKING: name cache count[%"WHEFS_ID_TYPE_PFMT"], alloced=[%"WHEFS_ID_TYPE_PFMT"], hash [%"WHEFS_HASHVAL_TYPE_PFMT"] for name [%s], ndx=[%"WHEFS_ID_TYPE_PFMT"]",
                        fs->cache.hashes->count, fs->cache.hashes->alloced, h, name, ndx );
        if( fs->cache.hashes->list[ndx].id == id ) return whefs_rc.OK;
        WHEFS_DBG_ERR("ERROR: name cache hash collision on hash code "
                      "%"WHEFS_HASHVAL_TYPE_PFMT" "
                      "between inodes #"
                      "%"WHEFS_ID_TYPE_PFMT
                      " and #%"WHEFS_ID_TYPE_PFMT"!",
                      h, id, ndx );
        return whefs_rc.InternalError;
    }
    WHEFS_DBG_CACHE("ADDING: name cache count[%"WHEFS_ID_TYPE_PFMT"], alloced=[%"WHEFS_ID_TYPE_PFMT"], hash [%"WHEFS_HASHVAL_TYPE_PFMT"] for name [%s]",
                          fs->cache.hashes->count, fs->cache.hashes->alloced, h, name );
#endif
    whefs_hashid H = whefs_hashid_empty;
    H.hash = h;
    H.id = id;
    rc = whefs_hashid_list_add( fs->cache.hashes, &H );
    WHEFS_DBG_CACHE("Added to name cache: hash[%"WHEFS_HASHVAL_TYPE_PFMT"]=id[%"WHEFS_ID_TYPE_PFMT"], name=[%s], rc=%d", H.hash, H.id, name, rc );
    return whefs_rc.OK;
}


