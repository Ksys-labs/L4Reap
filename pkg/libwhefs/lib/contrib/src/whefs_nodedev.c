/*
  This file contains a whio_dev implementation which treats
  whefs_inode objects as an i/o device. Reads and writes are directed
  to/from the VFS data blocks associated with the inode. It has an
  embarassingly intimate relationship with the whefs internals.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain

  Trivia: by using this whio_dev as a target to whefs_mkfs_dev(), it
  is possible to create a VFS within a VFS.
*/
#include <stdlib.h> /* malloc() and friends */
#include <assert.h>
#include <string.h> /* memset() */
#include "whefs_details.c"

/**
   Ensures that ino->block.list is at least count items long,
   reallocating if need. As a special case, if count is 0 the
   list is freed. It may allocate more than count items, and
   ino->blocks.alloced will reflect the actual number.

   This function does not update ino->blocks.count unless count is 0
   (as described above).
*/
static int whefs_inode_block_list_reserve( whefs_fs * fs,
					whefs_inode * ino,
					whefs_id_type count )
{
    if( ! ino ) return whefs_rc.ArgError;
    else if( 0 == count )
    {
	free( ino->blocks.list );
	ino->blocks = whefs_block_list_empty;
	return whefs_rc.OK;
    }
    else if( ino->blocks.alloced >= count ) return whefs_rc.OK;
    //WHEFS_DBG("(Re)sizing inode block cache to %u items for inode #%u[%s].", count, ino->id, ino->name );
    whefs_block * li = (whefs_block *)realloc( ino->blocks.list, count * sizeof(whefs_block) );
    if( ! li )
    {
	return whefs_rc.AllocError;
    }
    ino->blocks.alloced = count;
    ino->blocks.list = li;
    whefs_id_type i = ino->blocks.count;
    for( ; i < count; ++i )
    {
	li[i] = whefs_block_empty;
    }
    return whefs_rc.OK;
}

/**
   Appends a copy of bl to ino's block list, expanding the list as
   necessary. If ino has blocks already, the last block has bl added
   as its next block and that block is flushed to disk.

   ino is assumed to be an opened inode. If it is not, results
   are undefined.

   Returns whefs_rc.OK on success.
*/
static int whefs_inode_block_list_append( whefs_fs * fs,
					  whefs_inode * ino,
					  whefs_block const * bl )
{
    if( ! fs || !ino || !bl ) return whefs_rc.ArgError;
    if( ino->blocks.alloced <= ino->blocks.count )
    {
	int rc = whefs_inode_block_list_reserve( fs, ino, (ino->blocks.count ? ino->blocks.count : 4 /* arbitrarily chosen*/) * 2 );
	if( whefs_rc.OK != rc ) return rc;
    }
    ino->blocks.list[ino->blocks.count] = *bl;
    if( 0 < ino->blocks.count )
    { // append block to the list
	whefs_block * prev = &ino->blocks.list[ino->blocks.count-1];
	if( ! prev->next_block )
	{
	    prev->next_block = bl->id;
	    whefs_block_flush( fs, prev );
	}
	else if( prev->next_block != bl->id )
	{
	    WHEFS_DBG_ERR("Internal error: previous block (#%"WHEFS_ID_TYPE_PFMT") says its next block is #%"WHEFS_ID_TYPE_PFMT", but request was made to append #%"WHEFS_ID_TYPE_PFMT,
			  prev->id, prev->next_block, bl->id );
	    assert( 0 && "Internal consistency error." );
	    return whefs_rc.InternalError;
	}
    }
    else
    { // set bl as the first block...
	if( ino->first_block )
	{
	    if( ino->first_block != bl->id )
	    {
		WHEFS_DBG_ERR("Internal error: inode #%"WHEFS_ID_TYPE_PFMT" says its first block is #%"WHEFS_ID_TYPE_PFMT", but #%"WHEFS_ID_TYPE_PFMT" was added as the first item of its block list.",
			      ino->id, ino->first_block, bl->id );
		assert( 0 && "Internal consistency error." );
		return whefs_rc.InternalError;
	    }
	}
	else
	{
	    ino->first_block = bl->id;
	    whefs_inode_flush( fs, ino );
	}
    }
    ++ino->blocks.count;
    //WHEFS_DBG("Appended block #%"WHEFS_ID_TYPE_PFMT" to chain (of %"WHEFS_ID_TYPE_PFMT" item(s)) for inode #%"WHEFS_ID_TYPE_PFMT"[%s].", bl->id, ino->blocks.count, ino->id, ino->name );
    return whefs_rc.OK;
}

