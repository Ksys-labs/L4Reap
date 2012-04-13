/*
  Test/demo app for libwhefs.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
//#include <unistd.h> /* sleep() */
#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_client_util.h>
#include <wh/whio/whio_encode.h>
#include "WHEFSApp.c"
#if 0
#if 1
#define MARKER if(1) printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(1) printf
#else
static void bogo_printf(char const * fmt, ...) {}
#define MARKER if(0) bogo_printf
#endif
#endif


struct app_data {
    whefs_fs_options fsopts;
} ThisApp;

typedef struct
{
    size_t pos;
} ForEachData;
static const ForEachData ForEachDataInit = {0};

int fs_entry_foreach( whefs_fs * fs, whefs_fs_entry const * ent, void * data )
{
    ForEachData * fd = (ForEachData *)data;
    if( 1 == ++fd->pos )
    {
        printf("%-16s%-16s%-12s%-16s%-16s\n",
               "Node ID:","First block:", "Size:", "Timestamp:","Name:" );
    }
    printf("%-16"WHEFS_ID_TYPE_PFMT"%-16"WHEFS_ID_TYPE_PFMT"%-12u%-16u%s\n",
           ent->inode_id,
           ent->block_id,
           ent->size,
           ent->mtime,
           ent->name.string );

    return whefs_rc.OK;
}

void do_foreach( whefs_fs * fs )
{
    ForEachData d = ForEachDataInit;
    whefs_fs_entry_foreach( fs, fs_entry_foreach, &d );
}


int test_one()
{
    MARKER("starting test\n");
    char const * fname = "bogo.whefs";
#if 0
    char const * fmode = "r+";
    FILE * ff = fopen(fname,fmode);
    MARKER("fopen(%s,%s) = %p\n",fname,fmode,(void const *)ff);
    if( ff ) fclose(ff);
#endif
    whefs_fs * fs = 0;
    int rc = whefs_mkfs( fname, &ThisApp.fsopts, &fs );
    MARKER("mkfs() rc = %d\n", rc);
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    MARKER("mkfs() worked.\n");

    int irc = whefs_rc.OK;
    //irc = whefs_test_insert_dummy_files( fs );
    assert( (whefs_rc.OK == irc) && "insertion of dummy data failed!" );

    whefs_fs_dump_info( fs, stdout );
    whefs_fs_finalize( fs );
    fs = 0;

#if 1
    MARKER("Closed FS. Re-opening it...\n");
    if( whefs_rc.OK != whefs_openfs( fname, &fs, 0 ) )
    {
	MARKER("openfs(%s) failed!\n",fname);
	return 1;
    }
    whefs_fs_dump_info( fs, stdout );
    whefs_fs_finalize( fs );
#endif

    MARKER("ending test\n");
    return 0;
}

