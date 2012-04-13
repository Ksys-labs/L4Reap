/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifndef MMSFBLAYER_H_
#define MMSFBLAYER_H_

#include "mmstools/mmslogger.h"
#include "mmsgui/fb/mmsfbbase.h"
#include "mmsgui/fb/mmsfbsurface.h"
#include "mmsgui/fb/mmsfbwindow.h"

#ifdef __HAVE_XLIB__
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>

typedef struct {
	Display *x_display;
	Window x_window;
	GC x_gc;
	int xv_port;
#ifdef __HAVE_XV__
	XvImage *xv_image1;
	XvImage *xv_image2;
#endif
	int w;
	int h;
	int x_screen;
} X11_IMPL;
#endif

//! describes the config of a layer
typedef struct {
	//! config available?
	bool	avail;
	//! layer's id
	int 	id;
	//! used backend, normally it is not layer specific
	MMSFBBackend backend;
	//! used outputtype, can be layer specific
	MMSFBOutputType outputtype;
	//! width
    int     w;
    //! height
    int     h;
    //! pixelformat
    MMSFBSurfacePixelFormat pixelformat;
    //! buffer mode
    string  buffermode;
    //! options
    string  options;
    //! pixelformat for windows
    MMSFBSurfacePixelFormat window_pixelformat;
    //! pixelformat for surfaces
    MMSFBSurfacePixelFormat surface_pixelformat;
} MMSFBLayerConfig;

//! This class describes a display layer.
/*!
\author Jens Schneider
*/
class MMSFBLayer {
    private:
    	//! is initialized?
    	bool initialized;

#ifdef  __HAVE_DIRECTFB__
    	//! interface to the dfb layer
        IDirectFBDisplayLayer   *dfblayer;
#endif

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
        MMSFBSurface	*mmsfbdev_surface;
#endif

#ifdef __HAVE_XLIB__
        XImage  		  *x_image1;
        XShmSegmentInfo   x_shminfo1;
        XImage  		  *x_image2;
        XShmSegmentInfo   x_shminfo2;
        XImage  		  *x_image_scaler;
        XShmSegmentInfo   x_shminfo_scaler;
        MMSFBSurface	  *scaler;
        Visual			  *x_visual;
        Window			  x_window;
        Pixmap 			  pixmap;
        XRenderPictFormat *pict_format;
        Picture 		  x_pixmap_pict;
        Picture           x_window_pict;
        GC 				  x_gc;
        int				  x_window_h;
        int               x_window_w;
        X11_IMPL		  impl;

#endif
#ifdef __HAVE_XV__
        XvImage  		*xv_image1;
        XShmSegmentInfo xv_shminfo1;
        XvImage  		*xv_image2;
        XShmSegmentInfo xv_shminfo2;
#endif

        // layer configuration
        MMSFBLayerConfig        config;

        // layers surface
        MMSFBSurface            *surface;

        // flags which are used when flipping
        MMSFBFlipFlags			flipflags;

        // first time flag for createSurface()
        static bool 			firsttime_createsurface;

        // first time flags for createWindow()
        static bool 			firsttime_createwindow_usealpha;
        static bool 			firsttime_createwindow_noalpha;

    public:
        MMSFBLayer(int id, MMSFBBackend backend, MMSFBOutputType outputtype);
        virtual ~MMSFBLayer();

        bool isInitialized();

        bool getID(int *id);
        bool setExclusiveAccess();
        bool getConfiguration(MMSFBLayerConfig *config = NULL);
        bool getResolution(int *w, int *h);
        bool getPixelFormat(MMSFBSurfacePixelFormat *pixelformat);
        bool setConfiguration(int w=0, int h=0, MMSFBSurfacePixelFormat pixelformat=MMSFB_PF_NONE, string buffermode="", string options="",
							  MMSFBSurfacePixelFormat window_pixelformat=MMSFB_PF_NONE, MMSFBSurfacePixelFormat surface_pixelformat=MMSFB_PF_NONE);
        bool setOpacity(unsigned char opacity);
        bool setLevel(int level);
        bool getSurface(MMSFBSurface **surface, bool clear = false);

        bool setFlipFlags(MMSFBFlipFlags flags);

        bool releaseLayer();
        bool restoreLayer();

        bool createSurface(MMSFBSurface **surface, int w, int h,
						   MMSFBSurfacePixelFormat pixelformat = MMSFB_PF_NONE, int backbuffer = 0);
        bool createWindow(MMSFBWindow **window, int x, int y, int w, int h,
						   MMSFBSurfacePixelFormat pixelformat = MMSFB_PF_NONE,
                           bool usealpha = true, int backbuffer = 1);

        void *getImplementation();

		friend class MMSFBManager;
		friend class MMSFBSurface;
};

#endif /*MMSFBLAYER_H_*/