/**
   Assumes ino is an opened inode and loads a block list cache for it.
   If !ino->first_block then this function does nothing but returns
   whefs_rc.OK, otherwise...

   For each block in the chain starting at ino->first_block,
   the block is loaded and appended to ino->blocks.list.

   On success ino->blocks.list contains ino->blocks.count
   whefs_block items representing ino's block chain. To add
   items to the list use whefs_inode_block_list_append().
   To remove them, change ino->blocks.count, empty out
   the now-unused entries, and update the on-disk blocks.

   ino->blocks.count should be 0 when this function is called. If not,
   it may emit a warning debug message but will return a success
   value.
*/
static int whefs_inode_block_list_load( whefs_fs * fs,
					whefs_inode * ino )
{
    if( ! whefs_inode_is_valid(fs,ino) ) return whefs_rc.ArgError;
    if( ! ino->first_block ) return whefs_rc.OK;
    if( ino->blocks.count )
    {
	WHEFS_DBG_WARN("this function shouldn't be called when ino->blocks.count is !0. inode=#%u",
		       ino->id);
	return whefs_rc.OK;
    }
    whefs_block bl = whefs_block_empty;
    int rc = whefs_block_read( fs, ino->first_block, &bl );
    if( whefs_rc.OK != rc ) return rc;
#if 0
    if( ! ino->blocks.list )
    {
	rc = whefs_inode_block_list_reserve( fs, ino, 5 /* arbitrarily chosen */ );
	if( whefs_rc.OK != rc ) return rc;
    }
#endif
    rc = whefs_inode_block_list_append( fs, ino, &bl );
    if( whefs_rc.OK != rc ) return rc;
    while( bl.next_block )
    {
	rc = whefs_block_read_next( fs, &bl, &bl );
	if( whefs_rc.OK != rc ) return rc;
        rc = whefs_inode_block_list_append( fs, ino, &bl );
	if( whefs_rc.OK != rc ) return rc;
    }
    //WHEFS_DBG("Loaded block chain of %u block(s) for inode #%u[%s].", ino->blocks.count, ino->id, ino->name );
    return whefs_rc.OK;
}

