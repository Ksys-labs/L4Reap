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

#include "mmsgui/mmspopupwindow.h"

MMSPopupWindow::MMSPopupWindow(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                               MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface,
                               bool *backbuffer, unsigned int duration) {
	create(className, dx, dy, w, h, alignment, flags, theme, own_surface, backbuffer, duration);
}

MMSPopupWindow::MMSPopupWindow(string className, string w, string h, MMSALIGNMENT alignment,
                               MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface,
                               bool *backbuffer, unsigned int duration) {
	create(className, "", "", w, h, alignment, flags, theme, own_surface, backbuffer, duration);
}

MMSPopupWindow::~MMSPopupWindow() {
	this->timeOut_connection.disconnect();
}

bool MMSPopupWindow::create(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                            MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface,
                            bool *backbuffer, unsigned int duration) {
	this->type = MMSWINDOWTYPE_POPUPWINDOW;
    this->className = className;
    if (theme) this->theme = theme; else this->theme = globalTheme;
    this->popupWindowClass = this->theme->getPopupWindowClass(className);
    this->baseWindowClass = &(this->theme->popupWindowClass.windowClass);
    if (this->popupWindowClass) this->windowClass = &(this->popupWindowClass->windowClass); else this->windowClass = NULL;

    if (duration)
        setDuration(duration);

    // create single shot timer
	this->timer = new MMSTimer(true);
	this->timeOut_connection = this->timer->timeOut.connect(sigc::mem_fun(this, &MMSPopupWindow::timeOut));

	return MMSWindow::create(dx, dy, w, h, alignment, flags, own_surface, backbuffer);
}

void MMSPopupWindow::timeOut(void) {

	// timeout reached, hide the window
	hide(false, true);
}

void MMSPopupWindow::afterShowAction(MMSPulser *pulser) {

	// call default window routine
	MMSWindow::afterShowAction(pulser);

	// start the timer
	unsigned int duration = getDuration();
	if (duration > 0) {
		this->timer->start(duration * 1000);
	}
}

bool MMSPopupWindow::beforeHideAction(MMSPulser *pulser) {

	// stop the timer if running
	this->timer->stop();

	// call default window routine and return
	return MMSWindow::beforeHideAction(pulser);
}


/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETPOPUPWINDOW(x) \
    if (this->myPopupWindowClass.is##x()) return myPopupWindowClass.get##x(); \
    else if ((popupWindowClass)&&(popupWindowClass->is##x())) return popupWindowClass->get##x(); \
    else return this->theme->popupWindowClass.get##x();

unsigned int MMSPopupWindow::getDuration() {
    GETPOPUPWINDOW(Duration);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSPopupWindow::setDuration(unsigned int duration) {
    myPopupWindowClass.setDuration(duration);

    /* restart timer if already running */
	if(duration == 0) {
		this->timer->stop();
	} else {
		if(this->isShown()) {
			if(!this->timer->isRunning()) {
				this->timer->start(duration * 1000);
			} else {
				this->timer->restart();
			} 
		}
	}
}

void MMSPopupWindow::updateFromThemeClass(MMSPopupWindowClass *themeClass) {
    if (themeClass->isDuration())
        setDuration(themeClass->getDuration());

    MMSWindow::updateFromThemeClass(&(themeClass->windowClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
