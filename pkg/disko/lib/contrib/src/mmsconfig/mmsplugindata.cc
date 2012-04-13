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

MMSPluginData::MMSPluginData() :
    id(-1),
    type(NULL),
    name(""),
    title(""),
    description(""),
    path(""),
    filename(""),
    active(false),
    icon(""),
    selectedicon(""),
    smallicon(""),
    smallselectedicon(""),
    importProperties(NULL),
    category(NULL),
    orderpos(-1),
    version("") {
}

MMSPluginData::MMSPluginData(const MMSPluginData &pd) :
    id(pd.id),
    type(pd.type),
    name(pd.name),
    title(pd.title),
    description(pd.description),
    path(pd.path),
    filename(pd.filename),
    active(pd.active),
    icon(pd.icon),
    selectedicon(pd.selectedicon),
    smallicon(pd.smallicon),
    smallselectedicon(pd.smallselectedicon),
    properties(pd.properties),
    importProperties(pd.importProperties),
    category(pd.category),
    orderpos(pd.orderpos),
    version(pd.version) {
}

MMSPluginData& MMSPluginData::operator=(const MMSPluginData &pd) {
	this->id                = pd.id;
	this->type              = pd.type;
	this->name              = pd.name;
	this->title             = pd.title;
	this->description       = pd.description;
	this->path              = pd.path;
	this->filename          = pd.filename;
	this->active            = pd.active;
	this->icon              = pd.icon;
	this->selectedicon      = pd.selectedicon;
	this->smallicon         = pd.smallicon;
	this->smallselectedicon = pd.smallselectedicon;
	this->properties        = pd.properties;
	this->importProperties  = pd.importProperties;
	this->category          = pd.category;
	this->orderpos			= pd.orderpos;
	this->version			= pd.version;

	return *this;
}

MMSPluginData::~MMSPluginData() {
	this->properties.clear();
}

int MMSPluginData::getId() {
    return this->id;
}

void MMSPluginData::setId(int id) {
    this->id = id;
}

MMSPluginTypeData *MMSPluginData::getType() {
    return this->type;
}

void MMSPluginData::setType(MMSPluginTypeData *type) {
    this->type = type;
}

string MMSPluginData::getName() {
    return this->name;
}
void MMSPluginData::setName(string name) {
    this->name = name;
}

string MMSPluginData::getTitle() {
    return this->title;
}
void MMSPluginData::setTitle(string title) {
    this->title = title;
}

string MMSPluginData::getDescription() {
    return this->description;
}
void MMSPluginData::setDescription(string description) {
    this->description = description;
}

string MMSPluginData::getPath() {
    return this->path;
}
void MMSPluginData::setPath(string path) {
    this->path = path;
}

string MMSPluginData::getFilename() {
    return this->filename;
}

void MMSPluginData::setFilename(string filename) {
    this->filename = filename;
}

bool MMSPluginData::getActive() {
    return this->active;
}

void MMSPluginData::setActive(bool active) {
    this->active = active;
}

string MMSPluginData::getIcon() {
    return this->icon;
}

void MMSPluginData::setIcon(string icon) {
    this->icon = icon;
}

string MMSPluginData::getSelectedIcon() {
    return this->selectedicon;
}

void MMSPluginData::setSelectedIcon(string icon) {
    this->selectedicon = icon;
}

string MMSPluginData::getSmallIcon() {
    return this->smallicon;
}

void MMSPluginData::setSmallIcon(string icon) {
    this->smallicon = icon;
}

string MMSPluginData::getSmallSelectedIcon() {
    return this->smallselectedicon;
}

void MMSPluginData::setSmallSelectedIcon(string icon) {
    this->smallselectedicon = icon;
}

vector<MMSPropertyData *>  MMSPluginData::getProperties() {
    return this->properties;
}

void MMSPluginData::setProperties(vector <MMSPropertyData *> props) {
    this->properties = props;
}

MMSImportPropertyData *MMSPluginData::getImportProperties() {
    return this->importProperties;
}

void MMSPluginData::setImportProperties(MMSImportPropertyData *props) {
    this->importProperties = props;
}

MMSPluginCategoryData *MMSPluginData::getCategory() {
    return this->category;
}

void MMSPluginData::setCategory(MMSPluginCategoryData *category){
    this->category = category;
}


MMSPropertyData*  MMSPluginData::getProperty(string name) {

    for(vector<MMSPropertyData*>::iterator it=properties.begin();it!=properties.end();it++) {

        if ((*it)->getParameter()==name)
            return (*it);
    }
    return NULL;
}

int MMSPluginData::getOrderpos() {
	return this->orderpos;
}

void MMSPluginData::setOrderpos(int orderpos) {
	this->orderpos = orderpos;
}

string MMSPluginData::getVersion() {
	return this->version;
}

void MMSPluginData::setVersion(string version) {
	this->version = version;
}
