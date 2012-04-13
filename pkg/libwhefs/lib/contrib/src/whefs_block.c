#include "whefs_details.c"
#include "whefs_encode.h"
#include <string.h> /* memset() */
// FIXME: there are lots of size_t's which should be replaced by whio_size_t
const whefs_block whefs_block_empty = whefs_block_empty_m;
const whefs_block_list whefs_block_list_empty = whefs_block_list_empty_m;

#if ! WHEFS_MACROIZE_SMALL_CHECKS
bool whefs_block_id_is_valid( whefs_fs const * fs, whefs_id_type blid )
{
    return whefs_block_id_is_valid_m(fs,blid);
}

bool whefs_block_is_valid( whefs_fs const * fs, whefs_block const * bl )
{
    return whefs_block_is_valid_m(fs,bl);
}
#endif //  WHEFS_MACROIZE_SMALL_CHECKS


whio_size_t whefs_block_id_pos( whefs_fs const * fs, whefs_id_type id )
{
    if( ! whefs_block_id_is_valid( fs, id ) )
    {
	return 0;
    }
    else
    {
	return fs->offsets[WHEFS_OFF_BLOCKS]
	    + ( (id-1) * fs->sizes[WHEFS_SZ_BLOCK] );
    }
}

void whefs_block_update_used( whefs_fs * fs, whefs_block const * bl )
{
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
    //if( ! whefs_block_id_is_valid( fs, bl ? bl->id : 0) ) return; // this is relatively costly here
    if( ! fs->bits.b_loaded ) return;
    if( bl->flags & WHEFS_FLAG_Used )
    {
	WHEFS_BCACHE_SET_USED(fs,bl->id);
    }
    else
    {
	WHEFS_BCACHE_UNSET_USED(fs,bl->id);
	if( fs->hints.unused_block_start > bl->id )
	{
	    fs->hints.unused_block_start = bl->id;
	}
    }
#endif /* WHEFS_CONFIG_ENABLE_BITSET_CACHE */
}

whio_size_t whefs_fs_sizeof_block( whefs_fs_options const * opt )
{
    return opt
	? (opt->block_size + whefs_sizeof_encoded_block)
	: 0;
}


static whio_size_t whefs_block_id_data_pos( whefs_fs const * fs, whefs_id_type id )
{
    whio_size_t rc = whefs_block_id_pos( fs, id );
    if( rc )
    {
	rc += whefs_sizeof_encoded_block;
    }
    return rc;
}

whio_size_t whefs_block_data_pos( whefs_fs const * fs, whefs_block const * bl )
{
    return bl ? whefs_block_id_data_pos( fs, bl->id ) : 0;
}

int whefs_block_id_seek( whefs_fs const * fs, whefs_id_type id )
{
    whio_size_t p = whefs_block_id_pos( fs, id );
    if( ! p ) return whefs_rc.ArgError;
    whio_size_t sk = fs->dev->api->seek( fs->dev, p, SEEK_SET );
    return (p == sk) ? whefs_rc.OK : whefs_rc.IOError;
}

int whefs_block_seek( whefs_fs const * fs, whefs_block const * bl )
{
    return bl ? whefs_block_id_seek( fs, bl->id ) : 0;
}

int whefs_block_id_seek_data( whefs_fs const * fs, whefs_id_type id, whio_size_t * tgt )
{
    whio_size_t p = whefs_block_id_data_pos( fs, id );
    if( ! p ) return whefs_rc.ArgError;
    whio_size_t sk = fs->dev->api->seek( fs->dev, p, SEEK_SET );
    return (p == sk) ? ((tgt?(*tgt=sk):0),whefs_rc.OK) : whefs_rc.IOError;
}

int whefs_block_seek_data( whefs_fs const * fs, whefs_block const * bl, whio_size_t * tgt )
{
    return bl ? whefs_block_id_seek_data( fs, bl->id, tgt ) : 0;
}


/**
   On-disk blocks are prefixed with this character.
*/
static const unsigned char whefs_block_tag_char = 'B';//0xdf /* sharp S, becase it looks like a B */;