int test_ramfs()
{
    MARKER("starting ramvfs test\n");
    char const * fname = ":memory:";
    whefs_fs * fs = 0;
    size_t rc = whefs_mkfs( fname, &ThisApp.fsopts, &fs );
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    MARKER("mkfs() worked.\n");
    whefs_fs_dump_info( fs, stdout );

    int irc = whefs_test_insert_dummy_files( fs );
    assert( (whefs_rc.OK == irc) && "insertion of dummy data failed!" );

    fname = "fopen_test.file";
    whefs_file * F = whefs_fopen( fs, fname, "r+" );
    assert(F && "whefs_fopen() failed");
    MARKER("F (read-write) == 0x%p [%s]\n", (void const *)F, whefs_file_name_get(F) );
    whio_dev * FD = whefs_fdev( F );
    size_t i = 0;
    for( ; i < 15; ++i )
    {
	whefs_fwritef(F, "Entry #%u ", i);
	//whio_dev_writef( FD, "Entry #%u ", i);
    }
    whio_dev_rewind( FD );

    enum { bufSize = 256 };
    char buf[bufSize];
    memset( buf, 0, bufSize );
    FD->api->seek( FD, 125, SEEK_SET );
    rc = FD->api->read( FD, buf, 40 );
    MARKER("FD read rc=%u\n", rc );
    if( rc )
    {
	MARKER("Read bytes: [%s]\n", buf );
    }

    whefs_fclose( F );
#if 1
    F = whefs_fopen( fs, fname, "r" );
    MARKER("F (read-only) == 0x%p\n", (void const *)F );
    assert(F && "whefs_fopen() failed");
    whefs_fclose( F );
#endif
    fname = "memory.whefs";
    int drc = whefs_fs_dump_to_filename( fs, fname );
    if( whefs_rc.OK != drc )
    {
	MARKER("dump failed with rc %d\n", drc );
	assert( 0 && "fs dump failed" );
    }
    MARKER("Dumped in-memory vfs to [%s]\n",fname);

#if 1
    drc = whefs_fs_append_blocks( fs, 5 );
    if( whefs_rc.OK != drc )
    {
	MARKER("append blocks failed with rc %d\n", drc );
	assert( 0 && "append blocks failed" );
    }
    fname = "memory-2.whefs";
    drc = whefs_fs_dump_to_filename( fs, fname );
    if( whefs_rc.OK != drc )
    {
	MARKER("dump failed with rc %d\n", drc );
	assert( 0 && "fs dump failed" );
    }
    MARKER("Dumped added-blocks in-memory vfs to [%s]\n",fname);
#endif


    whefs_id_type lscount = 0;
    char const * lspat = "*f*";
    whefs_string * ls = whefs_ls( fs, lspat, &lscount );
    whefs_string * head = ls;
    size_t count = 0;
    MARKER("VFS File list matching '%s':\n",lspat);
    while( ls )
    {
        ++count;
	printf("\t%s\n", ls->string );
	ls = ls->next;
    }
    whefs_string_finalize( head, true );
    assert( (2 == count) && "Unexpected result count!");

    whefs_fs_finalize( fs );
    MARKER("ending test\n");
    return 0;
}

int test_multiple_files()
{
    MARKER("starting inode opener tests\n");
    char const * fname = "multiples.whefs";
    whefs_fs * fs = 0;
    int rc = whefs_mkfs( fname, &ThisApp.fsopts, &fs );
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    MARKER("Created vfs file [%s]\n",fname);

    fname = "one.bogo";
    whefs_file * F = 0;

#if 1
    F = whefs_fopen( fs, fname, "r+" );
    MARKER("F (read-write) == 0x%p\n", (void const *)F );
    assert(F && "whefs_fopen() failed");
#endif
    fname = "one-renamed.bogo";
    rc = whefs_file_name_set( F, fname );
    assert( (whefs_rc.OK == rc) && "file rename failed!");

#if 1
    whefs_file * F2 = 0;
    {
	F2 = whefs_fopen( fs, fname, "r+" );
	assert( !F2 && "F2 is correctly null!");
	MARKER("Second attempt to open read/write correctly failed.\n");
    }

    MARKER("Starting writes...\n");
    whio_dev * FD = 0;
    {
	FD = whefs_fdev( F );
	size_t i = 0;
	for( ; i < 150; ++i )
	{
	    whefs_fwritef(F, "Entry #%u ", i);
	    //whio_dev_writef( FD, "Entry #%u ", i);
	}
	whio_dev_rewind( FD );
    }
    MARKER("Writes done.\n");
    //FD->api->flush(FD);
#endif

#if 1
    MARKER("Attempting to open concurrently but read-only...\n");
    F2 = whefs_fopen( fs, fname, "r" );
    assert( F2 && "fopen for read-only mode failed!");
    whio_dev * FD2 = whefs_fdev( F2 );
    enum { bufSize = 256 };
    char buf[bufSize];
    memset( buf, 0, bufSize );
    const size_t dPos = 125;
    MARKER("FD->size=%"WHIO_SIZE_T_PFMT", FD2->size=%"WHIO_SIZE_T_PFMT"\n",
	   whio_dev_size( FD ),
	   whio_dev_size( FD2 ) );
    FD->api->seek( FD, dPos + 2, SEEK_SET );
    FD->api->write( FD, "X X X", 5 );
    FD2->api->seek( FD2, dPos, SEEK_SET );
    rc = FD2->api->read( FD2, buf, 40 );
    MARKER("FD2 read rc=%u\n", rc );
    if( rc )
    {
	MARKER("Read bytes: [%s]\n", buf );
    }
    whefs_fclose( F2 );
#endif



    do_foreach(fs);

    whefs_fs_setopt_autoclose_files( fs, true );
#if 0//set this to 0 to test fs' close-on-shutdown anti-leak check:
    whefs_fclose( F );
#else
    MARKER("You may see a warning next regarding unclosed objects. This is to test a memory leak.\n");
#endif

    whefs_fs_finalize( fs );
    MARKER("ending test\n");
    return 0;
}

