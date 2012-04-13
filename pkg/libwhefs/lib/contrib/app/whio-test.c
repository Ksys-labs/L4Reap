#ifdef NDEBUG
#  undef NDEBUG
#endif
#if !defined(_POSIX_C_SOURCE)
/* required for popen(), pclose(), maybe others */
#  define _POSIX_C_SOURCE 200112L
//#  define _POSIX_C_SOURCE 199309L
//#  define _POSIX_C_SOURCE 199506L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <wh/whio/whio.h>
#include <wh/whio/whio_encode.h>

#ifndef WHIO_ENABLE_ZLIB
#define WHIO_ENABLE_ZLIB 0
#endif

struct Application
{
    whio_stream * cout;
} ThisApp = {
0
};

#if 1
static void my_printfv(char const * fmt, va_list vargs )
{
    whio_stream_writefv( ThisApp.cout, fmt, vargs );
}
static void my_printf(char const * fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);
    my_printfv( fmt, vargs );
    va_end(vargs);
}
#define MARKER if(1) my_printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(1) my_printf
#else
static void noop_printf(char const * fmt, ...) {}
#define MARKER if(0) noop_printf
#endif


int test_iodev()
{
    MARKER("starting test\n");
    char const * fname = "foo.iodev";
    whio_dev * dev = 0;
    size_t szrc = 0;
    if(1)
    {
	dev = whio_dev_for_filename( fname, "w+" );
	assert( dev && "whio_dev open failed!");
	MARKER("Opened on-disk device file [%s].\n", fname );
	//dev->api->write( dev, "hi, world!", 9 );
	size_t wrc = whio_dev_encode_cstring( dev, "hi, world!", 0 );
	MARKER("wrc=%u\n",wrc);
	assert( (wrc == 10 + whio_sizeof_encoded_cstring) && "write failed!" );
	dev->api->seek( dev, whio_sizeof_encoded_cstring + 2, SEEK_SET );
	dev->api->write( dev, "!", 1 );
	szrc = dev->api->seek( dev, 99, SEEK_SET );
	MARKER( "seek-past-bounds rc=%u\n", szrc );
	dev->api->write( dev, "!", 1 );
	dev->api->finalize(dev);

	dev = whio_dev_for_filename( fname, "r" );
	assert( dev && "whio_dev open failed!");
	char * rstr = 0;
	size_t rslen = 0;
	whio_dev_decode_cstring( dev, &rstr, &rslen );
	assert( rstr && "Read of string failed!" );
	MARKER("Read string of %u bytes: [%s]\n", rslen, rstr );
	free(rstr);
	dev->api->finalize(dev);
    }

    short expandable = 0;
    for( ; expandable < 2; ++expandable )
    {
	MARKER("*********** Starting %sexpanding membuf tests.\n",expandable ? "" : "non-");
	const size_t ramSize = 100;
	const float expFactor = 1.9;
	whio_dev * ram = whio_dev_for_membuf( ramSize, expFactor );
	assert( ram && "creation of ram iodev failed!");
	MARKER("ram size = %u\n", whio_dev_size( ram ) );
	if( 1 ) szrc = ram->api->write( ram, "HI", 2 );
	else szrc = whio_dev_writef( ram, "H%c", 'I' );
	ram->api->seek( ram, 95, SEEK_SET );
	ram->api->truncate( ram, 90 );
	MARKER("ram pos = %u\n", ram->api->tell(ram) );
	MARKER("ram size = %u\n", whio_dev_size( ram ) );
	int i = 0;
	for( ; i < 4; ++i )
	{
	    szrc = whio_dev_writef( ram, "#%d", i );
	    MARKER("write rc = %u\n", szrc );
	    if( expandable )
	    {
		assert( (2 == szrc) && "Write failed to grow buffer!" );
	    }
	    else if( 2 != szrc )
	    {
		MARKER("Short write - breaking write loop at iteration #%d\n", i );
		break;
	    }
	}
	//ram->api->seek( ram, 30, SEEK_SET );
	//whio_dev_rewind( ram );
	MARKER("ram size = %u\n", whio_dev_size( ram ) );
#if 0	
	szrc = ram->api->write( ram, "part 1!", 7 );
	MARKER("write rc = %u\n", szrc );
	szrc = ram->api->write( ram, "part 2!", 7 );
	MARKER("write rc = %u\n", szrc );
	MARKER("ram size = %u\n", whio_dev_size( ram ) );
	MARKER("ram pos = %u\n", ram->api->tell(ram) );
#endif

	fname = expandable ? "ram-expandable.iodev" : "ram-fixed.iodev";
	dev = whio_dev_for_filename( fname, "w" );
	int rc = whio_dev_copy( ram, dev );
	dev->api->finalize(dev);
	assert( (rc == whio_rc.OK) && "whio_dev_copy() failed!" );
	MARKER("Memory buffer device written to file [%s].\n", fname );
	ram->api->finalize(ram);
    }

    MARKER("ending test\n");
    return 0;
}