int whefs_block_flush( whefs_fs * fs, whefs_block const * bl )
{
    //if( ! whefs_block_id_is_valid(fs,bl ? bl->id : 0) ) return whefs_rc.ArgError;
    if( ! whefs_fs_is_rw(fs) ) return whefs_rc.AccessError;
    int rc = whefs_rc.OK;

    rc = whefs_block_id_seek( fs, bl->id );
    if( whefs_rc.OK != rc )
    {
        WHEFS_DBG_ERR("FAILED setting correct write position for block #%"WHEFS_ID_TYPE_PFMT"! rc=%d", bl->id, rc );
        return rc;
    }

#if 0
    fs->dev->api->write( fs->dev, &whefs_block_tag_char, 1 );
    whio_size_t check = 0;
    //check = whio_dev_uint32_encode( fs->dev, EXP );
    //if( whio_dev_sizeof_uint32 != check ) return whefs_rc.IOError
    // TODO: encode this data to a buffer and do a single write():
    check = whefs_dev_id_encode( fs->dev, bl->id );
    if( whefs_sizeof_encoded_id_type != check ) return whefs_rc.IOError;
    check = whio_dev_uint32_encode( fs->dev, bl->flags );
    if( whio_dev_sizeof_uint32 != check ) return whefs_rc.IOError;
    check = whefs_dev_id_encode( fs->dev, bl->next_block );
    if( whefs_sizeof_encoded_id_type != check ) return whefs_rc.IOError;
#else
    unsigned char buf[whefs_sizeof_encoded_block] = {0};
    // TODO: use whefs_encode_xxx() here
    buf[0] = whefs_block_tag_char;
    whio_size_t off = 1;
    off += whefs_id_encode( buf + off, bl->id );
    off += whio_encode_uint8( buf + off, bl->flags );
    whefs_id_encode( buf + off, bl->next_block );
    const whio_size_t wsz = fs->dev->api->write( fs->dev, buf, whefs_sizeof_encoded_block );
    if( whefs_sizeof_encoded_block != wsz )
    {
        WHEFS_DBG_ERR("FAILED flushing block info to disk for block #%"WHEFS_ID_TYPE_PFMT". wsz=%"WHIO_SIZE_T_PFMT, bl->id, wsz );
        return whefs_rc.IOError;
    }
#endif
    whefs_block_update_used( fs, bl );
    //WHEFS_DBG("block_write for block #%u returning %d", bl->id, rc );
    return rc;
}

/**
   src must contain an encoded block whefs_sizeof_encoded_block bytes long.
   It is decoded and dest is populated with the results. If any code other
   than whefs_rc.OK is returned then dest is left in an undefined state.
*/
//static
int whefs_block_decode( whefs_block * dest, unsigned char const * src )
{

    if( ! dest || !src ) return whefs_rc.ArgError;
    unsigned const char * x = src;
    int rc = 0;
    if( whefs_block_tag_char != *(x++) )
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
    rc = whefs_id_decode( x, &dest->next_block );
    RC;
#undef RC
    return rc;
}

int whefs_block_read( whefs_fs * fs, whefs_id_type bid, whefs_block * bl )
{
    if( ! whefs_block_id_is_valid(fs,bid)  )
    {
	if( bl )
	{ // WTF?
	    *bl = whefs_block_empty;
	    return whefs_rc.OK;
	}
	return whefs_rc.ArgError;
    }
    whio_size_t to = whefs_block_id_pos( fs, bid );
    if( (whefs_rc.SizeTError == fs->dev->api->seek( fs->dev, to, SEEK_SET )) )
    {
        return whefs_rc.IOError;
    }
#if 0
    // FIXME: error handling!
    // FIXME: read the whole block in one go and decode it from the buffer.
    unsigned char check = 0;
    fs->dev->api->read( fs->dev, &check, 1 );
    if( whefs_block_tag_char != check )
    {
	WHEFS_DBG_ERR("Cursor is not positioned at a data block!");
	return whefs_rc.InternalError;
    }
    whefs_dev_id_decode( fs->dev, &bl->id );
    whio_dev_uint32_decode( fs->dev, &bl->flags );
    whefs_dev_id_decode( fs->dev, &bl->next_block );
#else
    // Read block metadata in one go and decode it from memory.
    unsigned char buf[whefs_sizeof_encoded_block];
    memset( buf, 0, whefs_sizeof_encoded_block );
    whio_size_t iorc = fs->dev->api->read( fs->dev, buf, whefs_sizeof_encoded_block );
    if( iorc != whefs_sizeof_encoded_block )
    {
	WHEFS_DBG_ERR("read() error while reading block #%"WHEFS_ID_TYPE_PFMT". "
                      "Expected %"WHIO_SIZE_T_PFMT" bytes but got %"WHIO_SIZE_T_PFMT"!",
		      bid, whefs_sizeof_encoded_block, iorc );
        return whefs_rc.IOError;
    }
    int rc = whefs_block_decode( bl, buf );
    if( whefs_rc.OK != rc )
    {
	WHEFS_DBG_ERR("Error #%d while decoding block #%"WHEFS_ID_TYPE_PFMT"!",
		      rc, bid );
	return rc;
    }
#endif
    whefs_block_update_used( fs, bl );
    //WHEFS_DBG("Read block #%"WHEFS_ID_TYPE_PFMT". flags=0x%x",bl->id,bl->flags);
    return whefs_rc.OK;
}