int test_truncate()
{
    MARKER("starting truncate tests\n");
    char const * fname = "truncate.whefs";
    whefs_fs * fs = 0;
    int rc = whefs_mkfs( fname, &ThisApp.fsopts, &fs );
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    MARKER("Created vfs file [%s]\n",fname);

    fname = "one.bogo";
    whefs_file * F = 0;

    F = whefs_fopen( fs, fname, "r+" );
    MARKER("F (read-write) == 0x%p\n", (void const *)F );
    assert(F && "whefs_fopen() failed");
    whio_dev * FD = 0;
    {
	FD = whefs_fdev( F );
	size_t i = 0;
	for( ; i < 15; ++i )
	{
	    //whefs_fwritef(F, "Entry #%u ", i);
	    whio_dev_writef( FD, "Entry #%u ", i);
	}
	whio_dev_rewind( FD );
    }
    size_t truncSize = 100;
    whefs_ftrunc( F, truncSize );
    assert( whio_dev_size( FD ) == truncSize );
    const size_t step = 10;
    whefs_fseek( F, truncSize + step, SEEK_SET );
    FD->api->write( FD, "hi", 2 );
    assert( whio_dev_size( FD ) == (2 + step + truncSize) );
    whefs_fclose( F );
    FD = 0;

    fname = "file2";
    F = whefs_fopen( fs, fname, "r+" );
    assert( F );
    whefs_file_write( F, "hi hi", 5 );
    truncSize = 6000;
    rc = whefs_ftrunc( F, truncSize );
    assert( whefs_fsize( F ) == truncSize );
    truncSize = 2050;
    rc = whefs_ftrunc( F, truncSize );
    assert( whefs_fsize( F ) == truncSize );
    whefs_fseek( F, truncSize, SEEK_SET );
    whefs_file_write( F, "!!!", 3 );
    assert( whefs_fsize( F ) == (3 + truncSize) );
    whefs_fclose(F);

    do_foreach(fs);
    whefs_fs_finalize( fs );
    MARKER("ending test\n");
    return 0;
}



int bogo()
{
    MARKER("whefs_encoded_sizeof_id_type=%d\n",whefs_sizeof_encoded_id_type);
    uint64_t i6 = UINT64_C(0x3F01020304);
    MARKER("%016llx\n",(i6 << 8) );
    MARKER("%016llx\n",(i6 >>32) );
    MARKER("%016llx\n",(i6 >>32) << 40 );
    return -1;
}

