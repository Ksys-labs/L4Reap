/*
  A demonstration file for libwhefs. It uses static memory as
  the back-end for an EFS.

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
#include <wh/whefs/whefs_string.h>
#include <wh/whefs/whefs_client_util.h>



#if 1
#define MARKER if(1) printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); if(1) printf
#else
static void bogo_printf(char const * fmt, ...) {}
#define MARKER if(0) bogo_printf
#endif


#include <ctype.h> /* isdigit() */

/* StaticWHEFS.c is created by the build process. */
#include "StaticWHEFS.c"


int main( int argc, char const ** argv )
{
    whefs_setup_debug( stderr, (unsigned int)-1 );
    whefs_fs * fs = StaticWHEFS_whefs_open( true );
    assert( fs && "openfs failed!" );
    int rc = whefs_rc.OK;
    whefs_id_type lscount = 0;
    char const * lspat = "*";
    whefs_string * ls = whefs_ls( fs, lspat, &lscount );
    whefs_string * head = ls;
    MARKER("EFS File list matching '%s':\n",lspat);
    char const * openMe = 0;
    while( ls )
    {
	printf("\t%s\n", ls->string );
	if( ! openMe ) openMe = (char const *)ls->string;
	ls = ls->next;
    }
    assert( openMe && "didn't find a file to open!");
    whefs_file * fi =  whefs_fopen( fs, openMe, "r+" );
    whefs_string_finalize( head, true );
    assert( fi && "fopen failed!" );
    whio_dev * fdev = whefs_fdev( fi );

    fdev->api->seek( fdev, 30, SEEK_SET );
    char const * toWrite = "[fdev was here]";
    fdev->api->write( fdev, toWrite, strlen(toWrite) );
    rc = whefs_file_name_set( fi, "renamed!" );
    printf("Attempted to rename file. RC=%d\n", rc );
    whefs_fclose( fi );

    head = ls = whefs_ls( fs, lspat, &lscount );
    MARKER("EFS File list matching '%s':\n",lspat);
    while( ls )
    {
	printf("\t%s\n", ls->string );
	ls = ls->next;
    }
    whefs_string_finalize( head, true );

    toWrite = "StaticWHEFS-export.whefs";
    rc = whefs_fs_dump_to_filename( fs, toWrite );
    printf("Dumping EFS to file [%s]. RC=%d\n", toWrite, rc );

#if 0
    rc = whefs_fs_append_blocks( fs, 10 );
    printf("Adding blocks: rc=%d\n",rc);
    assert( (rc==0) && "appending blocks should have failed!");
#endif

    StaticWHEFS_whefs_finalize();
    puts("Done!");
    return 0;
}