int whefs_block_wipe_data( whefs_fs * fs, whefs_block const * bl, whio_size_t startPos )
{
    whio_size_t fpos = 0;
    const size_t bs = whefs_fs_options_get(fs)->block_size;
    if( startPos >= bs ) return whefs_rc.RangeError;
    int rc = whefs_block_id_seek_data( fs, bl->id, &fpos );
    if( whefs_rc.OK != rc ) return rc;
    if( (fpos + bs) < fpos /* overflow! */ ) return whefs_rc.RangeError;
    const size_t count = bs - startPos;
    {
	enum { bufSize = 1024 * 4 };
	static unsigned char buf[bufSize] = {'*',0};
	if( '*' == buf[0] )
	{
	    memset( buf+1, 0, bufSize-1 );
	    buf[0] = 0;
	}
	size_t wrc = 0;
	size_t total = 0;
	while( total < count )
	{
	    const size_t x = count - total;
            const size_t wsz = (bufSize > x) ? x : bufSize;
	    wrc = whefs_fs_write( fs, buf, wsz);
	    if( ! wrc ) break;
            if( wsz != wrc ) return whefs_rc.IOError;
	    total += wrc;
	}
	//WHEFS_DBG("Wrote %u bytes to zero block #%u. Range=[%u .. %u)", total, bl->id, fpos, fpos+count );
    }
    return whefs_rc.OK;
}

int whefs_block_wipe( whefs_fs * fs, whefs_block * bl,
		      bool wipeData,
		      bool wipeMeta,
		      bool deep )
{
    if( ! whefs_block_id_is_valid( fs, bl ? bl->id : 0 ) ) return whefs_rc.ArgError;
    size_t fpos = 0;
    const size_t bs = whefs_fs_options_get(fs)->block_size;
    if( (fpos + bs) < fpos /* overflow! */ ) return whefs_rc.RangeError;
    int rc = 0;
    if( deep && bl->next_block )
    {
	whefs_block next = *bl;
	whefs_block xb = *bl;
	bl->next_block = 0;
	while( xb.next_block )
	{
	    if( whefs_rc.OK != (rc = whefs_block_read_next( fs, &xb, &next )) )
	    {
		WHEFS_DBG_ERR("block #%"WHEFS_ID_TYPE_PFMT": could not load next block, #%"WHEFS_ID_TYPE_PFMT, xb.id, xb.next_block );
		return rc;
	    }
	    xb = next;
	    next.next_block = 0; /* avoid that the next call recurses deeply while still honoring 'deep'. */
	    if( whefs_rc.OK != (rc = whefs_block_wipe( fs, &next, wipeData, wipeMeta, deep )) )
	    {
		WHEFS_DBG_ERR("Error zeroing block #%"WHEFS_ID_TYPE_PFMT"! deep=%s", xb.id, xb.next_block, deep ? "true" : "false" );
		return rc;
	    }
	}
    }
    if( wipeMeta )
    {
	if( ! deep && bl->next_block )
	{
	    WHEFS_FIXME("Warning: we're cleaning up the metadata without cleaning up children! We're losing blocks!");
	}
	const whefs_id_type oid = bl->id;
	//WHEFS_DBG("Wiping block #%"WHEFS_ID_TYPE_PFMT". flags=0x%x",bl->id,bl->flags);
	*bl = whefs_block_empty;
	bl->id = oid;
	rc = whefs_block_flush( fs, bl );
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("Wiping block #%"WHEFS_ID_TYPE_PFMT" failed: flush failed with error code #%d!\n", bl->id, rc);
	    return rc;
	}
	//WHEFS_DBG("Wiped block #%"WHEFS_ID_TYPE_PFMT". flags=0x%x",bl->id,bl->flags);
	if( oid < fs->hints.unused_block_start )
	{
	    fs->hints.unused_block_start = oid;
	}
    }
    if( wipeData )
    {
	rc = whefs_block_wipe_data( fs, bl, 0 );
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("Wiping block #%u failed with error code #%d!\n", bl->id, rc);
	    return rc;
	}
    }
    return whefs_rc.OK;
}