#include "../src/whefs_encode.h"
#include <assert.h>
int test_encodings()
{
    enum { bufSize = 30 };
    unsigned char buf[bufSize];
    memset( buf, 0, bufSize );

    uint16_t ui = 27;
    size_t sz = whio_encode_uint16( buf, ui );
    MARKER("sz=%u\n",sz);
    MARKER("Buffer bytes=");
    size_t i = 0;
    for( i = 0; i < 5; ++i )
    {
	printf("#%u=[0x%02x]\t", i, buf[i] );
    } puts("");
    uint16_t uid = (uint16_t)-1;
    int rc = whio_decode_uint16( buf, & uid );
    MARKER("rc=%d uid=%"PRIu16"\n", rc, uid );
    MARKER("Buffer bytes=");
    for( i = 0; i < 5; ++i )
    {
	printf("#%u=[0x%02x]\t", i, buf[i] );
    } puts("");
    assert( uid == ui );
    return 0;
}

#include "../src/whefs_hash.h"
int test_hash()
{
    whefs_hashid_list * li = 0;
    whefs_hashid_list_alloc( &li, 10 );
    MARKER("hash list sizes: alloced=%"WHEFS_ID_TYPE_PFMT", count=%"WHEFS_ID_TYPE_PFMT"\n",
           li->alloced, li->count );
    whefs_hashid h = whefs_hashid_empty;

    whefs_id_type i;

#define HASH(X) ((whefs_hashval_type)(X*11))
#define SETH(X) h.hash = HASH(X); h.id = X; whefs_hashid_list_add( li, &h )

    for( i = 10; i >= 1; --i ) { SETH(i); }

    MARKER("hash list sizes: alloced=%"WHEFS_ID_TYPE_PFMT", count=%"WHEFS_ID_TYPE_PFMT"\n",
           li->alloced, li->count );
#if 0
    SETH(9);
    MARKER("hash list sizes: alloced=%"WHEFS_ID_TYPE_PFMT", count=%"WHEFS_ID_TYPE_PFMT"\n",
           li->alloced, li->count );
#endif


#define DUMPLIST(HEADER)                                                \
    printf("%s\n\tlist.count = %"WHEFS_ID_TYPE_PFMT", alloced=%"WHEFS_ID_TYPE_PFMT"\n",\
           HEADER, li->count,li->alloced);                              \
    for( i = 0; i < li->count; ++i )                      \
    { \
        whefs_hashid const * H = &li->list[i]; \
        printf("\tndx[%"WHEFS_ID_TYPE_PFMT"]=hash[%"WHEFS_HASHVAL_TYPE_PFMT"]=id[%"WHEFS_ID_TYPE_PFMT"] hits=[%"PRIu16"]\n", i, H->hash, H->id, H->hits ); \
    }
    DUMPLIST("Unsorted list...");
    whefs_hashid_list_sort( li );
    DUMPLIST("Sorted list...");

    whefs_hashval_type hashval;
    whefs_id_type ndx;
    ndx = 0; hashval = HASH(9); // avoid unused var warnings.
#define SEARCH(X) hashval = HASH(X); \
    ndx = whefs_hashid_list_index_of( li, hashval );                  \
    MARKER("search for hash [%"WHEFS_HASHVAL_TYPE_PFMT"] = ndx [%"WHEFS_ID_TYPE_PFMT"] ",hashval,ndx); \
    if( whefs_rc.IDTypeEnd != ndx ) \
        printf("id=[%"WHEFS_ID_TYPE_PFMT"], hits=[%"PRIu16"]",\
               li->list[ndx].id,li->list[ndx].hits);    \
    putchar('\n');


#if 0
    SETH(9); SETH(9); SETH(9);
    SETH(3); SETH(3); SETH(3);
    SETH(4);
    whefs_hashid_list_sort( li );
    //DUMPLIST("Sorted list with several duplicates...");
    SEARCH(9);
    SEARCH(10);
    SEARCH(14);
    SEARCH(2);
#endif

#if 0
    whefs_hashid_list_add_slots( li, li->count-1, 4 );
    DUMPLIST("Shifted list...");
    //whefs_hashid_list_sort( li );
#endif
    SEARCH(1);
    SEARCH(4);
    SEARCH(9);
    SEARCH(2);

    DUMPLIST("BEFORE erase()'d list...");;
    enum { countTo = 6 };

#define DUMPSEARCH(HEADER) \
    MARKER("%s\n",HEADER); \
    for( i = 1; i <= countTo; ++i ) \
    { SEARCH(i); }

    for( i = 1; i <= countTo; ++i )
    {
        if( ! (i%2) )
        {
            whefs_hashid_list_wipe_index( li, i );
        }
    }
    DUMPLIST("AFTER erase()'d list...");;
    //DUMPSEARCH("AFTER erase()'d list...");
    whefs_hashid_list_sort( li );
    DUMPLIST("after sorting erase()'d list...");;

#if 1
    for( i = 3; i <= countTo+1; ++i )
    {
        if( (i % 2) )
        {
            SETH(i);
        }
    }
    SETH(13);
    DUMPLIST("After re-inserting half of them:");
    whefs_hashid_list_sort( li );
    //DUMPLIST("After erasing a few and adding back half of them...");
    for( i = 3; i < (li->count/2); ++i )
    {
        whefs_hashid_list_index_of( li, HASH(i) );
        whefs_hashid_list_index_of( li, HASH(i) );
        whefs_hashid_list_index_of( li, HASH(i) );
    }
    DUMPLIST("After sorting again:");
    DUMPSEARCH("Search after re-adding:");
#endif

    whefs_hashid_list_chomp_lv( li );
    DUMPLIST("After chomping least-visited:");

#undef SEARCH
#undef SETH
#undef HASH
#undef DUMPLIST

    MARKER("whefs_hashid_list_sizeof() = %u\n",whefs_hashid_list_sizeof(li));

    whefs_hashid_list_free( li );

    return 0;
}