/**
   This function is the heart of the pseudofile i/o beast...

   It tries to map a logical file position to a data block
   associated with an inode.

   It starts with ino's first block and increments blocks until the
   block in which pos would land is found. If ino doesn't have enough
   blocks, the behaviour is defined by the expands parameter:

   If expands is true then it will add blocks to the inode's chain in
   order to reach the destination pos, if necessary. If expands is
   false and pos is not within the inode's current data size then the
   function fails.

   On success, tgt is populated with the block associated with the
   given position and inode, and ino *may* be updated (if it had no
   blocks associated with it beforehand). To figure out the proper
   offset of pos to use within the block use
   (pos%whefs_fs_options_get(fs)->block_size).

   This function never actually changes the logical size of the inode,
   but may allocate new blocks to it.

   On success whefs_rc.OK is returned, else some other error
   value. Some possibilities include:

   - whefs_rc.RangeError = pos it past EOF and expands is false.
   - whefs_rc.FSFull = ran out of blocks while trying to expand.
   - whefs_rc.ArgEror = !fs, !tgt, or ino is not valid

   BIG FAT HAIRY WARNING:

   It is intended that the ino argument be an inode which has been
   opened via whefs_inode_open(), but this function does not check
   that because doing so is relatively costly and this routine is
   called from the i/o device implementation for every read and write.

   Because the ino argument *may* be updated, it is imperative that if
   ino refers to an opened inode, that the ino argument actually be a
   pointer to that object, as opposed to being a copy of it. Failure
   to follow this may result in mis-synchronization of the node's
   state or a memory leak. Specifically, if the inode previously had
   no blocks, and this function adds at least one, then ino must be
   updated. If ino has the same ID as an opened inode but is not that
   opened inode object (see whefs_inode_search_opened()), then ino
   will be updated but the opened inode will not, which will probably
   lead to any new blocks allocated by this call to become lost the
   next time the opened inode is flushed.


   BIGGER, FATTER, HAIRIER WARNING:

   Because profiling has shown that this function spends a significant
   amount of time validating fs and ino (by calling
   whefs_inode_is_valid()), that check has been removed. Since this
   function "can only" be called from the whefs_nodedev
   implementation, we're relying on that to do the validation (which
   it does).

*/
static int whefs_block_for_pos( whefs_fs * restrict fs, whefs_inode * restrict ino, whio_size_t pos, whefs_block * restrict tgt, bool expand )
{
    //if(  !tgt || !whefs_inode_is_valid( fs, ino ) ) return whefs_rc.ArgError;
    if( (ino->data_size <= pos) && !expand )
    {
	//WHEFS_DBG("return whefs_rc.RangeError");
	return whefs_rc.RangeError;
    }
    const whio_size_t bs = whefs_fs_options_get(fs)->block_size;
    const whefs_id_type bc = /* how many blocks will we need? */
        1 + (pos / bs);
    if(0) WHEFS_DBG("pos=%"WHIO_SIZE_T_PFMT" bs=%"WHIO_SIZE_T_PFMT" bc=%"WHEFS_ID_TYPE_PFMT,pos,bs,bc);
    /** ^^^ does this leave us with one too many blocks when we truncate() to
        an exact multiple of blocksize? */
    if( bc > whefs_fs_options_get(fs)->block_count )
    {
        WHEFS_DBG_WARN("VFS doesn't have enough blocks "
                       "(%"WHEFS_ID_TYPE_PFMT") to satisfy the "
                       "request for position %"WHIO_SIZE_T_PFMT
                       " of inode #%"WHEFS_ID_TYPE_PFMT,
                       whefs_fs_options_get(fs)->block_count, pos, ino->id );
        return whefs_rc.RangeError;
    }
    int rc = whefs_rc.OK;
    if( ! ino->blocks.list )
    {
	rc = whefs_inode_block_list_load( fs, ino );
	if( whefs_rc.OK != rc ) return rc;
    }
    if( !expand && (ino->blocks.count < bc) )
    { /* can't grow list for this request. */
	return whefs_rc.RangeError;
    }
    // TODO: check number of available inodes here, and don't try to expand if we can't reach the end
    whefs_block bl = whefs_block_empty;
    //WHEFS_DBG("About to search inode #%u for %u block(s) (size=%u) to find position %u", ino->id, bc, bs, pos );
    rc = whefs_rc.OK;
    whefs_block * blP = 0;
    if( bc <= ino->blocks.count)
    {
	blP = &ino->blocks.list[bc-1]; /* jump right to it */;
    }
    else
    { /* expand the list */
	if( ! expand )
	{
	    if(0) WHEFS_DBG("Cannot expand to %"WHEFS_ID_TYPE_PFMT" blocks for position %"WHIO_SIZE_T_PFMT" because [expand] parameter is false.",
			    bc, pos );
	    return whefs_rc.RangeError;
	}
	//bl = ino->blocks.list[ino->blocks.count-1];
	whefs_id_type i = ino->blocks.count;
	blP = 0;
	for( ; i < bc; ++i )
	{
	    rc = whefs_block_next_free( fs, &bl, true );
	    if( whefs_rc.OK == rc ) rc = whefs_inode_block_list_append( fs, ino, &bl );
	    if( whefs_rc.OK != rc ) return rc;
	    /**
	       We "might" want to truncate the inode back to its
	       previous length, but why add room for yet another error
	       on top of the one we just encountered?
	    */
	}
	blP = &bl;
    }
    if( whefs_rc.OK == rc )
    {
	*tgt = *blP;
    }
    //WHEFS_DBG("Using block id #%u for pos %u of inode #%u", blP->id, pos, ino->id );
    return rc;
}

/**
   Internal implementation details for the whio_dev whefs_inode
   wrapper.
*/
typedef struct whio_dev_inode_meta
{
    /**
       Associated filesystem.
     */
    whefs_fs * fs;
    /* block size, to avoid having to continually deref fs->options.block_size. */
    uint32_t bs;
    /**
       File position cursor.
    */
    whio_size_t posabs;
    /** rw==true if read/write, else false. */
    bool rw;
    /** inode associated with device. */
    whefs_inode * inode;
} whio_dev_inode_meta;

/** Initializer object. */
#define WHIO_DEV_INODE_META_INIT { \
0, /* fs */ \
0, /* bs */ \
0, /* posabs */ \
false, /* read/write */ \
0 /* inode */  \
}

static const whio_dev_inode_meta whio_dev_inode_meta_empty = WHIO_DEV_INODE_META_INIT;


#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC /* see whio_common.h for details */
enum {
/**
   The number of elements to statically allocate
   in the whio_dev_inode_meta_alloc_slots object.
*/
whio_dev_inode_meta_alloc_count = 10
};
static struct
{
    whio_dev_inode_meta objs[whio_dev_inode_meta_alloc_count];
    char used[whio_dev_inode_meta_alloc_count];
    size_t next;
} whio_dev_inode_meta_alloc_slots = { {WHIO_DEV_INODE_META_INIT},{0}, 0 };
#endif

