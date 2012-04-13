/*
  Implementations for whefs_inode operations.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#include <string.h>
#include <stdlib.h> /* realloc() and friends. */
#include "whefs_details.c"
#include "whefs_cache.h"

//#include "whio_dev.h" /* for whio_dev_sizeof_uint32 */

#include <time.h> /* gettimeofday() */
#include <sys/time.h>

const whefs_inode whefs_inode_empty = whefs_inode_empty_m;

const whefs_inode_list whefs_inode_list_empty = whefs_inode_list_empty_m;


/**
   If WHEFS_CONFIG_ENABLE_STATIC_MALLOC is true then we statically allocate
   whefs_inode_list_alloc_count whefs_inode_list objects to dole out via
   whefs_inode_list_alloc(), falling back to malloc() if the list is full.
*/
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whefs_inode_list_alloc_slots object.
*/
whefs_inode_list_alloc_count = 10
};
static struct
{
    whefs_inode_list objs[whefs_inode_list_alloc_count];
    char used[whefs_inode_list_alloc_count];
    size_t next;
} whefs_inode_list_alloc_slots = { {whefs_inode_list_empty_m}, {0}, 0 };
#endif

static whefs_inode_list * whefs_inode_list_alloc()
{
    whefs_inode_list * obj = 0;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = whefs_inode_list_alloc_slots.next;
    for( ; i < whefs_inode_list_alloc_count; ++i )
    {
	if( whefs_inode_list_alloc_slots.used[i] ) continue;
	whefs_inode_list_alloc_slots.next = i+1;
	whefs_inode_list_alloc_slots.used[i] = 1;
	obj = &whefs_inode_list_alloc_slots.objs[i];
	break;
    }
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whefs_inode_list *) malloc( sizeof(whefs_inode_list) );
    if( obj ) *obj = whefs_inode_list_empty;
    return obj;
}

static void whefs_inode_list_free( whefs_inode_list * obj )
{
    if( obj ) *obj = whefs_inode_list_empty;
    else return;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whefs_inode_list_alloc_slots.objs[0]) ||
	(obj > &whefs_inode_list_alloc_slots.objs[whefs_inode_list_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whefs_inode_list_alloc_slots.objs[0]);
	whefs_inode_list_alloc_slots.used[ndx] = 0;
	if( ndx < whefs_inode_list_alloc_slots.next ) whefs_inode_list_alloc_slots.next = ndx;
	return;
    }
#else
    free(obj);
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   This updates the internal used-inodes cache (if enabled) and inode
   search hints. ino must be a valid inode for fs.
*/
static void whefs_inode_update_used( whefs_fs * fs, whefs_inode const * ino )
{
    if( ! whefs_inode_is_valid( fs, ino) ) return;
    else if( ino->flags & WHEFS_FLAG_Used )
    {
	WHEFS_ICACHE_SET_USED(fs,ino->id);
    }
    else
    {
	WHEFS_ICACHE_UNSET_USED(fs,ino->id);
        if( fs->hints.unused_inode_start > ino->id )
        {
            fs->hints.unused_inode_start = ino->id;
        }
    }
}


