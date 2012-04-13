#define _POSIX_C_SOURCE 1
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

#include "WHEFSApp.c"
#include <wh/whefs/whefs_string.h>

enum { ThisApp_MAX_COMMANDS = 20 };
typedef int (*ls_command_f)();
static struct ThisApp_
{
    struct Funcs
    {
        /* implementation funcs for specific --flags */
	ls_command_f f[ThisApp_MAX_COMMANDS];
	size_t count;
    } funcs;
    bool didSomething;
    bool oneMode; // "ls -1"
} ThisApp =
    {
    { /* funcs */
    {0} /* f */,
    0 /*count*/
    },
    false /* didSomething */,
    false /* oneMode */
    };

/**
   Appends f to ThisApp.funcs.
*/
static int ThisApp_push_command( ls_command_f f )
{
    assert( ThisApp.funcs.count < ThisApp_MAX_COMMANDS );
    ThisApp.funcs.f[ThisApp.funcs.count++] = f;
    return 0;
}


/**
   Internal data for use with whefs_inode_foreach().
*/
struct ls_foreach_info
{
    /** Total byte size matched so far. */
    uint64_t totalSize;
    /** Number of inodes checks. */
    whefs_id_type checkedInodesCount;
    /** Number of inodes which matched a pattern. */
    whefs_id_type matchCount;
    /**
       Buffer for passing the inode name around.
    */
    whefs_string name;
};
typedef struct ls_foreach_info ls_foreach_info;
static ls_foreach_info ls_foreach_info_init = {
0U /*totalSize*/,
0U /*checkedInodesCount*/,
0U /*matchCount*/,
whefs_string_empty_m /*name*/
};

