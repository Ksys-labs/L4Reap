/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re.
 *
 */

#include "mmstools/vfswrapper.h"
#include "mmstools/tools.h"

#include <l4/libwhefs/wh/whefs/whefs.h>

#include <stdio.h>

VfsWrapper::creator s_singleton_creator;

char const * WHEFSApp_errno_to_string( int n )
{

    /* whio and whefs... */
    if( whefs_rc.OK == n ) return "whefs_rc.OK";
    if( -1 == n ) return "whefs_rc.SizeTError";

    /* whefs... */
    if( whefs_rc.ArgError == n ) return "whefs_rc.ArgError";
    if( whefs_rc.IOError == n ) return "whefs_rc.IOError";
    if( whefs_rc.AllocError == n ) return "whefs_rc.AllocError";
    if( whefs_rc.BadMagicError == n ) return "whefs_rc.BadMagicError";
    if( whefs_rc.InternalError == n ) return "whefs_rc.InternalError";
    if( whefs_rc.RangeError == n ) return "whefs_rc.RangeError";
    if( whefs_rc.FSFull == n ) return "whefs_rc.FSFull";
    if( whefs_rc.AccessError == n ) return "whefs_rc.AccessError";
    if( whefs_rc.ConsistencyError == n ) return "whefs_rc.ConsistencyError";
    if( whefs_rc.NYIError == n ) return "whefs_rc.NYIError";
    if( whefs_rc.UnsupportedError == n ) return "whefs_rc.UnsupportedError";

    /* whio... */
    if( whio_rc.ArgError == n ) return "whio_rc.ArgError";
    if( whio_rc.IOError == n ) return "whio_rc.IOError";
    if( whio_rc.AllocError == n ) return "whio_rc.AllocError";
    if( whio_rc.InternalError == n ) return "whio_rc.InternalError";
    if( whio_rc.RangeError == n ) return "whio_rc.RangeError";
    if( whio_rc.AccessError == n ) return "whio_rc.AccessError";
    if( whio_rc.ConsistencyError == n ) return "whio_rc.ConsistencyError";
    if( whio_rc.NYIError == n ) return "whio_rc.NYIError";
    if( whio_rc.UnsupportedError == n ) return "whio_rc.UnsupportedError";
    if( whio_rc.TypeError == n ) return "whio_rc.TypeError";

    return "Unknown Error Code";
}

VfsWrapper::VfsWrapper() :
    isFsOpened(false),
    fs(0)
{
    printf("----------------------------------------------------------------------\n");
    printf("VfsWrapper::VfsWrapper\n");

    int rc = whefs_openfs( "rom/disko.whefs", &fs, false );
    if( whefs_rc.OK == rc ) {
        isFsOpened = true;
    }
    else {
        printf("VfsWrapper: cannot read file rom/disko.whefs (%s)\n", WHEFSApp_errno_to_string(rc));
    }

    test_fs_content();
    printf("----------------------------------------------------------------------\n");
}

VfsWrapper::~VfsWrapper()
{
    if (fs)
        whefs_fs_finalize( fs );
}


whefs_file* VfsWrapper::fopen(const char * filename, const char * mode)
{
    if (!isFsOpened)
        return 0;

    return whefs_fopen(fs, filename, mode);
}

int VfsWrapper::fclose ( whefs_file *stream )
{
    if (!isFsOpened)
        return EOF;

    return whefs_fclose(stream);
}

int VfsWrapper::f_eof(whefs_file *stream)
{
    if (!isFsOpened)
        return 1;

    long int pos = ftell(stream);
    if (pos == -1)
        return 1;

    size_t size = whefs_fsize(stream);
    if (pos == size)
        return 1;

    return 0;
}

void VfsWrapper::rewind ( whefs_file * stream )
{
    if (!isFsOpened)
        return;

    whefs_frewind(stream);
}

int VfsWrapper::fseek ( whefs_file * stream, long int offset, int origin )
{
    if (!isFsOpened)
        return EOF;

    return whefs_fseek(stream, offset, origin);
}

long int VfsWrapper::ftell ( whefs_file * stream )
{
    if (!isFsOpened)
        return -1L;

    return whefs_fseek(stream, 0, SEEK_CUR);
}

size_t VfsWrapper::fread ( void * ptr, size_t size, size_t count, whefs_file * stream )
{
    if (!isFsOpened)
        return 0;

    size_t res = whefs_fread(stream, size, count, ptr);
    return res;
}

char * VfsWrapper::fgets ( char * str, int num, whefs_file * stream )
{
    if (!isFsOpened)
        return 0;

    char c;
    char *p;

    p = str;
    while (--num > 0) {

        if ( fread(&c, 1, 1, stream) != 1)
            break;

        *p++ = c;
        if ( c == '\n')
            break;
    }

    if (p == str) return 0;

    *p = '\0';

    return str;
}

size_t VfsWrapper::fwrite ( const void * ptr, size_t size, size_t count, whefs_file * stream )
{
    printf("VfsWrapper::fwrite not implemented\n");
    return 0;
}

bool VfsWrapper::exists(const char * filename)
{
    whefs_file *f = whefs_fopen(fs, filename, "r");

    if( f != 0 ) {
        whefs_fclose(f);
        return true;
    }
    return false;
}


#include <wh/whefs/whefs_client_util.h>

typedef struct
{
    size_t pos;
} ForEachData;
static const ForEachData ForEachDataInit = {0};

/** Callback for use with whefs_fs_entry_foreach(). */
int ls_foreach( whefs_fs * fs, whefs_fs_entry const * ent, void * data )
{
    ForEachData * fd = (ForEachData *)data;
    if( 1 == ++fd->pos )
    {
        printf("%-16s%-16s%-12s%-16s%-16s\n",
               "Node ID:","First block:", "Size:", "Timestamp:","Name:" );
    }
    printf("%-16"WHEFS_ID_TYPE_PFMT"%-16"WHEFS_ID_TYPE_PFMT"%-12u%-16u%s\n",
           ent->inode_id,
           ent->block_id,
           ent->size,
           ent->mtime,
           ent->name.string );

    return whefs_rc.OK;
}

void VfsWrapper::test_fs_content()
{
    if (!isFsOpened)
        return;

    ForEachData d = ForEachDataInit;
    whefs_fs_entry_foreach( fs, ls_foreach, &d );
}