int whefs_inode_name_set( whefs_fs * fs, whefs_id_type nid, char const * name )
{
    //if( !n || !n->id ) return whefs_rc.ArgError;
    if( !whefs_inode_id_is_valid(fs,nid) )
    {
        return whefs_rc.ArgError;
    }
    int rc = 0;

    /**
       We have to see if we have an existing entry for the given inode ID, so
       we can replace its hashvalue in the cache. If we don't do this we end
       up with stale/useless entries in the cache.
    */
    whefs_hashid * H = 0;
    char const * nameCheck = name;
    enum { bufSize = WHEFS_MAX_FILENAME_LENGTH + 1 };
    char buf[bufSize] = {0};
    whefs_string ncheck = whefs_string_empty;
    ncheck.string = buf;
    ncheck.alloced = bufSize;
    if(1)
    {
        rc = whefs_inode_name_get( fs, nid, &ncheck );
        assert( (ncheck.string == buf) && "illegal (re)alloc!");
        if( whefs_rc.OK != rc ) return rc;
        if( *buf && (0==strcmp(buf,name))) return whefs_rc.OK;
        if( *buf ) nameCheck = ncheck.string;
    }
    WHEFS_DBG_CACHE("inode-name-set check for collision in [old=[%s]][new=[%s]][checkAgainst=[%s]].",ncheck.string,name,nameCheck);
    whefs_id_type ndx = whefs_inode_hash_cache_search_ndx( fs, nameCheck );
    if( ndx != whefs_rc.IDTypeEnd )
    {
        WHEFS_DBG_CACHE("inode-name-set found an existing entry for [%s].",nameCheck);
        H = &fs->cache.hashes->list[ndx];
        if( H->id != nid )
        {
            WHEFS_DBG_ERR("Internal error: cache hash collision for name [%s]!",name);
            return whefs_rc.InternalError;
        }
    }
    /**
       Maintenance reminders:

       We write to disk before updating any opened inode because
       writing is much more likely to fail than updating the opened
       inode is, since the latter operation is either just a string
       copy and possibly a relatively small malloc for the name
       strings cache. So we do the ops in order of likely failure, to
       reduce the possibility of, e.g. on-disk and in-memory inode
       names not matching.
    */
    rc = whefs_fs_name_write( fs, nid, name );
    if( whefs_rc.OK != rc ) return rc;
#if 1
    if( H )
    {
        H->hash = fs->cache.hashfunc(name);
        //fs->cache.hashes->isSorted = false;
        whefs_hashid_list_sort( fs->cache.hashes );
        WHEFS_DBG_CACHE("Replacing hashcode for file [%s].",name);
    }
#endif
    return rc;
}


#if ! WHEFS_MACROIZE_SMALL_CHECKS
bool whefs_inode_id_is_valid( whefs_fs const * restrict fs, whefs_id_type nid )
{
    return whefs_inode_id_is_valid_m(fs,nid);
}
#endif

#if ! WHEFS_MACROIZE_SMALL_CHECKS
bool whefs_inode_is_valid( whefs_fs const * restrict fs, whefs_inode const * n )
{
    return n ? whefs_inode_id_is_valid( fs, n->id ) : false;
}
#endif

whio_size_t whefs_inode_id_pos( whefs_fs const * restrict fs, whefs_id_type nid )
{
    if( ! whefs_inode_id_is_valid( fs, nid ) )
    {
	return 0;
    }
    else
    {
	return fs->offsets[WHEFS_OFF_INODES_NO_STR]
	    + ( (nid - 1) * fs->sizes[WHEFS_SZ_INODE_NO_STR] );
    }
}

int whefs_inode_id_seek( whefs_fs * fs, whefs_id_type id )
{
    whio_size_t p = whefs_inode_id_pos( fs, id );
    if( ! p ) return whefs_rc.ArgError;
    whio_size_t sk = whefs_fs_seek( fs, p, SEEK_SET );
    return (p == sk) ? whefs_rc.OK : whefs_rc.IOError;
}

int whefs_inode_seek( whefs_fs * fs, whefs_inode const * n )
{
    return n ? whefs_inode_id_seek( fs, n->id ) : 0;
}

/**
   On-disk inodes are prefixed with this character as a
   consistency-checking measure.
*/
static const unsigned char whefs_inode_tag_char = 'I';//0xef /* small i with diaeresis */;

int whefs_inode_flush( whefs_fs * fs, whefs_inode const * n )
{
    if( ! whefs_inode_is_valid( fs, n ) ) return whefs_rc.ArgError;
    if( ! whefs_fs_is_rw(fs) ) return whefs_rc.AccessError;
    if(0) WHEFS_DBG_FYI("Flushing inode #%"WHEFS_ID_TYPE_PFMT". inode->data_size=%u",
			n->id, n->data_size );
    whefs_inode_update_used( fs, n );
    //WHEFS_DBG("Writing node #%"WHEFS_ID_TYPE_PFMT" at offset %u", n->id, pos );
    enum { bufSize = whefs_sizeof_encoded_inode };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );
    whefs_inode_encode( n, buf );
#if 0
    return whio_blockdev_write( &fs->fences.i, n->id - 1, buf );
#else
    int rc = whefs_inode_id_seek( fs, n->id );
    if( whefs_rc.OK != rc ) return rc;
    whio_size_t const wsz = whefs_fs_write( fs, buf, bufSize );
    return (wsz == bufSize) ? whefs_rc.OK : whefs_rc.IOError;
#endif
}

