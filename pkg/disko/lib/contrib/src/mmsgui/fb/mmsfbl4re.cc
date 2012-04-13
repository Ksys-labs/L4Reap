/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re.
 *
 */

#ifdef __HAVE_L4_FB__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mmsgui/fb/mmsfbl4re.h"

#include <l4/re/util/cap_alloc>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/video/goos>

using L4Re::Util::Auto_cap;

#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBL4RE is not initialized");return false;}

MMSFBL4Re::MMSFBL4Re() {
	// init fb vals
	this->isinitialized = false;
    this->framebuffer_base = NULL;
    memset(this->layers, 0, sizeof(this->layers));
    this->layers_cnt = 0;
    this->active_screen = 0;
}

MMSFBL4Re::~MMSFBL4Re() {
	closeDevice();
}

void MMSFBL4Re::printScreenInfo() {
    printf("MMSFBL4Re: screen info ------------\n");
    printf("    x:          %d\n", fbi.width);
    printf("    y:          %d\n", fbi.height);
    printf("    bit/pixel:  %d\n", fbi.pixel_info.bytes_per_pixel()*8);
    printf("    bytes/line: %d\n", fbi.bytes_per_line);
    printf("    A size:     %d\n", fbi.pixel_info.a().size());
    printf("    A shift:    %d\n", fbi.pixel_info.a().shift());
    printf("    R size:     %d\n", fbi.pixel_info.r().size());
    printf("    R shift:    %d\n", fbi.pixel_info.r().shift());
    printf("    G size:     %d\n", fbi.pixel_info.g().size());
    printf("    G shift:    %d\n", fbi.pixel_info.g().shift());
    printf("    B size:     %d\n", fbi.pixel_info.b().size());
    printf("    B shift:    %d\n", fbi.pixel_info.b().shift());

}

bool MMSFBL4Re::openDevice(char *device_file) {

    L4Re::Env const *env = L4Re::Env::env();

    L4::Cap<L4Re::Video::Goos> fb = env->get_cap<L4Re::Video::Goos>("fb");
    if (!fb.is_valid())
    {
        printf("MMSFBL4Re::%s: fb not found\n", __func__);
        MMSFB_SetError(0, "fb not found!");
        return false;
    }

    gfb.setup(fb);

    if (gfb.view_info(&fbi))
    {
        printf("MMSFBL4Re::%s: view_info error\n", __func__);
        MMSFB_SetError(0, "view_info error!");
        return false;
    }

    env->rm()->attach(&fb_addr, gfb.buffer()->size(), L4Re::Rm::Search_addr,
                      gfb.buffer(), 0, L4_SUPERPAGESHIFT);
    printf("MMSFBL4Re::openDevice mapped frame buffer at %p\n", fb_addr.get());

    printScreenInfo();

    // compatability with disko
    this->framebuffer_base = fb_addr.get();

    // build the preset pixelformat
    buildPixelFormat();

    // all initialized :)
    this->isinitialized = true;

    return true;
}

void MMSFBL4Re::closeDevice() {
	// reset all other
	this->isinitialized = false;
}

bool MMSFBL4Re::isInitialized() {
	return this->isinitialized;
}