int test_memmap()
{
    MARKER("starting test\n");
    char const * fname = "memmap.iodev";

    enum { ramSize = 100 };
    void * ramBuf = malloc( ramSize );
    memset( ramBuf, 0, ramSize );
    whio_dev * ram = whio_dev_for_memmap_rw( ramBuf, ramSize );
    assert( ram && "creation of memmap iodev failed!");
    MARKER("memmap size = %u\n", whio_dev_size( ram ) );

    int rc = 0;
    size_t ioctlsz = 0;
    rc = whio_dev_ioctl(ram, whio_dev_ioctl_BUFFER_size, &ioctlsz );
    MARKER("ioctl (rc=%d) says the buffer size is %u bytes\n", rc, ioctlsz );


    //ram->api->truncate( ram, 35 );
    ram->api->seek( ram, 30, SEEK_SET );
    MARKER("ram size = %u\n", whio_dev_size( ram ) );

    size_t szrc = 0;
    szrc = ram->api->write( ram, "part 1!", 7 );
    MARKER("write rc = %u\n", szrc );
    szrc = ram->api->write( ram, "part 2!", 7 );
    MARKER("write rc = %u\n", szrc );
    MARKER("ram size = %u\n", whio_dev_size( ram ) );
    MARKER("ram pos = %u\n", ram->api->tell(ram) );

    fname = "membuf.iodev";

    whio_dev * dev = whio_dev_for_filename( fname, "w" );
    rc = whio_dev_copy( ram, dev );
    assert( (rc == whio_rc.OK) && "whio_dev_copy() failed!" );
    MARKER("Memory buffer device written to file [%s].\n", fname );
    dev->api->finalize(dev);
    ram->api->finalize(ram);

    whio_dev * ro = whio_dev_for_memmap_ro( ramBuf, ramSize );
    ro->api->seek( ro, 30, SEEK_SET );
    char rbuf[ramSize];
    memset( rbuf, 0, ramSize );
    ro->api->read( ro, rbuf, 20 );
    MARKER("Read string: [%s]\n", rbuf );
    szrc = ro->api->write( ro, "nono", 4 );
    MARKER("write rc = %u\n",szrc);
    assert( (0 == szrc) && "Write should not have worked!" );
    ro->api->finalize(ro);
    free(ramBuf);
    MARKER("ending test\n");
    return 0;
}

enum {
whbits_prealloc_bytes = 8
};
#define BITSET_TYPE_DECLARE(LEN) typedef struct bitset_ # LEN \
{ \
    unsigned char bytes[LEN]; \
    size_t sz_bytes; \
    size_t sz_bits; \
} bitset_ # LEN;


int test_stuff()
{
    MARKER("Testing random stuff.\n");
    enum {
    Size = 5
    };
    unsigned char bytes[Size];
    //#define SET(BIT) (bytes[ (BIT / 8) ] |= (0x01 << (BIT%8)))
    //#define UNSET(BIT) (bytes[ BIT / 8 ] &= ~(0x01 << (BIT%8)))
    //#define GET(BIT) ((bytes[BIT / 8] & (0x01 << (BIT%8))) ? 0x01 : 0x00)

#define BS_ARRAY bytes
#define BS_SIZE_BYTES Size
#define BS_CLEAR memset( BS_ARRAY, 0, BS_SIZE_BYTES )
#define BS_BYTEFOR(BIT) (BS_ARRAY[ BIT / 8 ])
#define BS_SET(BIT) ((BS_BYTEFOR(BIT) |= (0x01 << (BIT%8))),0x01)
#define BS_UNSET(BIT) ((BS_BYTEFOR(BIT) &= ~(0x01 << (BIT%8))),0x00)
#define BS_GET(BIT) ((BS_BYTEFOR(BIT) & (0x01 << (BIT%8))) ? 0x01 : 0x00)
#define BS_TOGGLE(BIT) (BS_GET(BIT) ? BS_UNSET(BIT) : BS_SET(BIT))
    BS_CLEAR;
    size_t i = 0;
    for( ; i < Size; ++i )
    {
	printf("0x01 << %d = 0x%02x = %u\n", i, (0x01 << i), (0x01 << i) );
	printf("~(0x01 << %d) = 0x%02x = %u\n", i, ~(0x01 << i), ~(0x01 << i) );
    }
    BS_SET(0);
    BS_SET(1);
    BS_SET(4);
    BS_SET(5);
    BS_UNSET(5);
    BS_SET(8);
    for( i = 0; i < 10; ++i )
    {
	printf("BS_GET(%u) = %d", i, BS_GET(i) );
	printf("\tBS_TOGGLE(%u) = %d", i, BS_TOGGLE(i) );
	printf("\tBS_GET(%u) = %d\n", i, BS_GET(i) );
    }

    BS_UNSET(8);
    BS_SET(31);
    for( i = 0; i < Size; ++i )
    {
	printf("byte[%u]=0x%02x = %d\n", i, bytes[i], bytes[i] );
    }
    MARKER("End testing random stuff.\n");
    return 0;
}