int whefs_inode_id_read( whefs_fs * fs, whefs_id_type nid, whefs_inode * tgt )
{
    if( !tgt || !whefs_inode_id_is_valid( fs, nid ) ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    enum { bufSize = whefs_sizeof_encoded_inode };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );
#if 0
    rc = whio_blockdev_read( &fs->fences.i, nid - 1, buf );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Error #%d while reading inode #%"WHEFS_ID_TYPE_PFMT"!",
		      rc, nid );
	return rc;
    }
#else
    rc = whefs_inode_id_seek( fs, nid );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Error #%d while seeking to disk pos for inode #%"WHEFS_ID_TYPE_PFMT"!",
		      rc, nid );
	return rc;
    }
    whio_size_t const rsz = whefs_fs_read( fs, buf, bufSize );
    if( rsz != bufSize )
    {
	WHEFS_DBG_ERR("Error reading %u bytes for inode #%"WHEFS_ID_TYPE_PFMT". Only got %"WHIO_SIZE_T_PFMT" bytes!",
		      bufSize, nid, rsz );
	return rc;
    }
#endif
    rc = whefs_inode_decode( tgt, buf );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Error #%d while decoding inode #%"WHEFS_ID_TYPE_PFMT"!",
		      rc, nid );
	return rc;
    }
    whefs_inode_update_used( fs, tgt );
    return rc;
}

int whefs_inode_read_flags( whefs_fs * fs, whefs_id_type nid, uint32_t * flags )
{
    if( ! whefs_inode_id_is_valid( fs, nid ) || !fs->dev ) return whefs_rc.ArgError;
    whefs_inode ino = whefs_inode_empty;
    int rc = whefs_inode_id_read( fs, nid, &ino );
    if( whefs_rc.OK == rc )
    {
        if( flags ) *flags = ino.flags;
    }
    else
    {
	WHEFS_DBG_ERR("Error code #%d while reading inode #%"WHEFS_ID_TYPE_PFMT"!",
		      rc, nid );
        (void)0;
    }
    return rc;
}

int whefs_inode_foreach( whefs_fs * fs, whefs_inode_predicate_f where, void * whereData,
                         whefs_inode_foreach_f func, void * foreachData )
{
    if( ! fs || !func ) return whefs_rc.ArgError;
    whefs_id_type i = 2;// skip root inode
    whefs_inode n = whefs_inode_empty;
    int rc = whefs_rc.OK;
    for( ; i <= fs->options.inode_count; ++i )
    {
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
	if( fs->bits.i_loaded && !WHEFS_ICACHE_IS_USED(fs,i) )
	{
	    continue;
	}
#endif
	rc = whefs_inode_id_read( fs, i, &n );
	if( whefs_rc.OK != rc ) return rc;
	if( n.id != i )
	{
	    assert( 0 && "node id mismatch after whefs_inode_id_read()" );
	    WHEFS_FIXME("Node id mismatch after successful whefs_inode_id_read(). Expected %"WHEFS_ID_TYPE_PFMT" but got %"WHEFS_ID_TYPE_PFMT".", i, n.id );
	    return whefs_rc.InternalError;
	}
        if( where && ! where( fs, &n, whereData ) ) continue;
        rc = func( fs, &n, foreachData );
        if( whefs_rc.OK != rc ) break;
    }
    return rc;
}
int whefs_inode_next_free( whefs_fs * restrict fs, whefs_inode * restrict tgt, bool markUsed )
{
    if( ! fs || !tgt ) return whefs_rc.ArgError;
    whefs_id_type i = fs->hints.unused_inode_start;
    if( i < 2 )
    {
	i = fs->hints.unused_inode_start = 2;
	/* we skip the root node, which is reserved at ID 1. */
    }
    whefs_inode n = whefs_inode_empty;
    if(0) WHEFS_DBG("i=%"WHEFS_ID_TYPE_PFMT", fs->hints.unused_inode_start=%"WHEFS_ID_TYPE_PFMT
		    ", fs->options.inode_count=%"WHEFS_ID_TYPE_PFMT,
		    i, fs->hints.unused_inode_start, fs->options.inode_count );
    for( ; i <= fs->options.inode_count; ++i )
    {
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
	if( fs->bits.i_loaded && WHEFS_ICACHE_IS_USED(fs,i) )
	{
	    //WHEFS_DBG("Got cached inode USED entry for inode #%"WHEFS_ID_TYPE_PFMT"", i );
	    continue;
	}
	//WHEFS_DBG("Cache says inode #%i is unused.", i );
#endif
	int rc = whefs_inode_id_read( fs, i, &n );
	//WHEFS_DBG("Checking inode #%"WHEFS_ID_TYPE_PFMT" for freeness. Read rc=%d",i,rc);
	if( whefs_rc.OK != rc )
	{
	    return rc;
	}
	if( n.id != i )
	{
	    assert( 0 && "node id mismatch after whefs_inode_id_read()" );
	    WHEFS_FIXME("Node id mismatch after successful whefs_inode_id_read(). Expected %"WHEFS_ID_TYPE_PFMT" but got %"WHEFS_ID_TYPE_PFMT".", i, n.id );
	    return whefs_rc.InternalError;
	}
	if( WHEFS_FLAG_Used & n.flags )
	{
	    whefs_inode_update_used( fs, &n );
	    continue;
	}
	if( markUsed )
	{
	    n.flags |= WHEFS_FLAG_Used;
	    whefs_inode_update_mtime( fs, &n );
	    whefs_inode_flush( fs, &n );
	    fs->hints.unused_inode_start = n.id + 1;
	    // FIXME: error checking!
	}
	*tgt = n;
	//WHEFS_DBG( "Returning next free inode: %"WHEFS_ID_TYPE_PFMT"",tgt->id );
	return whefs_rc.OK;
    }
    WHEFS_DBG_ERR("VFS appears to be full :(");
    if(0) WHEFS_DBG("i=%"WHEFS_ID_TYPE_PFMT", n.id=%"WHEFS_ID_TYPE_PFMT", fs->hints.unused_inode_start=%"WHEFS_ID_TYPE_PFMT", fs->options.inode_count=%"WHEFS_ID_TYPE_PFMT"",
		    i, n.id, fs->hints.unused_inode_start, fs->options.inode_count );
    return whefs_rc.FSFull;
}

