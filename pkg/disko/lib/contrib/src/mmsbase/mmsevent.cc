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

#include "mmsbase/mmsevent.h"

MMSEvent::MMSEvent(string heading) {
    this->heading = heading;
}

void MMSEvent::setHeading(string heading) {
    this->heading=heading;
}

string MMSEvent::getHeading() {
    return this->heading;
}

string MMSEvent::getData(string key) {
	std::map<string, string>::iterator it;
	it = data.find(key);
	if(it!=data.end())
    	return it->second;
    else
    	return "";
}

void MMSEvent::setData(string key, string value) {
	data.insert(std::make_pair(key,value));
}

void MMSEvent::clear() {
	data.clear();
}

void MMSEvent::send() {
    this->dispatcher->raise(this);
}

void MMSEvent::sendTo(int pluginid) {
    this->dispatcher->raise(this,pluginid);
}

void MMSEvent::setDispatcher(IMMSEventDispatcher *dispatcher) {
    this->dispatcher = dispatcher;
}

IMMSEventDispatcher *MMSEvent::dispatcher = NULL;