int test_stream()
{
    MARKER("Starting stream test...\n");
    char const * fname = "streamdev.iodev";
    whio_dev * rw = whio_dev_for_filename( fname, "w+" );
    assert(rw);
    whio_stream * s = whio_stream_for_dev( rw, false );
    assert(s);
    MARKER("Streaming to file [%s]...\n", fname );

    size_t i = 0;
    whio_size_t szrc = 0;
    char digit = '0';
    for( ; i < 10; ++i, ++digit )
    {
	szrc = s->api->write( s, "test #", 6 );
        assert( szrc == 6 );
	//MARKER("Write length = %u\n", szrc );
	szrc = s->api->write( s, &digit, 1 );
        assert( szrc == 1 );
    }
    s->api->finalize(s);

    whio_dev_rewind( rw );
    s = whio_stream_for_dev( rw, true );

    enum {ramSize = 100};
    char buf[ramSize];
    memset(buf, 0, ramSize);
    const size_t expLen = 7;
    for( i = 0; i < 5; ++i )
    {
	szrc = s->api->read( s, buf, expLen );
	//MARKER("Read length = %u\n", szrc );
	assert( (expLen == szrc) && "Read failed!");
	buf[expLen+1] = 0;
	MARKER("Read: %"WHIO_SIZE_T_PFMT" bytes [%s]\n", szrc, buf );
    }
    s->api->finalize(s);

    MARKER("end stream test.\n");
    return 0;
}

int test_subdev()
{
    MARKER("Starting stream test...\n");
    char const * fname = "subdev.iodev";
    whio_dev * parent = whio_dev_for_filename( fname, "w+" );
    assert(parent);
    parent->api->write( parent, "!", 1 );
    parent->api->seek( parent, 99, SEEK_SET );
    parent->api->write( parent, "!", 1 );
    whio_dev * sub = whio_dev_subdev_create( parent, 10, 43 );
    assert(sub);
    int rc = whio_dev_subdev_rebound( sub, 60, 98 );
    assert( (whio_rc.OK == rc) );
    MARKER("Subdevice in place for file [%s]...\n", fname );

    size_t i = 0;
    whio_size_t szrc = 0;
    char digit = '0';
    for( ; i < 5; ++i, ++digit )
    {
	szrc = sub->api->write( sub, "0123456789", 10 );
	//MARKER("Write length = %u\n", szrc );
    }
    whio_dev_rewind( sub );
    enum { bufSize = 200 };
    char buf[bufSize];
    memset( buf, 0, bufSize );
    sub->api->seek( sub, 5, SEEK_SET );
    MARKER("sub->api->tell = %u\n", sub->api->tell( sub ) );
    szrc = sub->api->read( sub, buf, 10 );
    MARKER("Read %"WHIO_SIZE_T_PFMT" bytes: [%s]\n", szrc, buf );
    MARKER("sub->api->tell = %u\n", sub->api->tell( sub ) );
    sub->api->seek( sub, 5, SEEK_END );
    MARKER("sub->api->tell = %u\n", sub->api->tell( sub ) );
    MARKER("sub size = %u\n", whio_dev_size( sub ) );
    sub->api->finalize(sub);
    parent->api->finalize(parent);
    MARKER("end stream test.\n");
    return 0;
}