extern void whefs_inode_hash_cache_sort(whefs_fs * fs );
int test_caching()
{
    MARKER("Caching tests...\n");
    const bool toFile = true; /* using a file is many orders of magnitude slower than in-memory (300x on my box at home, but only 10-20x on some very fast drives). */
    whefs_fs * fs = 0;
    char const * const defaultFile = "caching.whefs";
    char const * fname = toFile
        ? defaultFile
        : ":memory:"
        ;

    /**
       Reminder to self:

       To test the block-boundary-is-size stuff, use: loops=20,
       count=15, bufSize=1024. We end up with pfiles where block size
       is exactly 1/2 the file size, allowing us to test corner cases
       where the pfile size is exactly the block size (or a multiple
       of it).
    */
    enum { count = 10, xpos = 4 /*see pname var*/, loops = 10 };
    assert( (count<=26) && "count must be less than 27!" );

    enum { bufSize = 1024*4 };
    char buf[bufSize] = {0};

    whefs_fs_options fopt = whefs_fs_options_default;
    fopt.block_size = (bufSize*(loops+1))/2;
    fopt.inode_count = 200;
    fopt.block_count = fopt.inode_count;
    fopt.filename_length = 64;
    int rc = whefs_mkfs( fname, &fopt, &fs );
    MARKER("mkfs([%s]) rc=%d\n",fname,rc);
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    whefs_fs_dump_info( fs, stdout );
    MARKER("mkfs([%s]) worked.\n",fname);

    const bool useHashCache = false;
    rc = whefs_fs_setopt_hash_cache( fs, useHashCache, false );
    MARKER("%s inode hash cache.\n", useHashCache ? "Enabling" : "Disabling");

#if 1
    char pname[xpos+2] = {'f','i','l','e',0/*index xpos*/,0};
    MARKER("looping %d times of over %d pseudofiles...\n",loops,count);
    size_t objCount = 0;
    whefs_file * files[count] = {0};
    size_t writeCount = 0;
    whio_size_t szck = 0U;
    int L;
    for( L = 1; L <= loops; ++L )
    {
        printf("#%d=[",L);
        char digit = 'A';
	int i;
        for( i = 0; i < count; ++i, ++digit )
        {
            pname[xpos] = digit;
            memset( buf, digit, bufSize );
            putchar('o');
            whefs_file * f = whefs_fopen( fs, pname, "r+" );
            assert( f && "fopen failed :(");
            files[i] = f;
            ++objCount;
            whio_dev * fd = whefs_fdev(f);
            fd->api->seek(fd,0L,SEEK_END);
            szck = fd->api->write( fd, buf, bufSize );
            assert( (szck==bufSize) && "write didn't return the expected result!");
            putchar('w');
            ++writeCount;
        }
        //if( 1 == L ) whefs_inode_hash_cache_sort(fs);
        for( i = 0; i < count; ++i )
        {
            putchar('c');
            whefs_file * f = files[i];
            assert( f && "files[i] is null!");
            whefs_fclose(f);
            files[i] = 0;
        }
        printf("] ");
        fflush(stdout);
    }
    printf("\n%u whefs_file objects were opened and closed, doing a total of %u writes of %u bytes each.\n",objCount,writeCount,bufSize);
    //whefs_inode_hash_cache_chomp_lv(fs);
#endif
    do_foreach(fs);
    if( ! toFile )
    {
        fname = defaultFile;
        whefs_fs_dump_to_filename(fs, fname);
        MARKER("Saved to [%s].\n",fname);
    }
    whefs_fs_finalize( fs );
    MARKER("End caching tests.\n");
    return 0;
}


