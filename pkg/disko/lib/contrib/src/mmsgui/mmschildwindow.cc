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

#include "mmsgui/mmschildwindow.h"

MMSChildWindow::MMSChildWindow(string className, MMSWindow *parent,
                               string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                               MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface,
                               bool *backbuffer) {
    create(className, parent, dx, dy, w, h, alignment, flags, theme, own_surface, backbuffer);
}

MMSChildWindow::MMSChildWindow(string className, MMSWindow *parent, string w, string h, MMSALIGNMENT alignment,
                               MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface,
                               bool *backbuffer) {
    create(className, parent, "", "", w, h, alignment, flags, theme, own_surface, backbuffer);
}

MMSChildWindow::~MMSChildWindow() {
}

bool MMSChildWindow::create(string className, MMSWindow *parent,
                            string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                            MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface, bool *backbuffer) {
	this->type = MMSWINDOWTYPE_CHILDWINDOW;
    this->className = className;
    if (theme) this->theme = theme; else this->theme = globalTheme;
    this->childWindowClass = this->theme->getChildWindowClass(className);
    this->baseWindowClass = &(this->theme->childWindowClass.windowClass);
    if (this->childWindowClass) this->windowClass = &(this->childWindowClass->windowClass); else this->windowClass = NULL;

//printf("create child window %x with parent = %x\n", this, parent);
    this->parent = parent;

    return MMSWindow::create(dx, dy, w, h, alignment, flags, own_surface, backbuffer);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

#define GETCHILDWINDOW(x,y) \
    if (this->myChildWindowClass.is##x()) return myChildWindowClass.get##x(y); \
    else if ((childWindowClass)&&(childWindowClass->is##x())) return childWindowClass->get##x(y); \
    else return this->theme->childWindowClass.get##x(y);

void MMSChildWindow::updateFromThemeClass(MMSChildWindowClass *themeClass) {
    MMSWindow::updateFromThemeClass(&(themeClass->windowClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
