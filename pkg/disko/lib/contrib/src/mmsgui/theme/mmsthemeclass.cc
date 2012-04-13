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

#include "mmsgui/theme/mmsthemeclass.h"

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_MMSTHEME_ATTR_I[] = MMSGUI_MMSTHEME_ATTR_INIT;


MMSThemeClass::MMSThemeClass() {
    unsetAll();
}

void MMSThemeClass::unsetAll() {
    unsetName();
    unsetFadeIn();
}

void MMSThemeClass::setAttributesFromTAFF(MMSTaffFile *tafff) {
	startTAFFScan
	{
		switch (attrid) {
		case MMSGUI_MMSTHEME_ATTR::MMSGUI_MMSTHEME_ATTR_IDS_name:
			setName(attrval_str);
			break;
		case MMSGUI_MMSTHEME_ATTR::MMSGUI_MMSTHEME_ATTR_IDS_fadein:
            setFadeIn((attrval_int) ? true : false);
            break;
		}
	}
	endTAFFScan
}

bool MMSThemeClass::isName() {
    return this->isname;
}

void MMSThemeClass::setName(string name) {
    this->name = name;
    this->isname = true;
}

void MMSThemeClass::unsetName() {
    this->isname = false;
}

string MMSThemeClass::getName() {
    return this->name;
}

bool MMSThemeClass::isFadeIn() {
    return this->isfadein;
}

void MMSThemeClass::setFadeIn(bool fadein) {
    this->fadein = fadein;
    this->isfadein = true;
}

void MMSThemeClass::unsetFadeIn() {
    this->isfadein = false;
}

bool MMSThemeClass::getFadeIn() {
    return this->fadein;
}