static whio_dev_inode_meta * whio_dev_inode_meta_alloc()
{
    whio_dev_inode_meta * obj = 0;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = whio_dev_inode_meta_alloc_slots.next;
    for( ; i < whio_dev_inode_meta_alloc_count; ++i )
    {
	if( whio_dev_inode_meta_alloc_slots.used[i] ) continue;
	whio_dev_inode_meta_alloc_slots.next = i+1;
	whio_dev_inode_meta_alloc_slots.used[i] = 1;
	obj = &whio_dev_inode_meta_alloc_slots.objs[i];
	break;
    }
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whio_dev_inode_meta *) malloc( sizeof(whio_dev_inode_meta) );
    if( obj ) *obj = whio_dev_inode_meta_empty;
    return obj;
}

static void whio_dev_inode_meta_free( whio_dev_inode_meta * obj )
{
    if( obj ) *obj = whio_dev_inode_meta_empty;
    else return;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whio_dev_inode_meta_alloc_slots.objs[0]) ||
	(obj > &whio_dev_inode_meta_alloc_slots.objs[whio_dev_inode_meta_alloc_count-1]) )
    { /* it does not belong to us */
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &whio_dev_inode_meta_alloc_slots.objs[0]);
	whio_dev_inode_meta_alloc_slots.used[ndx] = 0;
	if( ndx < whio_dev_inode_meta_alloc_slots.next ) whio_dev_inode_meta_alloc_slots.next = ndx;
	return;
    }
#else
    free(obj);
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
}


/**
   A helper for the whio_dev_inode API. Requires that the 'dev'
   parameter be-a whio_dev and that that device is-a whio_dev_inode_meta.
 */
#define WHIO_DEV_DECL(RV) whio_dev_inode_meta * meta = (dev ? (whio_dev_inode_meta*)dev->impl.data : 0); \
    if( !meta || ((void const *)&whio_dev_inode_meta_empty != dev->impl.typeID) ) return RV

/**
   Internal implementation of whio_dev_inode_read(). All arguments
   are as for whio_dev::read() except keepGoing:

   If this routine must "wrap" over multiple blocks, it will read what it
   can from one block and then set keepGoing to true and return the read
   size. The caller should, if keepGoing is true, call this function
   again, adding the returned size to dest and subtracting it from n.
*/
static whio_size_t whio_dev_inode_read_impl( whio_dev * dev,
					whio_dev_inode_meta * meta,
					void * dest,
					whio_size_t n,
					bool * keepGoing )
{
#if 0
    if( ! dev || !meta || !dest || !n || ! keepGoing )
    {
	if( keepGoing ) *keepGoing = false;
	return 0;
    }
#endif
    *keepGoing = false;
    if( ! n ) return 0U;
    else if( meta->posabs >= meta->inode->data_size ) return 0;
    int rc = 0;
    //whio_size_t eofpos = meta->inode->data_size;
    whefs_block block = whefs_block_empty;
    rc = whefs_block_for_pos( meta->fs, meta->inode, meta->posabs, &block, false );
    if( whefs_rc.OK != rc )
    {
#if 0
        if( !(meta->posabs % meta->fs->options.block_size) )
        {
            // FIXME: ensure that meta->inode->blocks[end] is really the end block that should line up here.
            /**
               This is an unusual special case. Directly at the EOF boundary we want to return
               a non-error code. We're at EOF. whefs_block_for_pos() isn't quite smart enough
               to know that, though.

               This fix should go in whefs_block_for_pos()
            */
            WHEFS_FIXME("Special-case block boundary story: inode #%"WHEFS_ID_TYPE_PFMT": error #%d getting block for meta->posabs=%u. n=%"WHIO_SIZE_T_PFMT", bs=%"WHIO_SIZE_T_PFMT", at end of the block chain.",
                        meta->inode->id, rc, meta->posabs, n, meta->fs->options.block_size );
            return 0;
        }
        else
        {
            WHEFS_DBG("Error #%d getting block for meta->posabs=%u. n=%"WHIO_SIZE_T_PFMT", bs=%"WHIO_SIZE_T_PFMT,
                      rc, meta->posabs, n, meta->fs->options.block_size );
            return 0;
        }
#else
        WHEFS_DBG("Error #%d getting block for meta->posabs=%u. n=%"WHIO_SIZE_T_PFMT", bs=%"WHIO_SIZE_T_PFMT,
                  rc, meta->posabs, n, meta->fs->options.block_size );
        return 0;
#endif
    }
    if(0) WHEFS_DBG("inode #%"WHEFS_ID_TYPE_PFMT" will be using block #%"WHEFS_ID_TYPE_PFMT" for a read at pos %"WHIO_SIZE_T_PFMT, meta->inode->id, block.id, meta->posabs );

    if( meta->posabs >= meta->inode->data_size ) return 0;
    const whio_size_t rdpos = (meta->posabs % meta->bs);
    const whio_size_t left = meta->bs - rdpos;
    const whio_size_t bdpos = whefs_block_data_pos( meta->fs, &block );
    whio_size_t rdlen = ( n > left ) ? left : n;
    if( (rdlen + meta->posabs) >= meta->inode->data_size )
    {
	rdlen = meta->inode->data_size - meta->posabs;
    }
    //WHEFS_DBG("rdpos=%u left=%u bdpos=%u rdlen=%u", rdpos, left, bdpos, rdlen );
    whio_dev * fd = meta->fs->dev;
    fd->api->seek( fd, bdpos + rdpos, SEEK_SET );
    const whio_size_t sz = fd->api->read( fd, dest, rdlen );
    if( ! sz ) return 0;
    const whio_size_t szCheck = meta->posabs + sz;
    if( szCheck > meta->posabs )
    {
	meta->posabs = szCheck;
    }
    else
    {
	WHEFS_DBG_ERR("Numeric overflow in read! (pos=%"WHIO_SIZE_T_PFMT" + readLength=%"WHIO_SIZE_T_PFMT") = overflow", meta->posabs, sz );
        return 0;
    }
    //whefs_block_flush( meta->fs, &block );
    if(0) WHEFS_DBG("Read %"WHIO_SIZE_T_PFMT" of %"WHEFS_ID_TYPE_PFMT" (n=%"WHIO_SIZE_T_PFMT") bytes "
		    "from inode #%"WHEFS_ID_TYPE_PFMT"'s block #%"WHEFS_ID_TYPE_PFMT". "
		    "fs pos=%"WHIO_SIZE_T_PFMT", block offset=%"WHIO_SIZE_T_PFMT" file pos=%"WHIO_SIZE_T_PFMT", file eof=%"WHIO_SIZE_T_PFMT,
		    sz, rdlen, n,
		    meta->inode->id, block.id,
		    bdpos, rdpos, meta->posabs, meta->inode->data_size );
    if( sz < rdlen )
    { /* short write! */
	return sz;
    }
    else if( rdlen < n )
    { /* Wrap to next block and continue... */
	*keepGoing = true;
	return sz;
    }
    else
    { /* got the exact right amount */
	return sz;
    }
}

