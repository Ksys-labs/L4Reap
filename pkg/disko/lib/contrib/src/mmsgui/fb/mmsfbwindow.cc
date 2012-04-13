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

#include "mmsgui/fb/mmsfbwindow.h"
#include "mmsgui/fb/mmsfbwindowmanager.h"

/*****************************************************************************/
/* Use DFB Window Manager                                                    */
/*****************************************************************************/
#ifdef USE_DFB_WINMAN

#define INITCHECK  if(!this->dfbwindow){MMSFB_SetError(0,"not initialized");return false;}

MMSFBWindow::MMSFBWindow(IDirectFBWindow *dfbwindow, int x, int y) {
    /* init me */
    this->dfbwindow = dfbwindow;
    this->surface = NULL;

    /* get the current config */
    if (this->dfbwindow) {
        getConfiguration();
        this->dfbwindow->SetOpacity(this->dfbwindow, 0);
        this->config.posx = x;
        this->config.posy = y;
        this->config.opacity = 255;
        this->config.shown = false;
    }
}

MMSFBWindow::~MMSFBWindow() {
    if (this->dfbwindow)
        this->dfbwindow->Release(this->dfbwindow);
}

bool MMSFBWindow::getSurface(MMSFBSurface **surface) {
    DFBResult           dfbres;
    IDirectFBSurface    *dfbsurface;

    /* check if initialized */
    INITCHECK;

    if (this->surface) {
        /* i have already a surface */
        *surface = this->surface;
        return true;
    }

    /* get windows surface */
    if ((dfbres=this->dfbwindow->GetSurface(this->dfbwindow, &dfbsurface)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::GetSurface() failed");
        return false;
    }

    /* create a new surface instance */
    *surface = new MMSFBSurface(dfbsurface);
    if (!*surface) {
        dfbsurface->Release(dfbsurface);
        MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
        return false;
    }

    /* save this for the next call */
    this->surface = *surface;

    return true;
}

bool MMSFBWindow::getConfiguration(MMSFBWindowConfig *config) {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* get windows surface */
    MMSFBSurface *s;
    if (!getSurface(&s))
        return false;

    /* get surface config */
    if (!this->surface->getConfiguration(&(this->config.surface_config)))
        return false;

    /* fill return config */
    if (config)
        *config = this->config;

    return true;
}

bool MMSFBWindow::isShown() {

    /* check if initialized */
    INITCHECK;

    /* return the shown flag */
    return this->config.shown;
}

bool MMSFBWindow::show() {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* already shown */
    if (this->config.shown)
        return true;

    /* set the opacity */
    if ((dfbres=this->dfbwindow->SetOpacity(this->dfbwindow, this->config.opacity)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::SetOpacity(" + iToStr(this->config.opacity) + ") failed");
        return false;
    }

    this->config.shown = true;
    return true;
}

bool MMSFBWindow::hide() {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* already hidden */
    if (!this->config.shown)
        return true;

    /* set the opacity */
    if ((dfbres=this->dfbwindow->SetOpacity(this->dfbwindow, 0)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::SetOpacity(0) failed");
        return false;
    }

    this->config.shown = false;
    return true;
}

bool MMSFBWindow::getOpacity(unsigned char *opacity) {

    /* check if initialized */
    INITCHECK;

    /* return the opacity */
    *opacity = this->config.opacity;

    return true;
}

bool MMSFBWindow::setOpacity(unsigned char opacity) {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    if (this->config.shown) {
        /* window is shown, set the opacity */
        if ((dfbres=this->dfbwindow->SetOpacity(this->dfbwindow, opacity)) != DFB_OK) {
            MMSFB_SetError(dfbres, "IDirectFBWindow::SetOpacity(" + iToStr(opacity) + ") failed");
            return false;
        }
    }

    /* save opacity */
    this->config.opacity = opacity;

    return true;
}

bool MMSFBWindow::getPosition(int *x, int *y) {

    /* check if initialized */
    INITCHECK;

    /* return the position */
    *x = this->config.posx;
    *y = this->config.posy;

    return true;
}

bool MMSFBWindow::moveTo(int x, int y) {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* move window */
    if ((dfbres=this->dfbwindow->MoveTo(this->dfbwindow, x, y)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::MoveTo(" + iToStr(x) + "," + iToStr(y) + ") failed");
        return false;
    }

    /* save the position */
    this->config.posx = x;
    this->config.posy = y;

    return true;
}

bool MMSFBWindow::getSize(int *w, int *h) {

    /* check if initialized */
    INITCHECK;

    /* return the size */
    *w = this->config.surface_config.w;
    *h = this->config.surface_config.h;

    return true;
}

bool MMSFBWindow::resize(int w, int h) {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* resize window */
    if ((dfbres=this->dfbwindow->Resize(this->dfbwindow, w, h)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::Resize(" + iToStr(w) + "x" + iToStr(h) + ") failed");
        return false;
    }

    /* get surface config */
    if (!this->surface->getConfiguration(&this->config.surface_config))
        return false;

    return true;
}

bool MMSFBWindow::raiseToTop(int zlevel) {
    DFBResult   dfbres;

//TODO: zlevel does not work for DFB

    /* check if initialized */
    INITCHECK;

    /* raise */
    if ((dfbres=this->dfbwindow->RaiseToTop(this->dfbwindow)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::RaiseToTop() failed");
        return false;
    }
    return true;
}

bool MMSFBWindow::lowerToBottom() {
    DFBResult   dfbres;

    /* check if initialized */
    INITCHECK;

    /* lower */
    if ((dfbres=this->dfbwindow->LowerToBottom(this->dfbwindow)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBWindow::LowerToBottom() failed");
        return false;
    }
    return true;
}

#endif


/*****************************************************************************/
/* Use MMSFB Window Manager                                                  */
/*****************************************************************************/
#ifdef USE_MMSFB_WINMAN

#define INITCHECK  if(!this->surface){MMSFB_SetError(0,"not initialized");return false;}

MMSFBWindow::MMSFBWindow(MMSFBSurface *surface, int x, int y) {
    /* init me */
#ifdef  __HAVE_DIRECTFB__
    this->dfbwindow = NULL;
#endif
    this->surface = surface;

    /* get the current config */
    if (this->surface) {
        getConfiguration();
        this->config.posx = x;
        this->config.posy = y;
        this->config.opacity = 255;
        this->config.shown = false;
    }
}

MMSFBWindow::~MMSFBWindow() {
    if (this->surface)
        delete this->surface;
}

bool MMSFBWindow::getSurface(MMSFBSurface **surface) {

    /* check if initialized */
    INITCHECK;

    /* return the surface */
    *surface = this->surface;

    return true;
}

bool MMSFBWindow::getConfiguration(MMSFBWindowConfig *config) {

    /* check if initialized */
    INITCHECK;

    /* get surface config */
    if (!this->surface->getConfiguration(&(this->config.surface_config)))
        return false;

    /* fill return config */
    if (config)
        *config = this->config;

    return true;
}

bool MMSFBWindow::isShown() {

    /* check if initialized */
    INITCHECK;

    /* return the shown flag */
    return this->config.shown;
}

bool MMSFBWindow::show() {

    /* check if initialized */
    INITCHECK;

    /* already shown */
    if (this->config.shown)
        return true;

    /* set the shown flag */
    this->config.shown = true;

    /* inform the window manager */
    mmsfbwindowmanager->showWindow(this);

    return true;
}

bool MMSFBWindow::hide() {

    /* check if initialized */
    INITCHECK;

    /* already hidden */
    if (!this->config.shown)
        return true;

    /* clear the shown flag */
    this->config.shown = false;

    /* inform the window manager */
    mmsfbwindowmanager->hideWindow(this);

    return true;
}

bool MMSFBWindow::getOpacity(unsigned char *opacity) {

    /* check if initialized */
    INITCHECK;

    /* return the opacity */
    *opacity = this->config.opacity;

    return true;
}

bool MMSFBWindow::setOpacity(unsigned char opacity) {

    /* check if initialized */
    INITCHECK;

    /* save opacity */
    this->config.opacity = opacity;

    /* inform the window manager */
    mmsfbwindowmanager->setWindowOpacity(this);

    return true;
}

bool MMSFBWindow::getPosition(int *x, int *y) {

    /* check if initialized */
    INITCHECK;

    /* return the position */
    *x = this->config.posx;
    *y = this->config.posy;

    return true;
}

bool MMSFBWindow::moveTo(int x, int y, bool move_vrect) {

    // check if initialized
    INITCHECK;

    // get visible rectangle
    MMSFBRectangle vrect;
    if (move_vrect) {
		if (getVisibleRectangle(&vrect)) {
			vrect.x+= this->config.posx - x;
			vrect.y+= this->config.posy - y;
		}
		else
			move_vrect = false;
    }

    // save the position
    this->config.posx = x;
    this->config.posy = y;

	// inform the window manager
	if (move_vrect)
		mmsfbwindowmanager->setWindowPosition(this, &vrect);
	else
		mmsfbwindowmanager->setWindowPosition(this);

    return true;
}

bool MMSFBWindow::getSize(int *w, int *h) {

    /* check if initialized */
    INITCHECK;

    /* return the size */
    *w = this->config.surface_config.w;
    *h = this->config.surface_config.h;

    return true;
}

bool MMSFBWindow::resize(int w, int h) {

    /* check if initialized */
    INITCHECK;

    /* inform the window manager */
    mmsfbwindowmanager->setWindowSize(this, w, h);

    return true;
}

bool MMSFBWindow::raiseToTop(int zlevel) {

    /* check if initialized */
    INITCHECK;

    /* raise to top of the window stack */
    mmsfbwindowmanager->raiseToTop(this, zlevel);

    return true;
}

bool MMSFBWindow::lowerToBottom() {

    /* check if initialized */
    INITCHECK;

    /* lower to bottom of the window stack */
    mmsfbwindowmanager->lowerToBottom(this);

    return true;
}

bool MMSFBWindow::setVisibleRectangle(MMSFBRectangle *rect) {

    // check if initialized
    INITCHECK;

    // inform the window manager
    return mmsfbwindowmanager->setWindowVisibleRectangle(this, rect);
}

bool MMSFBWindow::getVisibleRectangle(MMSFBRectangle *rect) {

    // check if initialized
    INITCHECK;

    // get it from the window manager
    return mmsfbwindowmanager->getWindowVisibleRectangle(this, rect);
}

bool MMSFBWindow::getScreenshot() {

    // check if initialized
    INITCHECK;

    // get it from the window manager
    return mmsfbwindowmanager->getScreenshot(this);
}

#endif

