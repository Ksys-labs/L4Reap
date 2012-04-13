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

#include "mmsconfig/mmsimportsourcedata.h"

MMSImportSourceData::MMSImportSourceData() {
    this->id = -1;
    this->pluginId = 0;
    this->name = "";
    this->source = "";
    this->lifeTime = 0;
}

MMSImportSourceData::~MMSImportSourceData() {
}

int MMSImportSourceData::getId() {
    return this->id;
}

void MMSImportSourceData::setId(int id) {
    this->id = id;
}

int MMSImportSourceData::getPluginId() {
    return this->pluginId;
}

void MMSImportSourceData::setPluginId(int pluginId) {
    this->pluginId = pluginId;
}

string MMSImportSourceData::getName() {
    return this->name;
}

void MMSImportSourceData::setName(string name) {
    this->name = name;
}

string MMSImportSourceData::getSource() {
    return this->source;
}

void MMSImportSourceData::setSource(string source) {
    this->source = source;
}

int MMSImportSourceData::getLifeTime() {
    return this->lifeTime;
}

void MMSImportSourceData::setLifeTime(int lifeTime) {
    this->lifeTime = lifeTime;
}