static whio_size_t whio_dev_inode_read( whio_dev * dev, void * dest, whio_size_t n )
{
    WHIO_DEV_DECL(0);
    bool keepGoing = true;
    whio_size_t total = 0;
    while( keepGoing )
    {
	const whio_size_t sz = whio_dev_inode_read_impl( dev, meta, WHIO_VOID_PTR_ADD(dest,total), n - total, &keepGoing );
	total += sz;
    }
    return total;
}

/**
   This function's logic and handling of the keepGoing parameter are
   identical to that of whio_dev_inode_read_impl(), but apply to
   writes instead of reads.
*/
static whio_size_t whio_dev_inode_write_impl( whio_dev * dev,
					 whio_dev_inode_meta * meta,
					 void const * src, whio_size_t n,
					 bool * keepGoing )
{
    if( ! dev || !meta || !src || !n || !keepGoing )
    {
	if( keepGoing ) *keepGoing = false;
	return 0;
    }
    *keepGoing = false;
    int rc = 0;
    //whio_size_t eofpos = meta->inode->data_size;
    whefs_block block = whefs_block_empty;
    rc = whefs_block_for_pos( meta->fs, meta->inode, meta->posabs, &block, true );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG("Error #%d getting block for meta->posabs==%u", rc, meta->posabs );
	return 0;
    }
    const whio_size_t wpos = (meta->posabs % meta->bs);
    const whio_size_t left = meta->bs - wpos;
    const whio_size_t bdpos = whefs_block_data_pos( meta->fs, &block );
    const whio_size_t wlen = ( n > left ) ? left : n;
    //WHEFS_DBG("wpos=%u left=%u bdpos=%u wlen=%u", wpos, left, bdpos, wlen );
    whio_dev * fd = meta->fs->dev;
    fd->api->seek( fd, bdpos + wpos, SEEK_SET );
    whio_size_t sz = fd->api->write( fd, src, wlen );
    if( ! sz ) return 0;
    whefs_inode_update_mtime( meta->fs, meta->inode );
    whio_size_t szCheck = meta->posabs + sz;
    if( szCheck > meta->posabs )
    {
	meta->posabs = szCheck;
    }
    if( meta->inode->data_size < meta->posabs )
    {
	meta->inode->data_size = meta->posabs;
	//whefs_inode_flush( meta->fs, &meta->inode ); // we should do this, really.
    }
    //whefs_block_flush( meta->fs, &block );
    if(0) WHEFS_DBG("Wrote %u of %u (n=%u) bytes "
		    "to inode #%u's block #%u. "
		    "fs pos=%u, block offset=%u file pos=%u, file eof=%u",
		    sz, wlen, n,
		    meta->inode->id, block.id,
		    bdpos, wpos, meta->posabs, meta->inode->data_size );
    if( sz < wlen )
    { /* short write! */
	return sz;
    }
    else if( wlen < n )
    { /* Wrap to next block and continue... */
	*keepGoing = true;
	return sz;
    }
    else
    {
	return sz;
    }
}

