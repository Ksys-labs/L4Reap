/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re.
 *
 */

#ifndef _VFSWRAPPER_H_
#define _VFSWRAPPER_H_

#include <stdio.h>
#include <l4/libwhefs/wh/whefs/whefs.h>

class VfsWrapper
{
private:
    VfsWrapper();
    VfsWrapper(const VfsWrapper&);
    VfsWrapper& operator=(const VfsWrapper&);
    ~VfsWrapper();

    void test_fs_content();

public:
    static VfsWrapper& instance()
    {
        static VfsWrapper inst;
        return inst;
    }

    struct creator {
        creator() { VfsWrapper::instance(); }
    };
    static struct creator s_singleton_creator;


    whefs_file* fopen(const char * filename, const char * mode);
    int fclose ( whefs_file * stream );
    int f_eof ( whefs_file * stream );
    void rewind ( whefs_file * stream );
    int fseek ( whefs_file * stream, long int offset, int origin );
    long int ftell ( whefs_file * stream );
    size_t fread ( void * ptr, size_t size, size_t count, whefs_file * stream );
    char * fgets ( char * str, int num, whefs_file * stream );
    size_t fwrite ( const void * ptr, size_t size, size_t count, whefs_file * stream );

    bool exists(const char * filename);

private:
    whefs_fs    *fs;
    bool        isFsOpened;
};



#endif // _VFSWRAPPER_H_
