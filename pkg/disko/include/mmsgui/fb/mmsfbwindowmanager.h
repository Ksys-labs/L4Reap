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

#ifndef MMSFBWINDOWMANAGER_H_
#define MMSFBWINDOWMANAGER_H_

#include "mmstools/tools.h"
#include "mmstools/mmslogger.h"
#include "mmsgui/fb/mmsfblayer.h"
#include "mmsgui/fb/mmsfbwindowmanagerthread.h"

typedef struct {
	//! window pointer
    MMSFBWindow     *window;
    //! visible rectangle, can be set if only a part of the window should be displayed
    MMSFBRectangle	vrect;
} AVAILABLE_WINDOWS;

typedef struct {
	//! window pointer
    MMSFBWindow     *window;
    //! surface of the window
    MMSFBSurface    *surface;
    //! visible rectangle, can be set if only a part of the window should be displayed
    MMSFBRectangle	vrect;
    //! region of the window within layer (if vrect is used, the region is the visible region)
    MMSFBRegion     region;
    //! use of alpha value
    bool            alphachannel;
    //! opacity of the window
    unsigned char   opacity;
    //! last flip time in milliseconds
    int             lastflip;
    //! the window works direct on the layer (old staff, to be removed)
    bool            islayersurface;
    //! copy of surface if window works direct on the layer (old staff, to be removed)
    MMSFBSurface    *saved_surface;
} VISIBLE_WINDOWS;

//! Manager for MMSFBWindows will be instantiated once in an application.
/*!
\author Jens Schneider
*/
class MMSFBWindowManager {
    private:
        MMSFBLayer      		*layer;         /* layer on which the windows will be drawn */
        MMSFBSurface    		*layer_surface; /* layer's surface */
        MMSFBSurfacePixelFormat layer_pixelformat;

        // destination surface for flipSurface() method
        MMSFBSurface    		*dst_surface;

        vector<AVAILABLE_WINDOWS> windows;  /* a list of created windows */

        vector<VISIBLE_WINDOWS> vwins;      /* a list of currently visible windows */

        MMSFBSurface    *high_freq_surface; /* surface which will flipped with high frequency */
        MMSFBSurface    *high_freq_saved_surface;
        MMSFBRegion     high_freq_region;   /* rectangle which will flipped with high frequency */
        int             high_freq_lastflip; /* last flip time of the high_freq_region */

        bool 			show_pointer;
        int				pointer_posx;
        int				pointer_posy;
        MMSFBRectangle	pointer_rect;
        MMSFBRegion		pointer_region;
        MMSFBSurface	*pointer_surface;
        unsigned char	pointer_opacity;
        bool			button_pressed;
        int				pointer_fadecnt;

        MMSMutex lock;            /* to make it thread-safe */

        MMSFBWindowManagerThread *mmsfbwinmanthread;

        MMSFBSurfacePixelFormat	pixelformat;	// pixelformat for all my images
        bool					usetaff;		// use the taff (image) format?
        MMSTAFF_PF				taffpf;			// pixelformat for the taff converter

        //! set to true if disko is running in OpenGL mode (GL/GLES)
        bool	ogl_mode;

        void lockWM();
        void unlockWM();

        bool addWindow(MMSFBWindow *window);
        bool removeWindow(MMSFBWindow *window);

        bool raiseToTop(MMSFBWindow *window, int zlevel = 0);
        bool lowerToBottom(MMSFBWindow *window);

        bool loadWindowConfig(MMSFBWindow *window, VISIBLE_WINDOWS *vwin);

        bool showWindow(MMSFBWindow *window, bool locked = false, bool refresh = true);
        bool hideWindow(MMSFBWindow *window, bool locked = false, bool refresh = true);

        bool flipSurface(MMSFBSurface *surface, MMSFBRegion *region = NULL,
                         bool locked = false, bool refresh = true);

        bool setWindowOpacity(MMSFBWindow *window);
        bool setWindowPosition(MMSFBWindow *window, MMSFBRectangle *vrect = NULL);
        bool setWindowSize(MMSFBWindow *window, int w, int h);

        bool setWindowVisibleRectangle(MMSFBWindow *window, MMSFBRectangle *rect = NULL);
        bool getWindowVisibleRectangle(MMSFBWindow *window, MMSFBRectangle *rect);
        bool getScreenshot(MMSFBWindow *window);

        bool loadPointer();
        void drawPointer(MMSFBRegion *region);
        unsigned char getPointerOpacity();
        void setPointerOpacity(unsigned char opacity);
        void fadePointer();

    public:
        MMSFBWindowManager();
        virtual ~MMSFBWindowManager();

        bool init(MMSFBLayer *layer, bool show_pointer = false);
        bool reset();
        bool getLayer(MMSFBLayer **layer);

        void setPointerPosition(int pointer_posx, int pointer_posy, bool pressed = false);
        bool getPointerPosition(int &pointer_posx, int &pointer_posy);

    friend class MMSFBLayer;
    friend class MMSFBSurface;
    friend class MMSFBWindow;
    friend class MMSFBWindowManagerThread;
};

/* access to global mmsfbwindowmanager */
extern MMSFBWindowManager *mmsfbwindowmanager;

#endif /*MMSFBWINDOWMANAGER_H_*/
