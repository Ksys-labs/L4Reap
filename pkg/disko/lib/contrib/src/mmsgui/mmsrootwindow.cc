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

#include "mmsgui/mmsrootwindow.h"

MMSRootWindow::MMSRootWindow(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                             MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface, bool *backbuffer) {
    create(className, dx, dy, w, h, alignment, flags, theme, own_surface, backbuffer);
}

MMSRootWindow::MMSRootWindow(string className, string w, string h, MMSALIGNMENT alignment,
                             MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface, bool *backbuffer) {
    create(className, "", "", w, h, alignment, flags, theme, own_surface, backbuffer);
}

MMSRootWindow::~MMSRootWindow() {
}

bool MMSRootWindow::create(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                           MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface, bool *backbuffer) {
	this->type = MMSWINDOWTYPE_ROOTWINDOW;
    this->className = className;
    if (theme) this->theme = theme; else this->theme = globalTheme;
    this->rootWindowClass = this->theme->getRootWindowClass(className);
    this->baseWindowClass = &(this->theme->rootWindowClass.windowClass);
    if (this->rootWindowClass) this->windowClass = &(this->rootWindowClass->windowClass); else  this->windowClass = NULL;

    return MMSWindow::create(dx, dy, w, h, alignment, flags, own_surface, backbuffer);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSRootWindow::updateFromThemeClass(MMSRootWindowClass *themeClass) {
    MMSWindow::updateFromThemeClass(&(themeClass->windowClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