int whefs_inode_update_mtime( whefs_fs * fs, whefs_inode * n )
{
    if( ! n ) return whefs_rc.ArgError;
#if 0
    struct timeval tv;
    gettimeofday( &tv, 0 );
    n->mtime = (uint32_t)tv.tv_sec;
#else
    n->mtime = (uint32_t) time(NULL);
#endif
    return whefs_rc.OK;
}

int whefs_inode_search_opened( whefs_fs * fs, whefs_id_type nodeID, whefs_inode ** tgt )
{
    // FIXME: need to lock the fs here, or at least lock fs->opened_nodes.
    if( ! whefs_inode_id_is_valid(fs, nodeID) || !tgt ) return whefs_rc.ArgError;
    whefs_inode_list * li = fs->opened_nodes;
    for( ; li; li = li->next )
    {
	if( li->inode.id < nodeID ) continue;
	else if( li->inode.id > nodeID ) break;
	else
	{
	    //WHEFS_DBG("Found opened node #%"WHEFS_ID_TYPE_PFMT".", nodeID );
	    *tgt = &li->inode;
	    return whefs_rc.OK;
	}
    }
    return whefs_rc.RangeError;
}

int whefs_inode_open( whefs_fs * fs, whefs_id_type nodeID, whefs_inode ** tgt, void const * writer )
{
    if( ! whefs_inode_id_is_valid(fs, nodeID) || !tgt ) return whefs_rc.ArgError;
    //WHEFS_DBG_FYI( "Got request to open inode #%"WHEFS_ID_TYPE_PFMT". writer=@0x%p", nodeID, writer );
    whefs_inode * x = 0;
    int rc = whefs_inode_search_opened( fs, nodeID, &x );
    if( whefs_rc.OK == rc )
    { /* got an existing entry... */
	if(0) WHEFS_DBG_FYI( "Found existing entry for inode %"WHEFS_ID_TYPE_PFMT". entry->writer=@0x%p, writer param=@0x%p",
			     x->id, x->writer, writer );
	if( x->writer )
	{
	    if( x->writer == writer )
	    { /* no-op: opened twice by the same caller */
		WHEFS_DBG_WARN("inode #%"WHEFS_ID_TYPE_PFMT" was opened multiple times by the same writer. Ignoring sebsequent request.",
			       x->id );
		*tgt = x;
		return whefs_rc.OK;
	    }
#if 1
	    else if( writer )
	    {
		/* Only allow one writer for now. Maybe we'll fix this some day. */
		WHEFS_DBG_WARN("Only one writer is allowed on a file at a time, and inode #%"WHEFS_ID_TYPE_PFMT" is already opened for writing by the object at 0x%p.",
			       x->id, x->writer );
		return whefs_rc.AccessError;
	    }
#else
	    else
	    {
		WHEFS_DBG_WARN("inode #%"WHEFS_ID_TYPE_PFMT" is already opened in read/write mode by the object at 0x%p.",
			       x->id, x->writer );
		return whefs_rc.AccessError;
	    }
#endif
	}
	else
	{
	    x->writer = writer;
	}
	++x->open_count;
	*tgt = x;
	if(0) WHEFS_DBG_FYI("Re-using opened inode #%"WHEFS_ID_TYPE_PFMT". Open counter=%u, size=%u",
			    x->id, x->open_count, x->data_size );
	return  whefs_rc.OK;
    }
    /**
       Design note/reminder: the preference would have been to use an
       expanding array of whefs_inode for the opened nodes list, but
       when we realloc() it that could invalidate older pointers to
       those inodes (been there, done that). Thus we suffer a linked
       list and the associated mallocs...
    */
    whefs_inode_list * ent = whefs_inode_list_alloc();
    if( ! ent ) return whefs_rc.AllocError;
    *ent = whefs_inode_list_empty;
    
    ent->inode.id = nodeID;
    rc = whefs_inode_id_read( fs, nodeID, &ent->inode );
    if( whefs_rc.OK != rc )
    {
        whefs_inode_list_free(ent);
	WHEFS_DBG_ERR("Opening inode #%"WHEFS_ID_TYPE_PFMT" FAILED - whefs_inode_id_read() returned %d", ent->inode.id, rc );
	return rc;
    }
    //WHEFS_DBG("Opened inode #%"WHEFS_ID_TYPE_PFMT" with name [%s]", ent->inode.id, ent->inode.name.string );
    x = &ent->inode;
    x->writer = writer;
    whefs_inode_list * li = fs->opened_nodes;
    if( ! li )
    { /* we have the distinction of being the first entry. */
	fs->opened_nodes = li = ent;
    }
    else
    { /* let's keep the list sorted here, as that can save us some comparisons later. */
	while( li->next && (li->inode.id < ent->inode.id) )
	{
	    li = li->next;
	}
	if( (li->inode.id < ent->inode.id) )
	{ /* insert on the right */
	    ent->prev = li;
	    ent->next = li->next;
	    li->next = ent;
	    if( ent->next ) ent->next->prev = ent;
	}
	else
	{ /* insert on the left */
	    ent->next = li;
	    ent->prev = li->prev;
	    li->prev = ent;
	    if( ent->prev ) ent->prev->next = ent;
	}
    }
    /* make sure we keep the list at the proper position... */
    while( fs->opened_nodes->prev ) fs->opened_nodes = fs->opened_nodes->prev;
    if(0) WHEFS_DBG_FYI("Newly opened inode #%"WHEFS_ID_TYPE_PFMT".", x->id, x->open_count );
    x->open_count = 1;
    *tgt = x;
    return whefs_rc.OK;
}