bool MMSFBL4Re::buildPixelFormat() {

    this->layers[0].pixelformat = MMSFB_PF_NONE;
    switch (fbi.pixel_info.a().size()) {
    case 0:
        // pixelformat with no alphachannel
        if ((fbi.pixel_info.r().size() == 5)  && (fbi.pixel_info.g().size() == 6) && (fbi.pixel_info.b().size() == 5)
            && (fbi.pixel_info.r().shift() == 11) && (fbi.pixel_info.g().shift() == 5) && (fbi.pixel_info.b().shift() == 0))
        {
            this->layers[0].pixelformat = MMSFB_PF_RGB16;
        }
        else
        if ((fbi.pixel_info.r().size() == 8)  && (fbi.pixel_info.g().size() == 8) && (fbi.pixel_info.b().size() == 8)
            && (fbi.pixel_info.r().shift() == 16) && (fbi.pixel_info.g().shift() == 8) && (fbi.pixel_info.b().shift() == 0))
        {
            if (fbi.pixel_info.bytes_per_pixel()*8 == 24)
                this->layers[0].pixelformat = MMSFB_PF_RGB24;
            else
                this->layers[0].pixelformat = MMSFB_PF_RGB32;
        }
        else
        if ((fbi.pixel_info.r().size() == 8) && (fbi.pixel_info.g().size() == 8) && (fbi.pixel_info.b().size() == 8)
            && (fbi.pixel_info.r().shift() == 0) && (fbi.pixel_info.g().shift() == 8) && (fbi.pixel_info.b().shift() == 16))
        {
            if (fbi.pixel_info.bytes_per_pixel()*8 == 24)
                this->layers[0].pixelformat = MMSFB_PF_BGR24;
        }
        else
        if ((fbi.pixel_info.r().size() == 5) && (fbi.pixel_info.g().size() == 5) && (fbi.pixel_info.b().size() == 5)
            && (fbi.pixel_info.r().shift() == 0) && (fbi.pixel_info.g().shift() == 5) && (fbi.pixel_info.b().shift() == 10))
        {
            if (fbi.pixel_info.bytes_per_pixel()*8 == 16)
                this->layers[0].pixelformat = MMSFB_PF_BGR555;
        }
        else
        if ((fbi.pixel_info.r().size() == 0) && (fbi.pixel_info.g().size() == 0) && (fbi.pixel_info.b().size() == 0)
            && (fbi.pixel_info.r().shift() == 0) && (fbi.pixel_info.g().shift() == 0) && (fbi.pixel_info.b().shift() == 0))
        {
            if (fbi.pixel_info.bytes_per_pixel()*8 == 4)
                this->layers[0].pixelformat = MMSFB_PF_A4;
            else
                if (fbi.pixel_info.bytes_per_pixel()*8 == 16)
                    this->layers[0].pixelformat = MMSFB_PF_YUY2;
                else
                    this->layers[0].pixelformat = MMSFB_PF_NONE;
        }
        break;
    case 8:
        // pixelformat with 8 bit alphachannel
        if ((fbi.pixel_info.r().size() == 8)  && (fbi.pixel_info.g().size() == 8) && (fbi.pixel_info.b().size() == 8)
            && (fbi.pixel_info.r().shift() == 16) && (fbi.pixel_info.g().shift() == 8) && (fbi.pixel_info.b().shift() == 0))
        {
            this->layers[0].pixelformat = MMSFB_PF_ARGB;
        }
        else
        if ((fbi.pixel_info.r().size() == 8)  && (fbi.pixel_info.g().size() == 8) && (fbi.pixel_info.b().size() == 8)
            && (fbi.pixel_info.r().shift() == 0) && (fbi.pixel_info.g().shift() == 8) && (fbi.pixel_info.b().shift() == 16))
        {
            this->layers[0].pixelformat = MMSFB_PF_ABGR;
        }
        break;
    }

    if (this->layers[0].pixelformat != MMSFB_PF_NONE) {
        printf("MMSFBDev: current pixelformat is %s\n", getMMSFBPixelFormatString(this->layers[0].pixelformat).c_str());
        return true;
    }

    return false;
}


bool MMSFBL4Re::waitForVSync()
{
    INITCHECK;
  //  gfb.refresh(0, 0, fbi.width, fbi.height);
  //  printf("Warning: MMSFBL4Re::%s not implimented\n", __func__);

    return true;
}

bool MMSFBL4Re::panDisplay(int buffer_id, void *framebuffer_base)
{
    INITCHECK;

    printf("Warning: MMSFBL4Re::%s not implimented\n", __func__);

    return true;
}
//
bool MMSFBL4Re::testLayer(int layer_id)
{
    // is initialized?
    INITCHECK;

    // default fbdev does support only primary layer 0 on primary screen 0
    if (layer_id != 0) {
        printf("MMSFBL4Re: layer %d is not supported\n", layer_id);
        return false;
    }

    return true;
}