static whio_size_t whio_dev_inode_write( whio_dev * dev, void const * src, whio_size_t n )
{
    WHIO_DEV_DECL(0);
    if( ! meta->rw ) return 0;
    bool keepGoing = true;
    whio_size_t total = 0;
    while( keepGoing )
    {
	const whio_size_t sz = whio_dev_inode_write_impl( dev, meta, WHIO_VOID_CPTR_ADD(src,total), n - total, &keepGoing );
	total += sz;
    }
    return total;
}

static int whio_dev_inode_error( whio_dev * dev )
{
    WHIO_DEV_DECL(whio_rc.ArgError);
    return whio_rc.OK;
}

static int whio_dev_inode_clear_error( whio_dev * dev )
{
    WHIO_DEV_DECL(whio_rc.ArgError);
    return whio_rc.OK;
}

static int whio_dev_inode_eof( whio_dev * dev )
{
    WHIO_DEV_DECL(whio_rc.ArgError);
    return (meta->posabs >= meta->inode->data_size)
	? 1
	: 0;
}

static whio_size_t whio_dev_inode_tell( whio_dev * dev )
{
    WHIO_DEV_DECL(whio_rc.SizeTError);
    return meta->posabs;
}

static whio_size_t whio_dev_inode_seek( whio_dev * dev, whio_off_t pos, int whence )
{
    WHIO_DEV_DECL(whio_rc.SizeTError);
    whio_size_t too = meta->posabs;
    switch( whence )
    {
      case SEEK_SET:
	  if( pos < 0 ) return whio_rc.SizeTError;
	  too = (whio_size_t)pos;
	  break;
      case SEEK_END:
	  too = meta->inode->data_size + pos;
	  //if( too < meta->inode->data_size )  /* overflow! */ return whio_rc.SizeTError;
	  break;
      case SEEK_CUR:
	  too += pos;
	  if( too < meta->posabs )  /* overflow! */ return whio_rc.SizeTError;
	  break;
      default:
	  return whio_rc.SizeTError;
	  break;
    };
    return (meta->posabs = too);
}

static int whio_dev_inode_flush( whio_dev * dev )
{
    WHIO_DEV_DECL(whio_rc.ArgError);
    if(0) WHEFS_DBG_FYI("Flushing i/o %s device for inode #%"WHIO_SIZE_T_PFMT". "
			"inode->data_size=%"WHIO_SIZE_T_PFMT" posabs=%"WHIO_SIZE_T_PFMT,
			meta->rw ? "read/write" : "read-only", meta->inode->id,
			meta->inode->data_size, meta->posabs
			);
    int rc = meta->rw
	? whefs_inode_flush( meta->fs, meta->inode )
	: whefs_rc.OK;
    if( meta->rw )
    {
        whefs_fs_flush( meta->fs );
    }
    if(0) WHEFS_DBG_FYI("Flushed (rc=%d) i/o %s device for inode #%"WHEFS_ID_TYPE_PFMT". "
			"inode->data_size=%"WHIO_SIZE_T_PFMT" posabs=%"WHIO_SIZE_T_PFMT,
			rc, meta->rw ? "read/write" : "read-only", meta->inode->id,
			meta->inode->data_size, meta->posabs
			);
    return rc;
}