static bool ls_inode_predicate_name_matches( whefs_fs * fs, whefs_inode const * n, void * clientData )
{
    if(0) ls_inode_predicate_name_matches(0,0,0); // work around "static function defined but not used.
    ls_foreach_info * info = (ls_foreach_info*)clientData;
    ++(info->checkedInodesCount);
    if(! (n->flags & WHEFS_FLAG_Used)) return false;
    int rc = whefs_inode_name_get( fs, n->id, &info->name );
    if( whefs_rc.OK != rc )
    {
        APPERR("Error #%d while reading name for inode #%"WHEFS_ID_TYPE_PFMT"!",rc,n->id);
        return false;
    }
    if( ! info->name.length ) return false;
    else if( ! WHEFSApp.fe ) return true;
    else if( WHEFSApp_matches_fe( info->name.string ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

#include <time.h>
static int ls_inode_foreach_print( whefs_fs * fs, whefs_inode const * ino, void * clientData )
{
    ls_foreach_info * info = (ls_foreach_info*)clientData;
    ++(info->matchCount);
    if( ThisApp.oneMode )
    {
        puts(info->name.string);
    }
    else
    {
        time_t mt = (time_t)ino->mtime;
        struct tm * t = 0;
        t = localtime( &mt );
        enum { bufSize = 40 };
        char buf[bufSize];
        memset( buf, 0, bufSize );
        strftime( buf, bufSize-1, "%Y.%m.%d %H:%M:%S", t );
        printf("%-6"WHEFS_ID_TYPE_PFMT"%9u%22s  %s\n",
               ino->id,
               ino->data_size,
               buf,
               info->name.string );
    }
    info->totalSize += ino->data_size;
    return whefs_rc.OK;
}



static int ls_dump_inodes()
{
    ThisApp.didSomething = true;
    whefs_fs * fs = WHEFSApp.fs;
    int rc = 0;
    if( ! ThisApp.oneMode )
    {
        puts("List of file entries:\n");
        printf("%-10s%s%19s%10s\n",
               "Node ID:","Size:", "Timestamp: (YMD)","Name:" );
    }
    ls_foreach_info foi = ls_foreach_info_init;
    enum { bufSize = WHEFS_MAX_FILENAME_LENGTH + 1 };
    char buf[bufSize] = {0};
    foi.name.string = buf;
    foi.name.alloced = bufSize;
    rc = whefs_inode_foreach( fs, ls_inode_predicate_name_matches, &foi, ls_inode_foreach_print, &foi );
    //whefs_string_clear( &foi.name, false );
    //MARKER("foreach rc=%d, predicate checks=%"WHEFS_ID_TYPE_PFMT" \n",rc, foi.checkedInodesCount);
    if( ! ThisApp.oneMode )
    {
        printf("%31s %"PRIu64" bytes\n", "Total:", foi.totalSize );
        printf("%"WHEFS_ID_TYPE_PFMT" of %"WHEFS_ID_TYPE_PFMT" total inodes listed.\n",
               foi.matchCount,
               whefs_fs_options_get(fs)->inode_count );
    }
    return rc;
}

static int ls_dump_blocks()
{
    ThisApp.didSomething = true;
    whefs_fs * fs = WHEFSApp.fs;
    const size_t nc = whefs_fs_options_get(fs)->inode_count;
    size_t id = 1;
    int rc = 0;
    int64_t totalSize = 0;
    size_t used = 0;
    puts("List of used blocks per file:\n");
    whefs_block bl;
    whefs_inode ino;
    size_t bcount = 0;
    whefs_string name = whefs_string_empty;
    for( ; id <= nc; ++id )
    {
	rc = whefs_inode_id_read( fs, id, &ino );
	if( whefs_rc.OK != rc )
	{
	    APPERR("Error #%d while reading inode #%"WHEFS_ID_TYPE_PFMT"!\n", rc, id);
	    return rc;
	}
	if( ! ino.flags || ! ino.first_block ) continue;
	rc = whefs_inode_name_get( fs, ino.id, &name );
	if( whefs_rc.OK != rc )
	{
	    APPERR("Error #%d while reading name for inode #%"WHEFS_ID_TYPE_PFMT"!\n", rc, id);
	    whefs_string_clear( &name, false );
	    return rc;
	}
	if( WHEFSApp.fe && ! WHEFSApp_matches_fe( name.string ) ) continue;
	rc = whefs_block_read( fs, ino.first_block, &bl );
	if( whefs_rc.OK != rc )
	{
	    APPERR("Error #%d while reading block #%u of inode #%u[%s]!\n",
                   rc, ino.first_block, ino.id, name.string );
	    whefs_string_clear( &name, false );
	    return rc;
	}
	++used;
	++bcount;
	//printf("inode #%u[%s] blocks: %u", ino.id, name.string, bl.id );
	printf("#%04"WHEFS_ID_TYPE_PFMT"[%s]:\t(%"WHEFS_ID_TYPE_PFMT"%s", ino.id, name.string, bl.id,bl.next_block ? "," : "" );
	size_t lbcount = 1;
	for( ; 0 != bl.next_block; ++lbcount )
	{
	    const whefs_block blcheck = bl;
	    rc = whefs_block_read_next( WHEFSApp.fs, &bl, &bl );
	    if( whefs_rc.OK == rc )
	    {
		++bcount;
		printf( " %"WHEFS_ID_TYPE_PFMT"%s", bl.id,
                        bl.next_block ? "," : "");
	    }
	    else
	    {
		APPERR("Error #%d while reading block %"WHEFS_ID_TYPE_PFMT" (next block after %"WHEFS_ID_TYPE_PFMT")!\n",
		       rc, blcheck.id, bl.id );
		whefs_string_clear( &name, false );
		return rc;
	    }
	}
	totalSize += ino.data_size;
	if( WHEFSApp.verbose )
	{
	    printf(") = %u block(s), %u of %u bytes used\n",
		   lbcount,
		   ino.data_size,
		   (lbcount* whefs_fs_options_get(fs)->block_size) );
	}
	else puts(")");
    }
    whefs_string_clear( &name, false );
    printf("\nTotals: %llu byte(s) in %u blocks\n", totalSize, bcount );
    return rc;
}

static int ls_dump_blocks_table()
{
    ThisApp.didSomething = true;
    /** TODO: instead of a list, output the blocks like this:

    PARENT_ID --> CHILD_1_ID [ --> CHILD_2_ID ...]

    The performance on that operation is pathological using the
    current lib, though.
     */
    whefs_fs * fs = WHEFSApp.fs;
    const whefs_id_type bc = whefs_fs_options_get(fs)->block_count;
    size_t id = 1;
    int rc = 0;
    puts("List of used blocks:");
    //puts("Block ID:\tFlags:\tNext block:\tUsed bytes:");
    whefs_id_type used = 0;
    printf("%-16s%-16s%-16s\n", "Block ID:","Next block:","Flags:" );
    for( ; id < bc; ++id )
    {
	whefs_block bl;
	rc = whefs_block_read( fs, id, &bl );
	if( whefs_rc.OK != rc )
	{
	    APPERR("Error #%d while reading block #%u!\n", rc, id);
	    return rc;
	}
	if( ! bl.flags ) continue;
	++used;
	printf("%-16"WHEFS_ID_TYPE_PFMT"%-16"WHEFS_ID_TYPE_PFMT"%-16u\n", bl.id, bl.next_block, bl.flags );
    }
    printf("\n%"WHEFS_ID_TYPE_PFMT" of %"WHEFS_ID_TYPE_PFMT" blocks used\n", used, bc);
    return rc;
}


static int ls_dump_options_code()
{
    whefs_fs_options const * o = whefs_fs_opt( WHEFSApp.fs );
    printf("whefs_fs_options opt = { "
	   "FIXME_FS_MAGIC, "
	   "%"PRIu32" /*block_size*/, "
	   "%"WHEFS_ID_TYPE_PFMT" /*block_count*/, "
	   "%"WHEFS_ID_TYPE_PFMT" /*inode_count*/, "
	   "%u /*filename_length*/ "
	   "};",
	   o->block_size,
	   o->block_count ,
	   o->inode_count ,
	   o->filename_length
	   );
    puts("");
    return 0;
}

static int ls_dump_mkfs_command()
{
    whefs_fs_options const * o = whefs_fs_opt( WHEFSApp.fs );
    printf("whefs-mkfs -b%"PRIu32" -c%"WHEFS_ID_TYPE_PFMT" -i%"WHEFS_ID_TYPE_PFMT" -s%u\n",
	   o->block_size,
	   o->block_count ,
	   o->inode_count ,
	   o->filename_length
	   );
    return 0;
}

static int ls_dump_stats()
{
    whefs_fs_dump_info( WHEFSApp.fs, stdout );
    return 0;
}

static int ls_dump_core_magic()
{
    uint32_t const * a = whefs_get_core_magic();
    for( ; a && *a; ++a )
    {
	printf("%u%s", *a, (*(a+1) ? "." : "\n"));
    }
    return 0;
}
static int ls_dump_core_magic_cb( char const * key, char const * val, void const * d )
{
    return ls_dump_core_magic();
}

/**
   Proxy for a function pointer, to work around not being able to legally
   convert a (void const *) to a (ls_command_f).
*/
typedef struct
{
    ls_command_f f;
} ls_command_f_pedantic_kludge;

static const ls_command_f_pedantic_kludge ls_cmd_dump_inodes = {ls_dump_inodes};
static const ls_command_f_pedantic_kludge ls_cmd_dump_blocks = {ls_dump_blocks};
static const ls_command_f_pedantic_kludge ls_cmd_dump_blocks_table = {ls_dump_blocks_table};
static const ls_command_f_pedantic_kludge ls_cmd_dump_stats = {ls_dump_stats};
static const ls_command_f_pedantic_kludge ls_cmd_dump_mkfs_command = {ls_dump_mkfs_command};
static const ls_command_f_pedantic_kludge ls_cmd_dump_options_code = {ls_dump_options_code};


static int ls_arg_push_cb( char const * key, char const * val, void const * d )
{
    ThisApp.didSomething = true;
    return ThisApp_push_command( ((ls_command_f_pedantic_kludge const *)d)->f );
}

static ArgSpec LsArgSpec[] =
    {
    {"1",  ArgTypeBool, &ThisApp.oneMode,
     "A simple list of files, one name per line.",
     ls_arg_push_cb, &ls_cmd_dump_inodes },
    {"i",  ArgTypeIgnore, 0,
     "Show inode info (i.e. file list). This is the default.",
     ls_arg_push_cb, &ls_cmd_dump_inodes },
    {"b",  ArgTypeIgnore, 0,
     "Shows used blocks for each file. Use -v for more detail.",
     ls_arg_push_cb, &ls_cmd_dump_blocks },
    {"B",  ArgTypeIgnore, 0,
     "Shows block table.",
     ls_arg_push_cb, &ls_cmd_dump_blocks_table},
    {"s",  ArgTypeIgnore, 0,
     "Shows some overall EFS stats.",
     ls_arg_push_cb, &ls_cmd_dump_stats },
    {"dump-core-magic",  ArgTypeIgnore, 0,
     "Dumps the library's file format version number. This can be used without specifying an EFS.",
     ls_dump_core_magic_cb, 0 },
    {"dump-mkfs-command",  ArgTypeIgnore, 0,
     "Dumps the EFS options as arguments for passing to whefs-mkfs.",
     ls_arg_push_cb, &ls_cmd_dump_mkfs_command },
    {"dump-options-code",  ArgTypeIgnore, 0,
     "Dumps the EFS options as C source code.",
     ls_arg_push_cb, &ls_cmd_dump_options_code },
    {0,0,0,0,0}
    };

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags] vfs_file [filenames or quoted glob patterns]";
    WHEFSApp.helpText =
	"Lists information about an EFS file and the files contained within it."
	;
    bool gotHelp = false;
    int rc = WHEFSApp_init( argc, argv, WHEFSApp_OpenRO, &gotHelp, LsArgSpec );
    if( (0 != rc) )
    {
	APPMSG("Initialization failed with error code #%d\n", rc);
	return rc;
    }
    if( gotHelp ) return 0;
#define CHECKRC if(0 != rc ) { APPERR("Error code #%d!\n",rc); return rc; }
    if( !ThisApp.didSomething )
    {
	rc = ls_dump_inodes();
	CHECKRC;
    }
    if( ThisApp.funcs.count )
    {
	size_t i = 0;
	for( ; i < ThisApp.funcs.count; ++i )
	{
	    rc = ThisApp.funcs.f[i]();
	    CHECKRC;
	}
    }
#undef CHECKRC
    return rc;
}