bool MMSFBL4Re::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer)
{
    INITCHECK;

    if (layer_id != 0) {
        printf("MMSFBL4Re: layer %d is not supported\n", layer_id);
        return false;
    }

    //switch video mode
    if (!setMode(width, height, pixelformat, backbuffer))
        return false;

    if (width <= 0 || height <= 0) {
        // the layer is disabled now
        this->layers[layer_id].isinitialized = false;
        return true;
    }

    // save dimension of the layer
    this->layers[layer_id].width = this->fbi.width;
    this->layers[layer_id].height = this->fbi.height;

    // save the buffers
    memset(&this->layers[layer_id].buffers, 0, sizeof(this->layers[layer_id].buffers));
    switch (backbuffer) {
        case 2:
            this->layers[layer_id].buffers[2].ptr  = ((char *)this->framebuffer_base)
                + 2 * this->fbi.bytes_per_line * this->fbi.height;
            this->layers[layer_id].buffers[2].pitch= this->fbi.bytes_per_line;
            this->layers[layer_id].buffers[2].hwbuffer = true;
        case 1:
            this->layers[layer_id].buffers[1].ptr  = ((char *)this->framebuffer_base)
                + this->fbi.bytes_per_line * this->fbi.height;
            this->layers[layer_id].buffers[1].pitch= this->fbi.bytes_per_line;
            this->layers[layer_id].buffers[1].hwbuffer = true;
        case 0:
            this->layers[layer_id].buffers[0].ptr  = this->framebuffer_base;
            this->layers[layer_id].buffers[0].pitch= this->fbi.bytes_per_line;
            this->layers[layer_id].buffers[0].hwbuffer = true;
            break;
        default:
            return false;
    }

    // layer is initialized
    this->layers[layer_id].isinitialized = true;

    // this layer is on screen 0 (default)
    this->active_screen = 0;

    return true;
}

//
bool MMSFBL4Re::releaseLayer(int layer_id)
{
    printf("MMSFBL4Re: layer %d cannot be released\n", layer_id);
    return false;
}

bool MMSFBL4Re::restoreLayer(int layer_id)
{
    printf("MMSFBL4Re: layer %d cannot be restored\n", layer_id);
    return false;
}

//
bool MMSFBL4Re::getPixelFormat(int layer_id, MMSFBSurfacePixelFormat *pf)
{
    // is initialized?
    INITCHECK;

    // is layer initialized?
    if (!this->layers[layer_id].isinitialized)
        return false;

    // return pixelformat
    *pf = this->layers[layer_id].pixelformat;
    return true;
}

bool MMSFBL4Re::getFrameBufferBase(unsigned char **base)
{
    INITCHECK;
    *base = (unsigned char *)this->framebuffer_base;
    return true;
}

bool MMSFBL4Re::getFrameBufferPtr(int layer_id, MMSFBSurfacePlanesBuffer buffers, int *width, int *height)
{
    // is initialized?
    INITCHECK;

    // is layer initialized?
    if (!this->layers[layer_id].isinitialized) {
        return false;
    }

    // return buffer infos
    if (buffers)
        memcpy(buffers, this->layers[layer_id].buffers, sizeof(this->layers[layer_id].buffers));
    *width = this->layers[layer_id].width;
    *height = this->layers[layer_id].height;

    return true;
}

void MMSFBL4Re::genFBPixelFormat(MMSFBSurfacePixelFormat pf, unsigned int *nonstd_format, MMSFBPixelDef *pixeldef) {

    // generate std format
    if (nonstd_format) *nonstd_format = 0;
    getBitsPerPixel(pf, pixeldef);

    // try to get a fb specific format
    this->onGenFBPixelFormat.emit(pf, nonstd_format, pixeldef);
}

//
bool MMSFBL4Re::setMode(int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer)
{
    // is initialized?
    INITCHECK;

    if (width <= 0 || height <= 0) {
        // have to disable the framebuffer
        //disable(this->fd, this->device_file);
        return true;
    }

    // get bits per pixel and its length/offset
    unsigned int nonstd_format;
    MMSFBPixelDef pixeldef;
    genFBPixelFormat(pixelformat, &nonstd_format, &pixeldef);

    buildPixelFormat();

    return true;
}

#endif // __HAVE_L4_FB__
