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

#include "mmsgui/theme/mmspopupwindowclass.h"

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_POPUPWINDOW_ATTR_I[] = MMSGUI_POPUPWINDOW_ATTR_INIT;


MMSPopupWindowClass::MMSPopupWindowClass() {
    unsetAll();
}

void MMSPopupWindowClass::unsetAll() {
    this->className  = "";
    unsetDuration();
}

void MMSPopupWindowClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *path, bool reset_paths) {
	startTAFFScan
	{
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
            setClassName(attrval_str);
			break;
		case MMSGUI_POPUPWINDOW_ATTR::MMSGUI_POPUPWINDOW_ATTR_IDS_duration:
            setDuration(attrval_int);
            break;
		}
	}
	endTAFFScan
}

void MMSPopupWindowClass::setClassName(string className) {
    this->className = className;
}

string MMSPopupWindowClass::getClassName() {
    return this->className;
}

bool MMSPopupWindowClass::isDuration() {
    return this->isduration;
}

void MMSPopupWindowClass::setDuration(unsigned int duration) {
    this->duration = duration;
    this->isduration = true;
}

void MMSPopupWindowClass::unsetDuration() {
    this->isduration = false;
}

unsigned int MMSPopupWindowClass::getDuration() {
    return this->duration;
}

