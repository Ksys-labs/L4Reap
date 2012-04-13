/*
  A demonstration file for libwhefs. It tries to embed a VFS within
  another VFS.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wh/whefs/whefs.h>
#include <wh/whio/whio_devs.h>



#if 1
#define MARKER if(1) printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(1) printf
#else
static void bogo_printf(char const * fmt, ...) {}
#define MARKER if(0) bogo_printf
#endif


#include <ctype.h> /* isdigit() */

//#include "whefs_details.c"

int main( int argc, char const ** argv )
{
    whefs_setup_debug( stderr, (unsigned int)-1 );
    char const * hostFile = "my.whefs";
    whefs_fs * host = 0;
    whefs_openfs( hostFile, &host, true );
    if( ! host )
    {
	printf("Required file [%s] not found (or not readable)! "
	       "Try creating it with: "
	       "/path/to/whefs-mkfs %s\n",
	       hostFile, hostFile );
	return 1;
    }
    assert( host );

    char const * guestFile = "guest.whefs";
    whefs_file * gf = whefs_fopen( host, guestFile, "r+" );
    assert( gf && "whefs_fopen() failed" );
    whio_dev * gdev = whefs_fdev( gf );

    whefs_fs_options opt = WHEFS_FS_OPTIONS_INIT(4096, 16, 20);

    whefs_fs * gfs = 0;
    int rc = whefs_mkfs_dev( gdev, &opt, &gfs, false );
    assert( whefs_rc.OK == rc );
    whefs_fs_dump_to_filename( gfs, guestFile );

    whefs_file * g2f = whefs_fopen( gfs, "a_guest_file!", "r+" );
    assert( g2f );
    whefs_fwrite( g2f, 1, 8, "embedded" );
    whefs_fclose( g2f );
    
    enum { bufSize = 200 };
    char buf[bufSize];
    memset( buf, 0, bufSize );
    g2f = whefs_fopen( gfs, "a_guest_file!", "r" );
    assert( g2f );
    gdev = whefs_fdev( g2f );
    gdev->api->read( gdev, buf, bufSize );
    MARKER("Read back: [%s]\n", buf );

    whefs_fclose( g2f );
    whefs_fs_finalize( gfs );
    whefs_fclose( gf );
    whefs_fs_finalize( host );

    puts("Done!");
    return 0;
}
