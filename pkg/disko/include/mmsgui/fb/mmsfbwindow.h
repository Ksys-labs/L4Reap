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

#ifndef MMSFBWINDOW_H_
#define MMSFBWINDOW_H_

#include "mmstools/mmslogger.h"
#include "mmsgui/fb/mmsfbbase.h"
#include "mmsgui/fb/mmsfbsurface.h"

typedef struct {
    MMSFBSurfaceConfig  surface_config; /* windows surface config */
    int                 posx;           /* pos x */
    int                 posy;           /* pos y */
    unsigned char       opacity;        /* opacity of the window */
    bool                shown;          /* is the window currently shown */
} MMSFBWindowConfig;

//! This class describes a window on a specific layer.
/*!
\author Jens Schneider
*/
class MMSFBWindow {
    private:
#ifdef  __HAVE_DIRECTFB__
    	//! dfb window if used
        IDirectFBWindow     *dfbwindow;
#endif

        MMSFBSurface        *surface;   /* windows surface */

        MMSFBWindowConfig   config;     /* surface configuration */

    public:
#ifdef USE_DFB_WINMAN
        MMSFBWindow(IDirectFBWindow *dfbwindow, int x, int y);
#endif
#ifdef USE_MMSFB_WINMAN
        MMSFBWindow(MMSFBSurface *surface, int x, int y);
#endif
        virtual ~MMSFBWindow();

        bool getSurface(MMSFBSurface **surface);

        bool getConfiguration(MMSFBWindowConfig *config = NULL);

        bool isShown();
        bool show();
        bool hide();

        bool getOpacity(unsigned char *opacity);
        bool setOpacity(unsigned char opacity);

        bool getPosition(int *x, int *y);
        bool moveTo(int x, int y, bool move_vrect = false);

        bool getSize(int *w, int *h);
        bool resize(int w, int h);

        bool raiseToTop(int zlevel = 0);
        bool lowerToBottom();

        bool setVisibleRectangle(MMSFBRectangle *rect = NULL);
        bool getVisibleRectangle(MMSFBRectangle *rect);

        bool getScreenshot();
};

#endif /*MMSFBWINDOW_H_*/
