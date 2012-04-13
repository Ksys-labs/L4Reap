/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <wh/whio/whio_devs.h>
#include "WHEFSApp.c"

static struct
{
    bool dryRun;
    whefs_fs_options fsopt;
} ThisApp = {
    false, /* dryRun */
    WHEFS_FS_OPTIONS_INIT(1024 * 8, 200, 64)
};

static int mkfs_do_mkfs()
{
    if( WHEFSApp.fs ) return whefs_rc.ArgError;
    whefs_fs_options * fsopt = &ThisApp.fsopt;

    VERBOSE("VSF OPTIONS: block_size=%u "
	   "block_count=%"WHEFS_ID_TYPE_PFMT" "
	   "inode_count=%"WHEFS_ID_TYPE_PFMT" "
	   "filename_length=%u\n",
	   fsopt->block_size,
	   fsopt->block_count,
	   fsopt->inode_count,
	   fsopt->filename_length);

    if( fsopt->block_count < fsopt->inode_count )
    {
	APPERR("Block count (%"WHEFS_ID_TYPE_PFMT") must be greater than or equal to the inode count (%"WHEFS_ID_TYPE_PFMT").\n",
	       fsopt->block_count, fsopt->inode_count );
	return whefs_rc.RangeError;
    }
    if( fsopt->filename_length > WHEFS_MAX_FILENAME_LENGTH )
    {
	APPERR("Filename length %u is longer than the built-in maximum of WHEFS_MAX_FILENAME_LENGTH (%u)!\n",
	       fsopt->filename_length, WHEFS_MAX_FILENAME_LENGTH);
	return whefs_rc.RangeError;
    }
    int rc = 0; // WHEFSApp_openfs( WHEFSApp_OpenRW );
    if( whefs_rc.OK != rc ) return rc;

    whio_dev * dev = 0;
    const size_t fsSize = whefs_fs_calculate_size( &ThisApp.fsopt );
    bool inMem = (0 == strcmp( ":memory:", WHEFSApp.fsName ));
    if( inMem  )
    {
	MARKER("Allocating %u bytes of memory for the filesystem...\n", fsSize );
	dev = whio_dev_for_membuf( fsSize, 0.0 );
	MARKER("Allocated %u bytes of memory for the filesystem.\n", fsSize );
    }
    else
    {
	dev = whio_dev_for_filename( WHEFSApp.fsName, "w" );
    }
    if( ! dev )
    {
	if( inMem )
	{
	    WHEFS_DBG_ERR("Could not create an in-memory buffer of %u bytes!", fsSize );
	    return whefs_rc.AllocError;
	}
	else
	{
	    WHEFS_DBG_ERR("Could not open file [%s] for writing!",WHEFSApp.fsName);
	    return whefs_rc.AccessError;
	}
    }
    rc = whefs_mkfs_dev( dev, fsopt, &WHEFSApp.fs, true );
    if( whefs_rc.OK != rc )
    {
	dev->api->finalize(dev);
	remove( WHEFSApp.fsName );
	return rc;
    }
    return 0;
}


static void mkfs_dump_info()
{
    FILE * out = stdout;
    whefs_fs_options const * opt = &ThisApp.fsopt;
    uint64_t const dataSize = (opt->block_count * opt->block_size);
    size_t const contSize = whefs_fs_calculate_size(opt);
    if( ! contSize )
    {
	WHEFS_DBG_ERR("Something seriously wrong happened: calculated container size is 0!");
	return;
    }
    fprintf( out,
	     "EFS container file: %s\n"
	     "\tBlock size: %"PRIu32"\n"
	     "\tBlock count: %"WHEFS_ID_TYPE_PFMT"\n"
	     "\tinode count: %"WHEFS_ID_TYPE_PFMT"\n"
	     "\tFree data bytes: %"PRIu64"\n"
	     "\tMax filename length: %u\n"
	     "\tRequired container size (bytes): %u\n",
	     WHEFSApp.fsName,
	     opt->block_size,
	     opt->block_count,
	     opt->inode_count,
	     dataSize,
	     opt->filename_length,
	     contSize
	     );
    fprintf(out, "\tMetadata-to-data ratio: %3.4lf%%\n", 100 - ((double)dataSize/(double)contSize) * 100.0);
}


static ArgSpec MkfsArgSpec[] = {
{"n",  ArgTypeBool, &ThisApp.dryRun, "Dry-run mode (shows what WOULD be done, but doesn't do it).", 0, 0},
{"i",  ArgTypeIDType, &ThisApp.fsopt.inode_count, "Number of inodes for the EFS.", 0, 0},
{"inode-count",  ArgTypeIDType, &ThisApp.fsopt.inode_count, "Same as -i", 0, 0},
{"c",  ArgTypeIDType, &ThisApp.fsopt.block_count, "Number of data blocks for the EFS.", 0, 0},
{"block-count",  ArgTypeIDType, &ThisApp.fsopt.block_count, "Same as -c", 0, 0},
{"b",  ArgTypeUInt32, &ThisApp.fsopt.block_size, "The size of each block, in bytes.", 0, 0},
{"block-size",  ArgTypeUInt32, &ThisApp.fsopt.block_size, "Same as -b.", 0, 0},
{"s",  ArgTypeUInt16, &ThisApp.fsopt.filename_length, "The maximum length of file names in the EFS.", 0, 0},
{"string-length",  ArgTypeUInt16, &ThisApp.fsopt.filename_length, "Same as -s.", 0, 0},
{0}
};

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags] vfs_file";
    WHEFSApp.helpText = "Creates whefs embedded filesystem (EFS) container files.\n"
	"*** WARNING: DESTROYS TARGET EFS FILE WITHOUT ASKING UNLESS -n IS SPECIFIED! ***";
    int rc = 0;
    bool gotHelp = false;
    rc = WHEFSApp_init( argc, argv, WHEFSApp_NoOpen, &gotHelp, MkfsArgSpec );
    if( (0 != rc) )
    {
	APPMSG("Initialization failed with error code #%d\n", rc);
	return rc;
    }
    if( gotHelp )
    {
	return 0;
    }
    if( ThisApp.dryRun )
    {
	puts("Dry-run mode! (Not creating/changing EFS container file!)");
	mkfs_dump_info();
	return 0;
    }
    else
    {
	rc = mkfs_do_mkfs();
    }
    if( whefs_rc.OK == rc )
    {
	mkfs_dump_info();
    }
    else
    {
	WHEFS_DBG_ERR("There was some error or other. Try searching these sources for error code %d (%s).",
		      rc, WHEFSApp_errno_to_string(rc) );
    }
    APPMSG("Done! Error code=%d=[%s].\n",rc,
	   (0==rc)
	   ? "You win :)"
	   : "You lose :(");
    return rc;
}