int whefs_inode_close( whefs_fs * fs, whefs_inode * src, void const * writer )
{
    if( ! whefs_inode_is_valid(fs, src) ) return whefs_rc.ArgError;
    if(0) WHEFS_DBG_FYI("Closing shared inode #%"WHEFS_ID_TYPE_PFMT": Use count=%u, data size=%u",
			src->id, src->open_count, src->data_size );
    whefs_inode * np = 0;
    whefs_inode_list * li = fs->opened_nodes;
    for( ; li; li = li->next )
    {
	if( li->inode.id < src->id ) continue;
	else if( li->inode.id > src->id) break;
	//if( li->inode.id != src->id ) continue;
	else
	{
	    if(0) WHEFS_DBG_FYI("Found opened node #%"WHEFS_ID_TYPE_PFMT". We'll close this one.", src->id );
	    np = &li->inode;
	    break;
	}
    }
    if( ! np )
    {
	WHEFS_DBG_ERR("Cannot close inode #%"WHEFS_ID_TYPE_PFMT" - it was not opened via whefs_inode_open() (or is somehow not in the list)!",
		      src->id );
	return whefs_rc.ArgError;
    }
    if( np != src )
    {
	WHEFS_DBG_ERR("Cannot close shared inode #%"WHEFS_ID_TYPE_PFMT": src inode @0x%p is not the same object as the shared inode @0x%p! "
		      "Read the API docs for whefs_inode_open() and whefs_inode_close() for details!",
		      src->id, (void const *)src, (void const *)np );
	return whefs_rc.InternalError;
    }
    if( np->writer && (np->writer == writer) )
    {
	np->writer = 0;
	whefs_inode_flush( fs, np );
    }
    --np->open_count;
    if( 0 == np->open_count )
    {
	if(0) WHEFS_DBG_FYI("REALLY closing inode #%"WHEFS_ID_TYPE_PFMT": Use count=%u, data size=%u",
			    src->id, src->open_count, src->data_size );
	if( li == fs->opened_nodes ) fs->opened_nodes = (li->next ? li->next : li->prev);
	if( li->prev ) li->prev->next = li->next;
	if( li->next ) li->next->prev = li->prev;
	if( np->blocks.list )
	{
	    free(np->blocks.list);
	}
	np->blocks = whefs_block_list_empty;
	whefs_inode_list_free(li);
    }
    if(0) WHEFS_DBG_FYI("%p %p Closed shared inode #%"WHEFS_ID_TYPE_PFMT": Use count=%u, data size=%u",
			np, src, src->id, src->open_count, src->data_size );
    return whefs_rc.OK;
}


