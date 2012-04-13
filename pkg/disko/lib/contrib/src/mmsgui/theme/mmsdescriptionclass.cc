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

#include "mmsgui/theme/mmsdescriptionclass.h"

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_DESCRIPTION_ATTR_I[] = MMSGUI_DESCRIPTION_ATTR_INIT;


MMSDescriptionClass::MMSDescriptionClass() {
    unsetAll();
}

void MMSDescriptionClass::unsetAll() {
    unsetAuthor();
    unsetEmail();
    unsetDesc();
}

void MMSDescriptionClass::setAttributesFromTAFF(MMSTaffFile *tafff) {
	startTAFFScan
	{
		switch (attrid) {
		case MMSGUI_DESCRIPTION_ATTR::MMSGUI_DESCRIPTION_ATTR_IDS_author:
			setAuthor(attrval_str);
			break;
		case MMSGUI_DESCRIPTION_ATTR::MMSGUI_DESCRIPTION_ATTR_IDS_email:
			setEmail(attrval_str);
			break;
		case MMSGUI_DESCRIPTION_ATTR::MMSGUI_DESCRIPTION_ATTR_IDS_desc:
			setDesc(attrval_str);
			break;
		}
	}
	endTAFFScan
}

bool MMSDescriptionClass::isAuthor() {
    return this->isauthor;
}

void MMSDescriptionClass::setAuthor(string author) {
    this->author = author;
    this->isauthor = true;
}

void MMSDescriptionClass::unsetAuthor() {
    this->isauthor = false;
}

string MMSDescriptionClass::getAuthor() {
    return this->author;
}

bool MMSDescriptionClass::isEmail() {
    return this->isemail;
}

void MMSDescriptionClass::setEmail(string email) {
    this->email = email;
    this->isemail = true;
}

void MMSDescriptionClass::unsetEmail() {
    this->isemail = false;
}

string MMSDescriptionClass::getEmail() {
    return this->email;
}

bool MMSDescriptionClass::isDesc() {
    return this->isdesc;
}

void MMSDescriptionClass::setDesc(string desc) {
    this->desc = desc;
    this->isdesc = true;
}

void MMSDescriptionClass::unsetDesc() {
    this->isdesc = false;
}

string MMSDescriptionClass::getDesc() {
    return this->desc;
}

