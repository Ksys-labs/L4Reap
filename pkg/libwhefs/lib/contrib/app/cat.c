/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain

  This file implements an 'ls'-like took for whefs EFS containers.

  Bugs/misfeatures:

  - Due to how it iterates over inodes, if a node matches more than
  one of the names/wildcards provided on the command line, ls will
  only list the file once. This is arguably a good thing, but was
  unintentional and does hinder some of my caching tests :/.

*/

#include "WHEFSApp.c" /* common code for the whefs-* tools. */
#include <wh/whio/whio_streams.h> /* whio_stream_for_FILE() */

/** App-specific data. */
struct
{
    /** Destination stream for 'catted' data. */
    whio_stream * stdout;
} WhefsCatApp = {
0
};

/** Opens the given file in fs and copies it to dest */
static int cat_file( whefs_fs * fs, char const * name, whio_stream * dest )
{
    whio_stream * str = whefs_stream_open( fs, name, false, false );
    if( ! str )
    {
        APPERR("Could not open file [%s]!\n", name );
        return whefs_rc.IOError; // we have no info as to what went wrong
    }
    whio_stream_copy( str, dest );
    str->api->finalize(str);
    return whefs_rc.OK;
}

/**
   Loops over the WHEFSApp.fe list and sends each named file to
   WhefsCatApp.stdout.
*/
static int cat_doit()
{
    whefs_fs * fs = WHEFSApp.fs;
    int rc = whefs_rc.OK;
    WHEFSApp_fe const * arg = WHEFSApp.fe;
    for( ; arg; arg = arg->next )
    {
        rc = cat_file( fs, arg->name, WhefsCatApp.stdout );
        if( whefs_rc.OK != rc ) break;
    }
    return rc;
}

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "vfs_file name(s)-of-in-EFS-files";
    WHEFSApp.helpText =
	"Copies files from an EFS to standard output."
	;
    bool gotHelp = false;
    int rc = WHEFSApp_init( argc, argv, WHEFSApp_OpenRO, &gotHelp, 0 );
    if( (0 != rc) )
    {
	APPMSG("Initialization failed with error code #%d\n", rc);
	return rc;
    }
    if( gotHelp ) return 0;
    if( ! WHEFSApp.fe )
    {
        APPERR("At least one filename argument is required.\n");
        return whefs_rc.ArgError;
    }
    WhefsCatApp.stdout = whio_stream_for_FILE( stdout, true );
    if( ! WhefsCatApp.stdout )
    {
        APPERR("Could not open stdout for writing!\n");
        return whefs_rc.IOError;
    }
    rc = cat_doit();
    WhefsCatApp.stdout->api->finalize( WhefsCatApp.stdout );
    return rc;
}
