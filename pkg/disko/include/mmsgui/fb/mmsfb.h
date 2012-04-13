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

#ifndef MMSFB_H_
#define MMSFB_H_

#include "mmstools/mmstypes.h"
#include "mmstools/mmslogger.h"

#include "mmsgui/fb/mmsfbdev.h"
#include "mmsgui/fb/mmsfbdevmatrox.h"
#include "mmsgui/fb/mmsfbdevdavinci.h"
#include "mmsgui/fb/mmsfbdevomap.h"
#include "mmsgui/fb/mmsfbl4re.h"
#include "mmsgui/fb/mmsfblayer.h"
#include "mmsgui/fb/mmsfbwindowmanager.h"
#include "mmsgui/fb/mmsfbfont.h"
#include "mmsgui/fb/mmsfbbackendinterface.h"

#ifdef __HAVE_OPENGL__
#define LOCK_OGL(fbo)	{ mmsfb->lock(); glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); glDisable(GL_SCISSOR_TEST); }
#define UNLOCK_OGL		{ glDisable(GL_SCISSOR_TEST); glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); mmsfb->unlock(); }
#endif

#define MMSFBLAYER_MAXNUM 32

//! The lowest layer to the backends like DFB, X11(XSHM/XVSHM) or FBDEV.
/*!
\author Jens Schneider
*/
class MMSFB {
    private:
        int              argc;       /* commandline arguments */
        char             **argv;
        string 			 applname;
        string			 appliconname;
        bool			 hidden;
        MMSFBPointerMode pointer;

        //! name of binary
        string           bin;

    	//! is initialized?
    	bool initialized;

        MMSFBFullScreenMode fullscreen;

#ifdef  __HAVE_DIRECTFB__
        // interface to dfb
        IDirectFB       *dfb;
#endif

#ifdef  __HAVE_FBDEV__
        // interface to own FB device
        MMSFBDev		*mmsfbdev;
#endif
#ifdef __HAVE_L4_FB__
        MMSFBL4Re       *mmsfbdev;
#endif

#ifdef __HAVE_XLIB__
        //! connection to the x-server
        Display 		*x_display;
        int				x_screen;
        Window 			x_window;
        Window 			input_window;
        GC 				x_gc;
        Visual			*x_visual;
        int				x_depth;
        MMSMutex		xlock;
        int				display_w;
        int				display_h;
        int				target_window_w;
        int				target_window_h;
        bool            resized;
        bool 			resizeWindow();
        Window			x_windows[MMSFBLAYER_MAXNUM];
        XImage			*rootimage;
#endif

#ifdef __HAVE_XV__
        int 		xv_port;
#endif

#ifdef __HAVE_OPENGL__
        // backend interface server needed for OPENGL
        MMSFBBackEndInterface	*bei;
#endif

        MMSFBLayer 		*layer[MMSFBLAYER_MAXNUM];

        MMSFBBackend	backend;
        MMSFBRectangle  x11_win_rect;

        //! to make it thread-safe
        MMSMutex  		Lock;

    public:
        MMSFB();
        virtual ~MMSFB();

        bool init(int argc, char **argv, MMSFBBackend backend, MMSFBRectangle x11_win_rect,
        		  bool extendedaccel, MMSFBFullScreenMode fullscreen, MMSFBPointerMode pointer,
				  string appl_name = "Disko Application", string appl_icon_name = "Disko Application", bool hidden=false);
        bool release();
        bool isInitialized();

        MMSFBBackend getBackend();

        bool lock();
        bool unlock();

        bool getLayer(int id, MMSFBLayer **layer, MMSFBOutputType outputtype, bool virtual_console);
        bool getLayer(int id, MMSFBLayer **layer);

        void *getX11Window();
        void *getX11Display();
        bool refresh();

        bool createSurface(MMSFBSurface **surface, int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0, bool systemonly = false);

#ifdef  __HAVE_DIRECTFB__
        bool createImageProvider(IDirectFBImageProvider **provider, string filename);
#endif
        bool createFont(MMSFBFont **font, string filename, int width = 0, int height = 0);


        void realignLayer();

    friend class MMSFBLayer;
    friend class MMSFBSurface;
    friend class MMSFBFont;
    friend class MMSInputX11Handler;
    friend class MMSInputLISHandler;
};

/* access to global mmsfb */
extern MMSFB *mmsfb;

#endif /*MMSFB_H_*/