int test_streams()
{
    whefs_fs_options fopt = whefs_fs_options_default;
    whefs_fs * fs = 0;
    char const * fname = "streams.whefs";
    int rc = whefs_mkfs( fname, &fopt, &fs );
    MARKER("mkfs([%s]) rc=%d\n",fname,rc);
    assert((rc == whefs_rc.OK) && "mkfs failed :(" );
    whefs_fs_dump_info( fs, stdout );
    MARKER("mkfs([%s]) worked.\n",fname);

    char const * pfile = "astream.out";

    whio_stream * out = whefs_stream_open( fs, pfile, true, false );
    assert(out);
    char const * str = "Hi, world!";
    size_t slen = strlen(str);
    out->api->write( out, str, slen );
    out->api->finalize(out);
    do_foreach(fs);
    out = whefs_stream_open( fs, pfile, true, true );
    out->api->write( out, str, slen );
    out->api->flush(out);
#if 0
    out->api->finalize(out);
#else
    MARKER("You may see a warning next regarding unclosed objects. This is to test a memory leak.\n");
#endif

    whio_dev * dev = whefs_dev_open( fs, pfile, false );
    assert(dev);
    assert( (2*slen) == whio_dev_size(dev) );
    dev->api->finalize( dev );

    do_foreach(fs);

    whefs_fs_finalize(fs);
    return 0;
}

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags]";
    WHEFSApp.helpText =
	"Basic test/sanity-check program for whefs."
	;
    int rc = 0;
    WHEFSApp_init( argc, argv, WHEFSApp_NoOpen, 0, 0 ); // ignore return.
    ThisApp.fsopts = whefs_fs_options_default;
    ThisApp.fsopts.inode_count = 16;
    ThisApp.fsopts.block_count = 32;
    ThisApp.fsopts.filename_length = 32;
    ThisApp.fsopts.block_size = 1024 * 2;
    //if(!rc) rc =  test_encodings();
    //if(!rc) rc =  test_hash();
    //if(!rc) rc = test_one();
    //if(!rc) rc =  test_ramfs();
    //if(!rc) rc =  test_multiple_files();
    //if(!rc) rc = test_streams();
    //if(!rc) rc =  test_truncate();
    if(!rc) rc =  test_caching();
    printf("Done rc=%d=[%s].\n",rc,
	   (0==rc)
	   ? "You win :)"
	   : "You lose :(");
    return rc;
}
