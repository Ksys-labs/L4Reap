#include "whefs_details.c"
#include <wh/whefs/whefs_client_util.h>
#include <wh/whglob.h>
#include <string.h> // memset()

int whefs_test_insert_dummy_files( whefs_fs * fs )
{
    if( ! fs || ! fs->dev ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    whefs_file * F = 0;
    whefs_file_stats st;
    size_t nid = 0;
    char const * fname = "test_file_number_1";
    size_t szrc = 0;

#define INSERT(FN) \
    fname = FN; \
    F = whefs_fopen( fs, FN, "r+" );	   \
    assert( F && "whefs_fopen() failed!"); \
    nid = F->inode; \
    szrc = whefs_fwrite( F, 3, 1, "hi!" );				\
    WHEFS_DBG("fwrite result=%u",szrc); \
    rc = whefs_fstat( F, &st ); \
    WHEFS_DBG("fstat rc=%d\t F->inode = %u\tF->bytes = %u\tF->blocks = %u", rc, st.inode, st.bytes, st.blocks ); \
    rc = whefs_fclose( F ); \
    assert( (whefs_rc.OK == rc) && "whefs_fclose() failed!" ); \
    F = 0;

    INSERT("test_file_number_1");
    INSERT("test_file_number_2");
#undef INSERT

    whefs_inode ino;
    whio_dev * dev = 0;
#if 1
    whefs_inode_id_read( fs, nid, &ino );
    whefs_inode_name_set( fs, ino.id, "will be renamed");
    //whefs_inode_save( fs, &ino );
    dev = whefs_dev_for_inode( fs, ino.id, true );
    WHEFS_DBG("dev=%p",(void const *)dev);
    assert( dev && "opening of device for inode failed!" );
    nid = dev->api->write( dev, "Watson, come here!", 18 );
    WHEFS_DBG("write rc=%u", nid );
    nid = dev->api->write( dev, "\nComing, Alexander!", 19 );
    WHEFS_DBG("write rc=%u", nid );
    dev->api->finalize(dev);
#endif

#if 1
    //whefs_inode_next_free( fs, &ino, true );
    //ino = whefs_inode_empty; ino.id = nid;
    whefs_inode_id_read( fs, ino.id, &ino );
    WHEFS_DBG("Trampling over inode #%u", ino.id );
    fname = "via whio_dev_inode";
    whefs_inode_name_set( fs, ino.id, fname );
    dev = whefs_dev_for_inode( fs, ino.id, true );
    WHEFS_DBG("dev=%p",(void const *)dev);
    assert( dev && "couldn't re-open device!");
    dev->api->flush(dev);
    assert( dev && "opening of device for inode failed!" );
    size_t i = 0;
    size_t total = 0;
    for( ; i < 10; ++i )
    {
	nid = whio_dev_writef( dev, "Test #%02u", i );
	total += nid;
    }
    WHEFS_DBG("dev size=%u", whio_dev_size(dev) );
    dev->api->truncate( dev, 0 );
    dev->api->seek( dev, 30, SEEK_SET );
    WHEFS_DBG("dev size=%u", whio_dev_size(dev) );
    nid = dev->api->write( dev, "Stop saying that!!", 18 );
    total += nid;
    WHEFS_DBG("dev size=%u", whio_dev_size(dev) );
    dev->api->finalize(dev);

    F = whefs_fopen( fs, fname, "r+" );
    assert( F && "re-open of inode failed!" );
    dev = whefs_fdev( F );
    char const * str = "...And now a final word.";
    size_t slen = strlen(str);
    size_t dsize = whefs_fseek( F, 0, SEEK_END );
    WHEFS_DBG("F size=%u, slen=%u", dsize, slen );
    nid = whefs_fwrite( F, slen, 1, str );
    //WHEFS_DBG("whefs_fwrite() rc=%u", nid );
    assert( (1 == nid) && "write failed!" );
    dev->api->seek( dev, dsize + slen + 10, SEEK_SET );
    dev->api->write( dev, "!", 1 );
    //nid = whefs_fwrite( F, 1, 1, "!" );
    WHEFS_DBG("dev size=%u", whio_dev_size(dev) );
    whefs_fclose( F );
#endif

    return rc;
}

int whefs_fs_stats_get( whefs_fs * fs, whefs_fs_stats * st )
{
    if( ! fs || ! st ) return whefs_rc.ArgError;
    WHEFS_DBG_ERR("NYI!");

    st->size = fs->filesize;
    // FIXME: calculate used nodes.
    st->used_inodes = 1; /* root node is always considered used. */
    // FIXME: calculate used blocks
    st->used_blocks = 0;
    // FIXME: calculate used bytes
    st->used_bytes = 0;

    return whefs_rc.OK;
}



int whefs_fs_dump_to_FILE( whefs_fs * fs, FILE * out )
{
    if( ! fs || !out || !fs->dev ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    enum { bufSize = (1024 * 4) };
    unsigned char buf[bufSize];
    size_t rlen = 0;
    fs->dev->api->seek( fs->dev, 0L, SEEK_SET );
    while( (rlen = fs->dev->api->read( fs->dev, buf, bufSize ) ) )
	//#undef bufSize
    {
	if( 1 != fwrite( buf, rlen, 1, out ) )
	{
	    rc = whefs_rc.IOError;
	    break;
	}
    }
    return rc;
}

int whefs_import_dev( whefs_fs * fs, whio_dev * src, char const * fname, bool overwrite )
{
    if( ! fs || !src || !fname || !*fname ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    whefs_inode ino = whefs_inode_empty;
    bool existed = false;
    rc = whefs_inode_by_name( fs, fname, &ino );
    if( rc == whefs_rc.OK )
    {
	if( ! overwrite ) return whefs_rc.AccessError;
	existed = true;
    }
    else
    {
	rc = whefs_inode_next_free( fs, &ino, true );
	if( whefs_rc.OK != rc ) return rc;
	rc = whefs_inode_name_set( fs, ino.id, fname );
	if( whefs_rc.OK != rc ) return rc;
	whefs_inode_flush( fs, &ino );
    }
    whio_dev * imp = whefs_dev_for_inode( fs, ino.id, true );
    if( ! imp ) return whefs_rc.InternalError;

    const whio_size_t oldPos = src->api->tell( src );
    const whio_size_t szrc = src->api->seek( src, 0, SEEK_SET );
    if( whio_rc.SizeTError == szrc )
    {
	imp->api->finalize( imp );
	if( ! existed )
	{
	    whefs_inode_unlink( fs, &ino );
	}
	return whefs_rc.RangeError;
    }

    const size_t oldFSize = ino.data_size;
    const size_t newFSize = whio_dev_size( src );
    rc = imp->api->truncate( imp, newFSize );
    if( rc != whefs_rc.OK )
    {
	WHEFS_DBG_ERR("Could not truncate '%s' to %u bytes!", fname, newFSize );
	imp->api->truncate( imp, oldFSize );
	src->api->seek( src, oldPos, SEEK_SET );
	imp->api->finalize( imp );
	if( ! existed )
	{
	    whefs_inode_unlink( fs, &ino );
	}
	return rc;
    }
    imp->api->flush( imp );
    if( 0 == newFSize )
    {
	//WHEFS_DBG("Importing 0-byte file.");
	src->api->seek( src, oldPos, SEEK_SET );
	imp->api->finalize(imp);
	return whefs_rc.OK;
    }
    enum { bufSize = 1024 * 8 };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );
    size_t wlen = newFSize;
    size_t totalR = 0;
    size_t totalW = 0;
    size_t rrc = 0;
    size_t wrc = 0;
    imp->api->seek( imp, 0, SEEK_SET );
    do
    {
	rrc = src->api->read( src, buf, bufSize );
	totalR += rrc;
	if( ! rrc ) break;
	if( (wlen - rrc) > wlen )
	{
	    WHEFS_DBG_ERR("Read an unexpected length (%u)! Continuing would cause an underflow!", rrc);
	    break;
	}
	wlen -= rrc;
	wrc = imp->api->write( imp, buf, rrc );
	//WHEFS_DBG("wrc=%u, rrc=%u, xoimp->tell=%u",wrc, rrc, imp->api->tell(imp));
	totalW += wrc;
	if( wrc != rrc ) break;
    }
    while( rrc && wrc );
    src->api->seek( src, oldPos, SEEK_SET );
    const size_t impSize = whio_dev_size( imp );
    imp->api->finalize(imp);
    imp = 0;

    rc = whefs_rc.OK;
    if( totalR != totalW )
    {
	WHEFS_DBG_ERR("Pseudofile [%s]: Total read bytes (%u) != total written bytes (%u)", fname, totalR, totalW );
	rc = whefs_rc.IOError;
    }
    else if( impSize != newFSize )
    {
	WHEFS_DBG_ERR("Pseudofile [%s]: Imported file size (%u) does not match the source's size (%u)", fname, totalR, totalW );
	rc = whefs_rc.IOError;
    }
    if( whefs_rc.OK != rc )
    {
	if( ! existed )
	{
	    whefs_inode_unlink( fs, &ino );
	}
    }
    return rc;
}

int whefs_fs_dump_to_filename( whefs_fs * fs, char const * filename )
{
    if( ! fs || !filename || !fs->dev ) return whefs_rc.ArgError;
    FILE * f = fopen( filename, "w+" );
    if( ! f ) return whefs_rc.AccessError;
    int rc = whefs_fs_dump_to_FILE( fs, f );
    fclose( f );
    return rc;
}

const whefs_fs_entry whefs_fs_entry_empty = whefs_fs_entry_empty_m;
int whefs_fs_entry_foreach( whefs_fs * fs, whefs_fs_entry_foreach_f func, void * foreachData )
{
    if( ! fs || !func ) return whefs_rc.ArgError;
    whefs_id_type i = 2;// skip root inode
    whefs_inode n = whefs_inode_empty;
    int rc = whefs_rc.OK;
    whefs_fs_entry ent = whefs_fs_entry_empty;
    enum { bufSize = whefs_sizeof_max_filename + 1 };
    char buf[bufSize];
    memset(buf,0,bufSize);
    ent.name.string = buf;
    ent.name.alloced = bufSize;
    for( ; i <= fs->options.inode_count; ++i )
    {
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
	if( fs->bits.i_loaded && !WHEFS_ICACHE_IS_USED(fs,i) )
	{
	    continue;
	}
#endif
	rc = whefs_inode_id_read( fs, i, &n );
	if( whefs_rc.OK != rc ) break;
	if( n.id != i )
	{
	    assert( 0 && "node id mismatch after whefs_inode_id_read()" );
	    WHEFS_FIXME("Node id mismatch after successful whefs_inode_id_read(). Expected %"WHEFS_ID_TYPE_PFMT" but got %"WHEFS_ID_TYPE_PFMT".", i, n.id );
	    rc = whefs_rc.InternalError;
            break;
	}
        if( !( n.flags & WHEFS_FLAG_Used ) ) continue;
        ent.inode_id = i;
        ent.mtime = n.mtime;
        ent.block_id = n.first_block;
        ent.size = n.data_size;
        rc = whefs_inode_name_get( fs, i, &ent.name );
        if( whefs_rc.OK != rc ) break;
        rc = func( fs, &ent, foreachData );
        if( whefs_rc.OK != rc ) break;
    }
    assert( (ent.name.string == buf) && "Internal error: illegal (re)alloc on string bytes!");
    return rc;
}


whefs_string * whefs_ls( whefs_fs * fs, char const * pattern, whefs_id_type * count  )
{
    // FIXME: reimplement in terms of whefs_fs_entry_foreach().
    if( ! fs ) return 0;
    const whefs_id_type nc = whefs_fs_options_get(fs)->inode_count;
    whefs_id_type id = 2; /* ID 1 is reserved for root node entry. */
    int rc = whefs_rc.OK;
    whefs_string * head = 0;
    whefs_string * str = 0;
    whefs_string * prev = 0;
    if( count ) *count = 0;
    whefs_string theString = whefs_string_empty;
    whefs_inode tmpn = whefs_inode_empty;
    for( ; id <= nc; ++id )
    {
#if WHEFS_CONFIG_ENABLE_BITSET_CACHE
	if( fs->bits.i_loaded && !WHEFS_ICACHE_IS_USED(fs,id) )
	{
	    continue;
	}
	//WHEFS_DBG("Cache says inode #%i is used.", i );
#endif
	rc = whefs_inode_name_get( fs, id, &theString );
	if( whefs_rc.OK != rc )
	{
	    WHEFS_DBG_ERR("whefs_inode_name_get() failed! rc=%d",rc);
	    whefs_string_clear( &theString, false );
	    return head;
	}
	if(0) WHEFS_DBG("Here. id=%"WHEFS_ID_TYPE_PFMT" str.len=%u str=[%s]",
			id, theString.length, theString.string);
	if( pattern && *pattern && !whglob_matches( pattern, theString.string ) )
	{
	    continue;
	}
        if(1)
        {// make sure it's marked as used, or skip it
#if 0
            uint32_t flagcheck = 0;
            whefs_inode_read_flags( fs, id, &flagcheck );
            if( ! (flagcheck & WHEFS_FLAG_Used) ) continue;
#else
            rc = whefs_inode_id_read( fs, id, &tmpn );
            if( whefs_rc.OK != rc ) break;
            if( ! (tmpn.flags & WHEFS_FLAG_Used) ) continue;
#endif
        }
	str = whefs_string_alloc();
	if( ! str ) return head;
	if( ! head ) head = str;
	*str = theString;
        theString = whefs_string_empty; // take over ownership of theString.string
	if( prev ) prev->next = str;
	prev = str;
	if( count ) ++(*count);
    }
    whefs_string_clear( &theString, false );
    return head;
}