int whefs_inode_unlink( whefs_fs * fs, whefs_inode * ino )
{
    if( ! whefs_inode_is_valid(fs,ino) ) return whefs_rc.ArgError;
    while(1)
    {
        whefs_inode * op = 0;
	if( whefs_rc.OK != whefs_inode_search_opened( fs, ino->id, &op ) ) break;
	WHEFS_DBG_WARN("Cannot unlink inode #%"WHEFS_ID_TYPE_PFMT" because it has %u opened handle(s)!",
		       op->id, op->open_count );
	return whefs_rc.AccessError;
    }
    const whefs_id_type nid = ino->id;
    int rc = whefs_rc.OK;
    if( ino->first_block )
    {
	whefs_block bl = whefs_block_empty;
	whefs_block_read( fs, ino->first_block, &bl );
 	rc = whefs_block_wipe( fs, &bl, true, true, true );
    }
    *ino = whefs_inode_empty;
    ino->id = nid;
    return ( whefs_rc.OK != rc )
	? rc
	: whefs_inode_flush( fs, ino );
}

int whefs_inode_id_unlink( whefs_fs * fs, whefs_id_type nid )
{
    if( ! whefs_inode_id_is_valid(fs,nid) ) return whefs_rc.ArgError;
    whefs_inode ino = whefs_inode_empty;
    int rc = whefs_inode_id_read( fs, nid, &ino );
    if( whefs_rc.OK == rc )
    {
	rc = whefs_inode_unlink( fs, &ino );
    }
    return rc;
}


