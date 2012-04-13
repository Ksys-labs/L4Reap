/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

#ifndef MMSFBL4RE_H_
#define MMSFBL4RE_H_

#ifdef __HAVE_L4_FB__

#include "mmsgui/fb/mmsfbconv.h"
#include "mmsgui/fb/fb.h"

#include <l4/re/util/video/goos_fb>
#include <l4/re/rm>

#define MMSFBDEV_MAX_LAYERS 32

class MMSFBL4Re {
private:
    L4Re::Util::Video::Goos_fb  gfb;
    L4Re::Video::View::Info     fbi;

    L4Re::Rm::Auto_region<char *> fb_addr;

    unsigned bpp;

    //! is initialized?
    bool	isinitialized;

    //! virtual framebuffer address
    void    *framebuffer_base;

    //! id of the active screen (this is for fbs != vesa)
    int     active_screen;

    typedef struct {
        //! is initialized?
        bool    isinitialized;
        //! width of the layer
        int width;
        //! height of the layer
        int height;
        //! describes the surface buffers
        MMSFBSurfacePlanesBuffer buffers;
        //! pixelformat of the layer
        MMSFBSurfacePixelFormat pixelformat;
    } MMSFBDEV_LAYER;

    //! layer infos
    MMSFBDEV_LAYER layers[MMSFBDEV_MAX_LAYERS];

    //! number of layers
    int layers_cnt;

    void printScreenInfo();
    bool buildPixelFormat();

    void genFBPixelFormat(MMSFBSurfacePixelFormat pf, unsigned int *nonstd_format, MMSFBPixelDef *pixeldef);

public:
    MMSFBL4Re();
    virtual ~MMSFBL4Re();

    bool openDevice(char *device_file = NULL);
    void closeDevice();
    bool isInitialized();

    bool waitForVSync();
    bool panDisplay(int buffer_id, void *framebuffer_base = NULL);
    //
    bool testLayer(int layer_id);
    bool initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0);
    //
    bool releaseLayer(int layer_id);
    bool restoreLayer(int layer_id);
    //
    bool getPixelFormat(int layer_id, MMSFBSurfacePixelFormat *pf);
    //bool getPhysicalMemory(unsigned long *mem);
    bool getFrameBufferBase(unsigned char **base);
    bool getFrameBufferPtr(int layer_id, MMSFBSurfacePlanesBuffer buffers, int *width, int *height);
    //
    //bool mapMmio(unsigned char **mmio);
    //bool unmapMmio(unsigned char *mmio);
    //
    bool setMode(int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0);
    //
    sigc::signal<bool, MMSFBSurfacePixelFormat, unsigned int*, MMSFBPixelDef*>::accumulated<neg_bool_accumulator> onGenFBPixelFormat;

private:

};

#endif /* __HAVE_L4_FB__ */

#endif /* MMSFBL4RE_H_ */
