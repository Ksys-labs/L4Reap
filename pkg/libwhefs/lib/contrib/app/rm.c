/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h> /* atexit() */
#include <string.h>
#include <assert.h>

#include "WHEFSApp.c"
#include <wh/whefs/whefs_client_util.h>

struct ThisApp
{
    bool dryRunMode;
    bool ignoreMissing;
} ThisApp =
    {
    false,
    false
    };

int rm_doit()
{
    int rc = whefs_rc.OK;
    WHEFSApp_fe * fe = WHEFSApp.fe;
    for( ; fe; fe = fe->next )
    {
	whefs_string * li = whefs_ls( WHEFSApp.fs, fe->name, 0 );
	if( ! li )
	{
            APPWARN("Empty match for [%s]\n",fe->name);
	    if( ThisApp.ignoreMissing )
	    {
		APPWARN("No match found for [%s]!\n", fe->name );
		continue;
	    }
	    else
	    {
		APPERR("No match found for [%s]!\n", fe->name );
		return whefs_rc.RangeError;
	    }
	}
	whefs_string * str = li;
	for( ; str; str = str->next )
	{
	    if( ThisApp.dryRunMode )
	    {
		VERBOSE("dry-run mode: not deleting [%s]\n", str->string );
		rc = whefs_rc.OK;
	    }
	    else
	    {
		VERBOSE("Deleting VFS file [%s]...\n", str->string );
		rc = whefs_unlink_filename( WHEFSApp.fs, str->string );
	    }
	    if( whefs_rc.OK != rc )
	    {
		APPERR("Got error code #%d while trying to remove [%s]!\n", rc, str->string );
		break;
	    }
	}
	whefs_string_finalize( li, true );
	if( whefs_rc.OK != rc ) break;
    }
    return rc;
}

static ArgSpec RmArgSpec[] = {
{"n", ArgTypeBool, &ThisApp.dryRunMode,
 "Dry-run mode: do not actually delete anything.",
 0, 0},
{"m", ArgTypeBool, &ThisApp.ignoreMissing,
 "Ignore missing files - do not error out if any given pattern does not match a file.",
 0, 0},
{0}
};

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags] vfs_file [filenames or quoted glob patterns to delete]";
    WHEFSApp.helpText =
	"Deletes files within a VFS."
	;
    bool gotHelp = false;
    int rc = WHEFSApp_init( argc, argv, WHEFSApp_OpenRW, &gotHelp, RmArgSpec );
    if( (0 != rc) )
    {
	APPMSG("Initialization failed with error code #%d\n", rc);
	return rc;
    }
    if( gotHelp ) return 0;

    rc = rm_doit();
    return rc;
}