static int whio_dev_inode_trunc( whio_dev * dev, whio_off_t len )
{
    /* Man, this was a bitch to do! */
    WHIO_DEV_DECL(whio_rc.ArgError);
    if( len < 0 ) return whio_rc.ArgError;
    if( ! meta->rw ) return whio_rc.AccessError;
    const whio_size_t off = (whio_size_t)len;
    if( off > len ) return whio_rc.RangeError; /* overflow */
    if( off == meta->inode->data_size ) return whefs_rc.OK;

    if( 0 == len )
    { /* special (simpler) case for 0 byte truncate */
	// (WTF?) FIXME: update ino->blocks.list[0]
        if( meta->inode->first_block ) 
        {
            whefs_block block = whefs_block_empty;
            int rc = whefs_block_read( meta->fs, meta->inode->first_block, &block ); // ensure we pick up whole block chain
            if( whefs_rc.OK != rc )
            {
                rc = whefs_block_wipe( meta->fs, &block, true, true, true );
            }
            if( whefs_rc.OK != rc ) return rc;
        }
	meta->inode->blocks.count = 0;
	meta->inode->first_block = 0;
	meta->inode->data_size = 0;
	whefs_inode_flush(meta->fs, meta->inode );
	return whio_rc.OK;
    }

    int rc = whio_rc.OK;
    //const size_t oldSize = off>meta->inode->data_size;
    const short dir = (off < meta->inode->data_size)
	? -1
	: ((off>meta->inode->data_size) ? 1 : 0);
    assert( (0 != off) && "This shouldn't be able to happen!" );

    /* Update inode metadata... */
    //WHEFS_DBG("truncating from %u to %u bytes",meta->inode->data_size, off);
    meta->inode->data_size = off;
    rc = whefs_inode_flush( meta->fs, meta->inode );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Flush failed for inode #%u. Error code=%d.",
		      meta->inode->id, rc );
	return rc;
    }
    /* Update block info... */
    whefs_block bl = whefs_block_empty;
    rc = whefs_block_for_pos( meta->fs, meta->inode, off, &bl, true );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Could not get block for write position %u of inode #%u. Error code=%d.",
		      off, meta->inode->id, rc );
	return rc;
    }
    //const size_t dest = meta->inode->data_size;
    if( dir < 0 )
    { /* we shrunk */
#if 1
	/*
	  We'll be nice and zero the remaining bytes... We do this
	  partially for consistency with how blocks will get removed
	  (they get wiped as well).  Theoretically we don't need this
	  because they get wiped when created and when unlinked, but a
	  failed unlink could leave data lying around, so we clean it
	  here. Maybe we should consider a 'dirty' flag for blocks,
	  wiping only dirty blocks, but that could get messy (no pun
	  intended).
	*/
	const uint32_t bs = whefs_fs_options_get( meta->fs )->block_size;
	rc = whefs_block_wipe_data( meta->fs, &bl, ( off % bs ) );
	if( whefs_rc.OK != rc ) return rc;
#endif
	if( ! bl.next_block )
	{ /* Lucky for us! No more work to do! */
	    meta->inode->blocks.count = 1;
	    return whefs_rc.OK;
	}

	whefs_block * blP = &meta->inode->blocks.list[0];
	whefs_block * nblP = blP + 1;
	uint32_t x = 1;
	for( ; (x < meta->inode->blocks.count)
		 && (nblP->id != bl.next_block)
		 ; ++nblP, ++x )
	{
	    /* Skip to bl.next_block */
	}
	if( (x == meta->inode->blocks.count) || (nblP->id != bl.next_block) )
	{
	    WHEFS_DBG_ERR("nblP->id=%u, bl.next_block=%u", nblP->id, bl.next_block );
	    WHEFS_DBG_ERR("Internal block cache for inode #%u is not as "
			  "long as we expect it to be or is missing entries!",
			  meta->inode->id );
	    return whefs_rc.InternalError;
	}
	blP = nblP - 1;
	meta->inode->blocks.count = x;
	whefs_block_wipe( meta->fs, nblP, true, true, true );
	blP->next_block = 0;
	return whefs_block_flush( meta->fs, blP );
    }
    else if( dir > 0 )
    { /* we grew - fill the new bytes with zeroes */
	/*
	  Actually... since we zero these when shrinking and during mkfs(),
	   we probably don't need to do this.
	*/
	enum { bufSize = 1024 * 4 };
	unsigned char buf[bufSize];
	memset( buf, 0, bufSize );
	const whio_size_t PosAbs = meta->posabs;
	const whio_size_t orig = meta->inode->data_size;
	const whio_size_t dest = off;
	dev->api->seek( dev, orig, SEEK_SET );
	whio_size_t wlen = dest - orig;
	whio_size_t iorc = 0;
        whio_size_t wsz = 0;
	do
	{
            wsz = (wlen < bufSize) ? wlen : bufSize;
	    iorc = dev->api->write( dev, buf, wsz );
	    wlen -= iorc;
	}
	while( iorc && (iorc == wsz) );
	iorc = dev->api->seek( dev, PosAbs, SEEK_SET );
	return (iorc == PosAbs)
	    ? whefs_rc.OK
	    : whefs_rc.IOError;
    }
    else
    {
	/* cannot happen due to special-case handling of truncate(0), above. */
	assert( 0 && "This is impossible!" );
    }
    WHEFS_DBG("You should never have gotten to this line!");
    return whefs_rc.InternalError;
}

short whio_dev_inode_iomode( whio_dev * dev )
{
    WHIO_DEV_DECL(-1);
    return (meta->inode->writer == dev) ? 1 : 0;
}