int whefs_inode_by_name( whefs_fs * fs, char const * name, whefs_inode * tgt )
{
    if( ! fs || !name || !*name || !tgt ) return whefs_rc.ArgError;
    if( ! fs->options.inode_count ) return whefs_rc.RangeError;
    const size_t slen = strlen(name);
    if( slen > fs->options.filename_length )
    {
	return whefs_rc.RangeError;
    }
    whefs_string ns = whefs_string_empty;
    int rc = whefs_rc.OK;
    bool expectExact = false; // when true, we stop with error if first guess isn't correct
    const whefs_hashval_type nameHash = fs->cache.hashfunc( name );
#if 1
    if( fs->cache.hashes && !fs->cache.hashes->isSorted )
    {
        whefs_inode_hash_cache_sort(fs);
    }
#endif
    char const * cname = 0; // cached name entry
    whefs_id_type i = whefs_hashid_list_index_of( fs->cache.hashes, nameHash );
    if( whefs_rc.IDTypeEnd == i )
    { // no cached record. Start from the beginning.
        i = 2; // 2 = first client-usable inode.
    }
    else
    { // we know directly what inode record to jump to now...
        expectExact = true;
        WHEFS_DBG_CACHE("Filename matched cached INDEX (%"WHEFS_ID_TYPE_PFMT") for hash code 0x%"WHEFS_HASHVAL_TYPE_PFMT" for name [%s]",i, nameHash,name);
        whefs_hashid * H = &fs->cache.hashes->list[i];
        ++H->hits;
        i = H->id;
    }

    enum { bufSize = WHEFS_MAX_FILENAME_LENGTH+1 };
    unsigned char buf[bufSize] = {0};
    memset(buf,0,bufSize);
    ns.string = (char *)buf;
    ns.alloced = bufSize;
    ns.length = 0;
    rc = whefs_rc.RangeError;
    for( ; i <= fs->options.inode_count; ++i )
    { // brute force... walk the inodes and compare them...
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE // we can't rely on this here.
        if( fs->bits.i_loaded )
        {
            if( ! WHEFS_ICACHE_IS_USED(fs,i) )
            {
                //WHEFS_DBG("Skipping unused inode entry #%"WHEFS_ID_TYPE_PFMT, i );
                if( expectExact )
                {
                    assert(0 && "If we have a cached entry for a specific bit then it must have been flagged as used.");
                    rc = whefs_rc.ConsistencyError;
                    break;
                }
                else continue;
            }
        }
        //WHEFS_DBG("Cache says inode #%i is used.", i );
#endif
        rc = whefs_inode_name_get( fs, i, &ns );
        assert( (ns.string == (char const *)buf) && "Internal consistency error!");
        if( whefs_rc.OK != rc )
        {
            WHEFS_DBG_ERR("whefs_inode_name_get(fs,%"WHEFS_ID_TYPE_PFMT",&ns) returned error code %d. file name=[%s]", i, rc, name );
            break;
        }
        if( 0 && *ns.string ) WHEFS_DBG("Trying inode #%"WHEFS_ID_TYPE_PFMT": name [%s] =? [%s]", i, name, ns.string);
        if( !*ns.string || !ns.length ) continue;
        rc = strcmp( ns.string, name );
        if( 0 == rc )
        {
            cname = ns.string;
            break;
        }
        if( expectExact )
        { /* can't happen */
            assert( 0 && "If expectExact is true then cname must also be non-zero!");
            return whefs_rc.ConsistencyError;
        }
        memset(buf,0,ns.length);
    }
    if( whefs_rc.OK != rc )
    {
        return rc;
    }
    if( ! *buf && ! cname ) return whefs_rc.RangeError;
    assert( cname && "cname must be non-0 or we should have returned by now!" );
    do
    {
        whefs_inode n = whefs_inode_empty;
        rc = whefs_inode_id_read( fs, i, &n );
        if( whefs_rc.OK != rc )
        {
            WHEFS_DBG_ERR("whefs_inode_id_read(fs,node[id=%"WHEFS_ID_TYPE_PFMT"]) returned error code %d. file name=[%s]", n.id, rc, name );
            break;
        }
        *tgt = n;
        rc =  whefs_rc.OK;
    } while(false);
    return rc;
}

int whefs_inode_encode( whefs_inode const * src, unsigned char * dest )
{
    if( ! dest || !src ) return whefs_rc.ArgError;
    unsigned char * x = dest;
    *(x++) = whefs_inode_tag_char;
    whefs_id_encode( x, src->id );
    x += whefs_sizeof_encoded_id_type;

    whio_encode_uint8( x, src->flags );
    x += whio_sizeof_encoded_uint8;

    whio_encode_uint32( x, src->mtime );
    x += whio_sizeof_encoded_uint32;

    whio_encode_uint32( x, src->data_size );
    x += whio_sizeof_encoded_uint32;

    whefs_id_encode( x, src->first_block );
    return whefs_rc.OK;
}

int whefs_inode_decode( whefs_inode * dest, unsigned char const * src )
{

    if( ! dest || !src ) return whefs_rc.ArgError;
    unsigned const char * x = src;
    int rc = 0;
    if( whefs_inode_tag_char != *(x++) )
    {
	return whefs_rc.ConsistencyError;
    }
#define RC if( rc != whefs_rc.OK ) return rc
    rc = whefs_id_decode( x, &dest->id );
    RC;
    x += whefs_sizeof_encoded_id_type;
    rc = whio_decode_uint8( x, &dest->flags );
    RC;
    x += whio_sizeof_encoded_uint8;
    rc = whio_decode_uint32( x, &dest->mtime );
    RC;
    x += whio_sizeof_encoded_uint32;
    rc = whio_decode_uint32( x,  &dest->data_size );
    RC;
#undef RC
    x += whio_sizeof_encoded_uint32;
    rc = whefs_id_decode( x, &dest->first_block );
    return rc;
}

