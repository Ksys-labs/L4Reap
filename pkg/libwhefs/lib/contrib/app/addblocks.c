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
    whefs_id_type count;
} ThisApp = {
0
};

#include "WHEFSApp.c"

/**
   The order of these are is significant: any change must be reflected
   in main(), where AddblocksArgSpec is filled out.
 */
static ArgSpec AddblocksArgSpec[] = {
{"a",  ArgTypeIDType, &ThisApp.count,
 "Sets the number of blocks to append to the EFS.",
 0, 0 },
{0}
};
int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags] vfs_file";
    WHEFSApp.helpText =
	"This program adds data blocks to an existing whefs container."
	;
    int rc = 0;
    bool gotHelp = false;
    rc = WHEFSApp_init( argc, argv, WHEFSApp_OpenRW, &gotHelp, AddblocksArgSpec );
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
    if( ! ThisApp.count )
    {
        APPERR("You must specify -aNNN to set the number of blocks to add.\n");
        return whefs_rc.ArgError;
    }
    whefs_fs_options const * fopt = whefs_fs_options_get( WHEFSApp.fs );
    const whefs_id_type total = fopt->block_count + ThisApp.count;
    if( (total < fopt->block_count) ) /* this only catches once-around overflows :( */
    {
        APPERR("Adding %"WHEFS_ID_TYPE_PFMT" blocks to the EFS would overflow its ID type!\n",ThisApp.count);
        return whefs_rc.RangeError;
    }
    VERBOSE("total blocks=%"WHEFS_ID_TYPE_PFMT"\n",total);
    rc = whefs_fs_append_blocks( WHEFSApp.fs, ThisApp.count );
    if( whefs_rc.OK != rc )
    {
        APPWARN("whefs_fs_append_blocks([%s], %"WHEFS_ID_TYPE_PFMT") failed with rc %d!\n",
                WHEFSApp.fsName, ThisApp.count,rc );
    }
    else
    {
        VERBOSE("Appended %"WHEFS_ID_TYPE_PFMT".\n", ThisApp.count );
    }
    return rc;
}