static int whio_dev_inode_ioctl( whio_dev * dev, int arg, va_list vargs )
{
    int rc = whio_rc.UnsupportedError;
    WHIO_DEV_DECL(whio_rc.ArgError);
    switch( arg )
    {
      case whio_dev_ioctl_GENERAL_size:
	  rc = whio_rc.OK;
	  *(va_arg(vargs,whio_size_t*)) = meta->inode->data_size;
	  break;
      default: break;
    };
    return rc;
}

static bool whio_dev_inode_close( whio_dev * dev )
{
    if( dev && ((void const *)&whio_dev_inode_meta_empty == dev->impl.typeID))
    {
	if( dev->client.dtor ) dev->client.dtor( dev->client.data );
	dev->client = whio_client_data_empty;
	whio_dev_inode_meta * meta = (whio_dev_inode_meta*)dev->impl.data;
	if( meta )
	{
            whefs_fs_closer_dev_remove( meta->fs, dev );
	    dev->impl.data = 0;
            if( meta->rw ) dev->api->flush(dev);
	    if(0) WHEFS_DBG_FYI("Closing i/o %s device for inode #%u. "
				"inode->data_size=%u posabs=%u",
				meta->rw ? "read/write" : "read-only",
				meta->inode->id,
				meta->inode->data_size, meta->posabs
				);
	    whefs_inode_close( meta->fs, meta->inode, dev );
	    whio_dev_inode_meta_free( meta );
	    return true;
	}
    }
    return false;
}


static void whio_dev_inode_finalize( whio_dev * dev )
{
    if( dev && ((void const *)&whio_dev_inode_meta_empty == dev->impl.typeID))
    {
	if(0)
	{
	    whio_dev_inode_meta * meta = (whio_dev_inode_meta*)dev->impl.data;
	    WHEFS_DBG_FYI("Finalizing %s i/o device for inode #%u. "
			  "inode->data_size=%u posabs=%u",
			  meta->rw ? "read/write" : "read-only",
			  meta->inode->id,
			  meta->inode->data_size, meta->posabs
			  );
	    }
	dev->api->close( dev );
	whio_dev_free( dev );
    }
}

static const whio_dev_api whio_dev_api_inode_empty =
    {
    whio_dev_inode_read,
    whio_dev_inode_write,
    whio_dev_inode_close,
    whio_dev_inode_finalize,
    whio_dev_inode_error,
    whio_dev_inode_clear_error,
    whio_dev_inode_eof,
    whio_dev_inode_tell,
    whio_dev_inode_seek,
    whio_dev_inode_flush,
    whio_dev_inode_trunc,
    whio_dev_inode_ioctl,
    whio_dev_inode_iomode
    };

static const whio_dev whio_dev_inode_empty =
    {
    &whio_dev_api_inode_empty,
    { /* impl */
    0, /* data. Must be-a (whio_dev_inode_meta*) */
    (void const *)&whio_dev_inode_meta_empty /* typeID */
    }
    };

#undef WHIO_DEV_DECL



whio_dev * whefs_dev_for_inode( whefs_fs * fs, whefs_id_type nid, bool writeMode )
{
    //WHEFS_DBG("trying to open dev for inode #%u", nid );
    if( ! whefs_inode_id_is_valid( fs, nid ) ) return 0;
    whio_dev * dev = whio_dev_alloc();
    if( ! dev ) return 0;
    whefs_inode * ino = 0;
    void const * writeKey = (writeMode ? dev : 0);
    int rc = whefs_inode_open( fs, nid, &ino, writeKey );
    if( rc != whefs_rc.OK )
    {
	WHEFS_DBG_ERR("whefs_inode_open(fs,[inode #%"WHEFS_ID_TYPE_PFMT"],inode,%d) failed with rc %d!", nid, writeMode, rc );
	whio_dev_free( dev );
	return 0;
    }
    //WHEFS_DBG("Opened inode #%u[%s]", ino->id, ino->name );
    whio_dev_inode_meta * meta = whio_dev_inode_meta_alloc();
    if( ! meta )
    {
	whefs_inode_close( fs, ino, writeKey );
	whio_dev_free(dev);
	return 0;
    }
    *dev = whio_dev_inode_empty;
    *meta = whio_dev_inode_meta_empty;
    dev->impl.data = meta;
    meta->fs = fs;
    meta->rw = writeMode;
    meta->bs = whefs_fs_options_get(fs)->block_size;
    meta->inode = ino;
#if 0
    if( writeMode )
    {
	whefs_inode_flush( fs, meta->inode ); /* make sure on-disk matches */
    }
#endif
    whefs_fs_closer_dev_add( fs, dev );
    return dev;
}

