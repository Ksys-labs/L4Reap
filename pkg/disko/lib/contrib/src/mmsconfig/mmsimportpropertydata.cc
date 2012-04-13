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

#include "mmsconfig/mmsplugindata.h"
#include "mmsconfig/mmsimportpropertydata.h"

MMSImportPropertyData::MMSImportPropertyData() {
    this->id = -1;
    this->pluginId = 0;
    this->onStartUp = false;
    this->time = 0;
    this->interval = 0;
}

MMSImportPropertyData::~MMSImportPropertyData() {
}

int MMSImportPropertyData::getId() {
    return this->id;
}

void MMSImportPropertyData::setId(int id) {
    this->id = id;
}

int MMSImportPropertyData::getPluginId() {
    return this->pluginId;
}

void MMSImportPropertyData::setPluginId(int pluginId) {
    this->pluginId = pluginId;
}

bool MMSImportPropertyData::getOnStartUp(void) {
    return this->onStartUp;
}

void MMSImportPropertyData::setOnStartUp(bool onStartUp) {
    this->onStartUp = onStartUp;
}

int MMSImportPropertyData::getTime(void) {
    return this->time;
}

void MMSImportPropertyData::setTime(int time) {
    this->time = time;
}

int MMSImportPropertyData::getInterval(void) {
    return this->interval;
}

void MMSImportPropertyData::setInterval(int interval) {
    this->interval = interval;
}