#include <wh/whio/whio_zlib.h>
#if WHIO_ENABLE_ZLIB
#include "zlib.h" // Z_DEFAULT_COMPRESSION
int test_gzip()
{
    MARKER("starting gzip test\n");
    whio_stream * src = 0;
    whio_stream * dest = 0;
    char const * outfile = "test-out.gz";
    int rc = 0;

    src = whio_stream_for_filename( __FILE__, "r" );
    dest = whio_stream_for_filename( outfile, "w" );
    assert( (src && dest) && "creation of streams failed!" );
    rc = whio_stream_gzip( src, dest, Z_DEFAULT_COMPRESSION );
    MARKER("gzip rc=%d\n", rc );
    src->api->finalize( src );
    dest->api->finalize( dest );
    MARKER("ending gzip test\n");

    src = whio_stream_for_filename( outfile, "r" );
    outfile = "test-out.ungz";
    dest = whio_stream_for_filename( outfile, "w" );
    assert( (src && dest) && "creation of streams failed!" );

    rc = whio_stream_gunzip( src, dest );
    src->api->finalize( src );
    dest->api->finalize( dest );
    MARKER("Decompress to file [%s]: rc=%d\n", outfile, rc );
    return rc;
}
#endif /* WHIO_ENABLE_ZLIB */

/** x must be-a (FILE*) opened by popen(). */
static void test_popen_dtor( void * x )
{
    int rc = pclose( (FILE*)x );
    MARKER("pclose(@0x%p) rc = %d\n", x, rc );
}
int test_popen()
{
    char const * cmd = "/bin/ls -ltd *.h *.c; echo rc=$?";
    FILE * p = popen(cmd, "r" );
    MARKER("Reading from popen(\"%s\") @0x%p...\n",cmd,p);
    assert( p && "popen() failed!");
    whio_stream * str = whio_stream_for_FILE( p, false );
    str->client.data = p;
    str->client.dtor = test_popen_dtor;
#if 0
    whio_size_t rdrc = 0;
    enum { bufSize = 2048 };
    unsigned char buf[bufSize+1];
    memset( buf, 0, bufSize );
    do
    {
        rdrc = str->api->read( str, buf, bufSize );
        if( ! rdrc ) break;
        buf[rdrc] = 0;
        my_printf("%s",buf);
    } while( bufSize == rdrc );
#else
    whio_stream_copy( str, ThisApp.cout );
#endif
    str->api->finalize(str);
    return 0;
}

#define TRY_MMAP 1

#if TRY_MMAP
#include <sys/mman.h>
typedef struct
{
    void * mem;
    whio_dev * fdev;
    int fileno;
    whio_size_t size;
}  MMapInfo;
static const MMapInfo MMapInfo_init = {NULL,NULL,0,0};

int reimpl_whio_dev_mmap_flush( whio_dev * dev )
{
    MARKER("flushing via msync...\n");
    MMapInfo * m = (MMapInfo *)dev->client.data;
    return msync( m->mem, m->size, MS_SYNC );
}

bool reimpl_whio_dev_mmap_close( whio_dev * dev )
{
    if( ! dev ) return false;
    MARKER("closing mmap()...\n");
    MMapInfo * m = (MMapInfo *)dev->client.data;
    assert(m && m->fdev && m->mem);
    if( m->mem )
    {
        dev->api->flush( dev );
        MARKER("munmap()...\n");
        munmap( m->mem, m->size );
    }
    if( m->fdev ) m->fdev->api->finalize(m->fdev);
    *m = MMapInfo_init;
    free(m);
    dev->client.data = 0;
    return whio_dev_api_memmap.close( dev );
}


