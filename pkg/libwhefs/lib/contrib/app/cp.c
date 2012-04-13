/*
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
#include "whargv.h"
#include <wh/whio/whio_devs.h>

static struct
{
    bool exportMode;
} ThisApp = {
false /* exportMode */
};

#include <ctype.h> /* isdigit() */

#include "WHEFSApp.c"
#include <wh/whefs/whefs_client_util.h>


enum Errors {
ErrOpenFailed = 1,
ErrIO = 2,
ErrNYI = 3,
ErrArgs = 4
};

enum { WHEFS_CP_BUFFER_SIZE = 1024 * 32 };

static int cp_import( char const *fname )
{
#define CLEANUP if(inf) inf->finalize(inf);
    whio_dev * inf = whio_dev_for_filename( fname, "r" );
    if( ! inf )
    {
	APPERR("Could not open local file '%s' for reading!\n",fname);
	return ErrOpenFailed;

    }
    int rc = whefs_import_dev( WHEFSApp.fs, inf, fname, true );
    size_t sz = whio_dev_size( inf );
    inf->api->finalize(inf);
    if( rc != whefs_rc.OK )
    {
	APPERR("Import of file [%s] failed!\n", fname );
	return rc;
    }
    VERBOSE("Copied %u bytes from [%s] into EFS\n", sz, fname);
#undef CLEANUP
    return 0;
}

static int cp_export( char const *fname )
{
#define CLEANUP if(dest) dest->api->finalize(dest); if(fdev) fdev->api->finalize(fdev)
    whio_dev * fdev = whefs_dev_open( WHEFSApp.fs, fname, false );
    if( ! fdev )
    {
	APPERR("Could not open pseudo-file '%s' for reading!\n",fname);
	return ErrOpenFailed;
    }

    whio_dev * dest = whio_dev_for_filename( fname, "w" );
    if( ! dest )
    {
	APPERR("Could not local file '%s' for writing!\n",fname);
	CLEANUP;
	return ErrOpenFailed;
    }
    dest->api->truncate( dest, 0U );

    const size_t destSize = whio_dev_size( fdev ); /* it makes me sick to think of how many disk accesses that needs. */
    unsigned char buf[WHEFS_CP_BUFFER_SIZE];
    size_t wrc = 0;
    size_t total = 0;
    while( true )
    {
	size_t rdrc = fdev->api->read( fdev, buf, WHEFS_CP_BUFFER_SIZE );
	//WHEFS_DBG("Read %u of %u bytes.", rdrc, WHEFS_CP_BUFFER_SIZE );
	if( ! rdrc ) break;
	wrc = dest->api->write( dest, buf, rdrc );
	//WHEFS_DBG("Wrote %u of %u bytes.", wrc, rdrc );
	if( wrc != rdrc )
	{
	    APPERR("Write failed! Read %u-byte block but could only write %u bytes to '%s'!\n", rdrc, wrc, fname );
	    CLEANUP;
	    return ErrIO;
	}
	total += wrc;
	if( wrc < WHEFS_CP_BUFFER_SIZE ) break;
    }
    if( total != destSize )
    {
	APPERR("Could only write %u of %u bytes!\n", total,
		      destSize );
	    CLEANUP;
	    return ErrIO;
    }
    VERBOSE("Copied %u bytes from EFS into [%s].\n",total, fname);
    CLEANUP;
#undef CLEANUP
    return 0;
}

/**
   The order of these are is significant: any change must be reflected
   in main(), where CpArgSpec is filled out.
 */
static ArgSpec CpArgSpec[] = {
{"i",  ArgTypeBoolInvert, &ThisApp.exportMode, "Enables import mode (the default).", 0, 0 },
{"x",  ArgTypeBool, &ThisApp.exportMode, "Enables export mode (disables -i).", 0, 0 },
{0}
};
enum {
ArgIDImport = 0,
ArgIDExport = 1
};

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags] vfs_file file1 [... fileN]";
    WHEFSApp.helpText =
	"This program imports and exports local files to and from an EFS file."
	;
    int rc = 0;
    bool gotHelp = false;
    rc = WHEFSApp_init( argc, argv, WHEFSApp_NoOpen, &gotHelp, CpArgSpec );
    if( (0 != rc) )
    {
	APPMSG("Initialization failed with error code #%d\n", rc);
	return rc;
    }
    if( (1==argc) || gotHelp )
    {
	//cp_show_help();
	return 0;
    }
    if( ! WHEFSApp.fe )
    {
	APPERR("No source files specified!\n");
	return whefs_rc.ArgError;
    }

    VERBOSE("Opening EFS [%s]\n", WHEFSApp.fsName);
    rc = whefs_openfs( WHEFSApp.fsName, &WHEFSApp.fs,
		       ThisApp.exportMode ? false : true );
    if( whefs_rc.OK != rc )
    {
	APPERR("Open of EFS file [%s] failed with error code %d!\n", WHEFSApp.fsName, rc );
	return rc;
    }

    WHEFSApp_fe * fe = WHEFSApp.fe;
    for( ; fe; fe = fe->next )
    {
	VERBOSE("Copying file [%s] %s EFS\n", fe->name,(ThisApp.exportMode?"from":"to"));
	if( ThisApp.exportMode ) rc = cp_export(fe->name);
	else rc = cp_import(fe->name);
	if( whefs_rc.OK != rc )
	{
	    APPERR("%s failed for file [%s]! Error code=%d!\n",
		   (ThisApp.exportMode?"Export":"Import"), fe->name, rc );
	    return rc;
	}
	continue;
    }
    //printf("vfs file=[%s]\n",WHEFSApp.fsName);
    //whefs_fs_dump_info( WHEFSApp.fs, stdout );
    /* reminder: memory is cleaned up via atexit() */
    return 0;
}
