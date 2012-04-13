/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/

#include <wh/whefs/whefs_config.h> // MUST COME FIRST b/c of __STDC_FORMAT_MACROS.

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <string.h>

#include <wh/whefs/whefs.h>

#if defined(__cplusplus)
extern "C" {
#endif


char const * whefs_home_page_url()
{
    return "http://code.google.com/p/whefs";
}

const whefs_rc_t whefs_rc =
    {
    0, /* OK */
    1, /* ArgError */
    2, /* IOError */
    3, /* AllocError */
    4, /* BadMagicError */
    5, /* InternalError */
    6, /* RangeError */
    7, /* FSFull */
    8, /* AccessError */
    9, /* ConsistencyError */
    10, /* NYIError */
    11, /* UnsupportedError */
    (whio_size_t)-1 /* SizeTError */,
    (whefs_id_type)-1 /* IDTypeEnd */
    };

const whefs_magic whefs_magic_default = WHEFS_MAGIC_DEFAULT;
const whefs_fs_options whefs_fs_options_default = WHEFS_FS_OPTIONS_DEFAULT;
const whefs_fs_options whefs_fs_options_nil = WHEFS_FS_OPTIONS_NIL;
 
const uint32_t * whefs_get_core_magic()
{
    return whefs_fs_magic_bytes;
}

char const * whefs_data_format_version_string()
{
    enum { bufLen = 60 };
    static char buf[bufLen] = {0,};
    if( ! *buf )
    {
	snprintf( buf, bufLen, "%4u-%02u-%02u with %02u-bit IDs",
		  whefs_fs_magic_bytes[0],
		  whefs_fs_magic_bytes[1],
		  whefs_fs_magic_bytes[2],
		  whefs_fs_magic_bytes[3] );
    }
    return buf;
}


#if 0
struct whefs_id_links
{
    whefs_id_type id;
    whefs_id_type parent;
    whefs_id_type next;
    whefs_id_type prev;
};
typedef struct whefs_id_links whefs_id_links;
int whefs_id_link( whefs_id_links * tgt, whefs_id_type const * parent, whefs_id_type const * next, whefs_id_type const * prev );
int whefs_id_link( whefs_id_links * tgt, whefs_id_type const * parent, whefs_id_type const * next, whefs_id_type const * prev )
{
    if( ! tgt ) return whefs_rc.ArgError;
    if( parent ) tgt->parent = *parent;
    if( next ) tgt->parent = *next;
    if( prev ) tgt->parent = *prev;
    return whefs_rc.OK;
}
#endif

#include "whefs_details.c" // only for debug stuff :(
void whefs_setup_debug( FILE * ostream, unsigned int flags )
{
    whdbg_set_stream( ostream );
    whdbg_set_flags( (-1==flags) ? WHEFS_DBG_F_DEFAULT : flags );
}


typedef struct
{
    char letter;
    unsigned int flag;
    char const * descr;
} whefs_dbg_flag_info;
static const whefs_dbg_flag_info whefs_dbg_flags[] =
    {// keep sorted on the letter field.
    {'a',WHDBG_ALWAYS,"All messages."},
    {'c',WHEFS_DBG_F_CACHE,"Caching messages."},
    {'d',WHEFS_DBG_F_DEFAULT,"Default log level."},
    {'e',WHEFS_DBG_F_ERROR,"Error messages."},
    {'f',WHEFS_DBG_F_FIXME,"FIXME messages."},
    {'h',WHEFS_DBG_F_DEFAULTS_HACKER,"Hacker-level messages."},
    {'l',WHEFS_DBG_F_LOCK,"Locking messages."},
    {'n',WHEFS_DBG_F_NYI,"NYI messages."},
    {'w',WHEFS_DBG_F_WARNING,"Warning messages."},
    {0,0,0}
    };

void whefs_setup_debug_arg( FILE * ostream, char const * arg )
{
    unsigned int flags = 0;
    if( arg ) for( ; *arg; ++arg )
    {
        whefs_dbg_flag_info const * fi = &whefs_dbg_flags[0];
        for( ; fi->letter && (*arg >= fi->letter); ++fi )
        {
            if( *arg == fi->letter )
            {
                flags |= fi->flag;
                break;
            }
        }
    }
    whefs_setup_debug( ostream, flags );
}

#if defined(__cplusplus)
} /* extern "C" */
#endif