/**
   Access a file via mmap() and a membuf whio_dev instance.
*/
int test_mmap()
{
    static whio_dev_api whio_dev_api_mmap = {0};
    if( 0 == whio_dev_api_mmap.read )
    {
        whio_dev_api_mmap = whio_dev_api_memmap;
        whio_dev_api_mmap.flush = reimpl_whio_dev_mmap_flush;
        whio_dev_api_mmap.close = reimpl_whio_dev_mmap_close;
    }

    char const * fname = "mmap.file";
    whio_dev * dev = whio_dev_for_filename( fname, "w+" );
    assert( dev );
    int fno = -1;
    int rc = whio_dev_ioctl( dev, whio_dev_ioctl_FILE_fd, &fno );
    assert( whio_rc.OK == rc );
    enum { bufSize = 10000 };
    char buf[bufSize];
    memset( buf, '*', bufSize );
    dev->api->truncate( dev, bufSize );
    dev->api->write( dev, buf, bufSize );
    whio_size_t dsz = whio_dev_size( dev );
    assert( dsz == bufSize );
    void * m = mmap( 0, dsz, PROT_WRITE, MAP_SHARED, fno, 0 );
    assert( m );
    whio_dev * md = whio_dev_for_memmap_rw( m, dsz );
    assert( md );
    md->api = &whio_dev_api_mmap;


    MMapInfo * minfo = (MMapInfo*)malloc(sizeof(MMapInfo));
    md->client.data = minfo;
    minfo->size = dsz;
    minfo->fdev = dev;
    minfo->fileno = fno;
    minfo->mem = m;

    whio_size_t i = 0;
    for( ; i < bufSize; i+=10 )
    {
        md->api->seek( md, (off_t)i, SEEK_SET );
        md->api->write( md, "!", 1 );
    }

    md->api->finalize(md);
    MARKER("Look for file [%s]\n",fname);
    return 0;
}

#endif // TRY_MMAP

int test_tar()
{
    // http://en.wikipedia.org/wiki/Tarball
    char const * fname = "iodev.tar";

    enum {
    TarHeaderOffsetName = 0,
    TarHeaderSizeName = 100,
    TarHeaderOffsetMode = TarHeaderOffsetName + TarHeaderSizeName,
    TarHeaderSizeMode = 8,
    TarHeaderOffsetOUID = TarHeaderOffsetMode + TarHeaderSizeMode,
    TarHeaderSizeOUID = 8,
    TarHeaderOffsetGUID = TarHeaderOffsetOUID + TarHeaderSizeOUID,
    TarHeaderSizeGUID = 8,
    TarHeaderOffsetSize = TarHeaderOffsetGUID + TarHeaderSizeGUID,
    TarHeaderSizeSize = 12,
    TarHeaderOffsetMtime = TarHeaderOffsetGUID + TarHeaderSizeGUID,
    TarHeaderSizeMtime = 12,
    TarHeaderOffsetChecksum = TarHeaderOffsetMtime + TarHeaderSizeMtime,
    TarHeaderSizeChecksum = 8,
    TarHeaderOffsetLinkType = TarHeaderOffsetChecksum + TarHeaderSizeChecksum,
    TarHeaderSizeLinkType = 1,
    TarHeaderOffsetLinkName = TarHeaderOffsetLinkType + TarHeaderSizeLinkType,
    TarHeaderSizeLinkName = 100,

    TarHeaderOffsetData = TarHeaderOffsetLinkName + TarHeaderSizeLinkName,
    TarHeaderSize = 512
    };

    enum {
    TestDataSize = 1024 * 2
    };
    const size_t dsize = TestDataSize;

    unsigned char buf[TarHeaderSize+1];
    memset( buf, 0, TarHeaderSize+1 );

    // Name:
    size_t slen = strlen(fname);
    memcpy( buf+TarHeaderOffsetName, fname, slen );

    // Mode:
    snprintf( (char *)(buf+TarHeaderOffsetMode), TarHeaderSizeMode+1, "%08o", 0644 );
    buf[TarHeaderOffsetMode + TarHeaderSizeMode-1] = ' ';

    // Size:
    snprintf( (char *)(buf+TarHeaderOffsetSize), TarHeaderSizeSize+1, "%011o", dsize );
    buf[TarHeaderOffsetSize + TarHeaderSizeSize-1] = ' ';
    slen = strlen((char *)(buf+TarHeaderOffsetSize));
    printf("size %"WHIO_SIZE_T_PFMT" octal=[%s] slen=[%u]\n",dsize, buf+TarHeaderOffsetSize,slen );

    snprintf( (char *)(buf+TarHeaderOffsetMtime), TarHeaderSizeMtime+1, "%011o", (60*60)*24*3+1 /*3rd Jan 1972*/ );
    buf[TarHeaderOffsetMtime + TarHeaderSizeMtime-1] = ' ';

    // Link. Link name left blank.
    buf[TarHeaderOffsetLinkType] = '0';

    // User/Group:
#if 0
    char const * empty = "  nobody"; // exactly 8 bytes
    memcpy( buf+TarHeaderOffsetOUID, empty, TarHeaderSizeOUID );
    memcpy( buf+TarHeaderOffsetGUID, empty, TarHeaderSizeGUID );
#else
    char const * empty = "00000000"; // exactly 8 bytes
    memcpy( buf+TarHeaderOffsetOUID, empty, TarHeaderSizeOUID );
    memcpy( buf+TarHeaderOffsetGUID, empty, TarHeaderSizeGUID );
#endif
    // Checksum:
    memset( buf+TarHeaderOffsetChecksum, ' ', TarHeaderSizeChecksum );

    uint32_t csum = 0U;
    unsigned char * x = buf;
    unsigned int i = 0;
    for( ; i < TarHeaderSize; ++i, ++x )
    {
        csum += *x;
    }
    snprintf( (char *)(buf+TarHeaderOffsetChecksum), TarHeaderSizeChecksum, "%06o0", csum );
    buf[TarHeaderOffsetChecksum + TarHeaderSizeChecksum -1] = ' ';
#if 1
    //memset( buf + TarHeaderOffsetData, '*', TarHeaderSize - TarHeaderOffsetData );
    x = buf;

    for( i = 0 ; i < TarHeaderSize; ++i, ++x )
    {
        if( !*x ) *x = '*';
    }
    puts((char *)buf);
#else
    fwrite( buf, TarHeaderSize, 1, stderr );
#endif

#if 0
    unsigned char data[TestDataSize];
    memset( data, '*', TestDataSize );
    fwrite( buf, TarHeaderSize, 1, stderr );
    fwrite( data, TestDataSize, 1, stderr );
#endif


#if 0
    whio_dev * dev = whio_dev_for_filename( fname, "w+");
    dev->api->truncate( dev, TarHeaderSize );
    dev->api->seek( dev, 0L, SEEK_SET );
    dev->api->write( dev, fname, slen ); 
    dev->api->seek( dev, 0L, SEEK_SET );
    dev->api->finalize(dev);
#endif
    return 0;
}
#define TEST_WHIO_ENCODE 0