int whefs_block_read_next( whefs_fs * fs, whefs_block const * bl, whefs_block * nextBlock )
{
    if( !nextBlock || !whefs_block_id_is_valid(fs,bl?bl->id:0) ) return whefs_rc.ArgError;
    size_t nb = bl->next_block;
    /** don't reference bl after this, for the case that (bl == nextBlock) */
    if( ! nb ) return whefs_rc.RangeError;
    *nextBlock = whefs_block_empty;
    return whefs_block_read( fs, nb, nextBlock );
}


int whefs_block_next_free( whefs_fs * restrict fs, whefs_block * restrict tgt, bool markUsed )
{
    if( ! fs || !tgt ) return whefs_rc.ArgError;
    whefs_id_type i = fs->hints.unused_block_start;
    if( ! i )
    {
	i = fs->hints.unused_block_start = 1;
    }
    whefs_block bl = whefs_block_empty;
    for( ; i <= fs->options.block_count; ++i )
    {
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
        // TODO(?): i think we could skip 8 entries at a time as long as (0xFF & fs->bits.b.bytes[i*8])
	if( fs->bits.b_loaded && WHEFS_BCACHE_IS_USED(fs,i) )
	{
	    //WHEFS_DBG("Got cached block USED entry for block #%u", i );
	    continue;
	}
	//WHEFS_DBG("Cache says block #%i is unused. markUsed=%d", i, markUsed );
#endif
        // FIXME: try an flock here and skip to the next if we can't get a lock.
        // Use a write lock if markUsed is true???
	int rc = whefs_block_read( fs, i, &bl );
	//WHEFS_DBG("Checking block #%u for freeness. Read rc=%d",i,rc);
	if( whefs_rc.OK != rc )
	{
	    return rc;
	}
	if( bl.id != i )
	{
	    WHEFS_FIXME("Block id mismatch after successful whefs_block_read(). Expected %u but got %u.", i, bl.id );
	    assert( 0 && "block id mismatch after successful whefs_block_read()" );
	    return whefs_rc.InternalError;
	}
	if( WHEFS_FLAG_Used & bl.flags )
	{
	    whefs_block_update_used( fs, &bl );
	    continue;
	}
	if( markUsed )
	{
	    bl.flags = WHEFS_FLAG_Used;
	    whefs_block_flush( fs, &bl );
	    fs->hints.unused_block_start = bl.id + 1;
	    // FIXME: error handling!
	}
	*tgt = bl;
	//WHEFS_DBG( "Returning next free block: #%u",tgt->id );
	return whefs_rc.OK;
    }
    WHEFS_DBG_ERR("VFS appears to be full :(");
    return whefs_rc.FSFull;
}


#if 0 // unused code
int whefs_block_append( whefs_fs * fs, whefs_block const * bl, whefs_block * tgt )
{
    if( ! fs || !tgt ) return whefs_rc.ArgError;
    if( bl && bl->next_block ) return whefs_rc.ArgError;
    int rc = 0;
    const size_t oid = bl ? bl->id : 0;
    if( ! oid )
    { // WTF did i do this for?
	return whefs_block_next_free( fs, tgt, true );
    }
    whefs_block tail = *bl;// = whefs_block_empty;
    while( tail.next_block )
    {
	rc = whefs_block_read_next( fs, &tail, &tail );
	if( whefs_rc.OK != rc ) return rc;
    }
    rc = whefs_block_next_free( fs, tgt, true );
    if( whefs_rc.OK != rc ) return rc;
    tail.next_block = tgt->id;
    whefs_block_flush( fs, &tail );
    whefs_block_flush( fs, tgt );
    return whefs_rc.OK;
}
#endif