#if TEST_WHIO_ENCODE
whio_size_t whio_encode_fv( void * dest, char const * fmt, va_list va );
whio_size_t whio_encode_f( void * dest, char const * fmt, ... );

int test_encode()
{
    whio_dev * dev = 0;
    char const * fname = "encode.iodev";
    dev = whio_dev_for_filename( fname, "w+" );
    enum { BufSize =
#define ESZ(N) whio_sizeof_encoded_uint##N
           ESZ(8)
           + ESZ(16)
           + ESZ(32)
           + ESZ(64)
#undef ESZ
    };
    uint8_t a1 = 33;
    uint16_t a2 = a1;
    uint32_t a3 = a2;
    uint64_t a4 = a3;
    unsigned char buf[BufSize];
    memset( buf, BufSize, 0 );
    whio_size_t erc = whio_encode_f( buf, "csil", a1, a2, a3, a4 );
    assert( (BufSize == erc) && "Unexpected result from whio_encode_f()!" );
    MARKER("BufSize=%d erc=%"WHIO_SIZE_T_PFMT"\n",BufSize,erc);
    dev->api->write( dev, buf, erc );
    dev->api->finalize(dev);
    MARKER("Encoding test output file is %s.\n",fname);
    return 0;
}
#endif //TEST_WHIO_ENCODE

void my_atexit()
{
    if( ThisApp.cout )
    {
	ThisApp.cout->api->finalize( ThisApp.cout );
	ThisApp.cout = 0;
    }
}

int main( int argc, char const ** argv )
{
    atexit(my_atexit);
    //ThisApp.cout = whio_stream_for_FILE( stdout, false );
    ThisApp.cout = whio_stream_for_fileno( 1, true );
    int rc = 0;
    if(!rc) rc =  test_iodev();
    if(!rc) rc =  test_memmap();
    if(!rc) rc =  test_stream();
    if(!rc) rc =  test_subdev();
#if WHIO_ENABLE_ZLIB
    if(!rc) rc =  test_gzip();
#endif
    if(!rc) rc =  test_stuff();
    if(!rc) rc =  test_popen();
    //if(!rc) rc =  test_tar();
#if TRY_MMAP
    if(!rc) rc =  test_mmap();
#endif
#if TEST_WHIO_ENCODE
    if(!rc) rc =  test_encode();
#endif
    printf("Done rc=%d=[%s].\n",rc,
	   (0==rc)
	   ? "You win :)"
	   : "You lose :(");
    return rc;
}
