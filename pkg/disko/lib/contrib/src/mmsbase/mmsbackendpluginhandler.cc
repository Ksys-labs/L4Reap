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

#include "mmsbase/mmsbackendpluginhandler.h"

MMSBackendPluginHandler::MMSBackendPluginHandler(MMSPluginData plugindata, bool autoload, IMMSBackendPlugin *_plugin) :
	loaded(false),
	initialized(false),
	plugindata(plugindata),
	plugin(_plugin),
	handler(NULL),
	switcher(NULL) {
	if(plugin)
		this->loaded = true;
	else if(autoload)
        this->load();
}

MMSBackendPluginHandler::~MMSBackendPluginHandler() {
    if (this->loaded) {
        delete this->plugin;
        if(this->handler) delete this->handler;
    }
}

void MMSBackendPluginHandler::invokeInitialize(void *data) {
    if (!this->loaded)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not loaded");
    if (this->initialized)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is already initialized");

    this->calllock.lock();
    this->initialized = this->plugin->initialize(this->plugindata, this->switcher);
    this->calllock.unlock();
}

void MMSBackendPluginHandler::invokeOnEvent(IMMSEvent event) {
    if (!this->loaded)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not loaded");
    if (!this->initialized)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not initialized");

    this->calllock.lock();
    this->plugin->onEvent(event);
    this->calllock.unlock();
}

void MMSBackendPluginHandler::invokeShutdown(void *data) {
    if (!this->loaded)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not loaded");
    if (!this->initialized)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not initialized");

    this->calllock.lock();
    this->plugin->shutdown();
    this->calllock.unlock();
}

bool MMSBackendPluginHandler::isLoaded() {
	return this->loaded;
}

bool MMSBackendPluginHandler::isInitialized() {
	return this->initialized;
}

void MMSBackendPluginHandler::load() {
    if (this->loaded)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is already loaded");

    this->handler = new MMSShlHandler(this->plugindata.getFilename());
    this->handler->open();
    NEWBACKENDPLUGIN_PROC newproc = (NEWBACKENDPLUGIN_PROC)this->handler->getFunction("newBackendPlugin");
    this->plugin = newproc();

    if(this->plugin)
    	this->loaded = true;
}

void MMSBackendPluginHandler::unload() {
    if (!this->loaded)
        throw MMSBackendPluginError(0,"Backend Plugin " + this->plugindata.getName() + " is not loaded");

    if(this->plugin) {
		delete this->plugin;
		this->plugin = NULL;
    }

    if(this->handler) {
		delete this->handler;
		this->handler = NULL;
    }

    this->loaded = false;
	this->initialized = false;
}

void MMSBackendPluginHandler::setPluginData(MMSPluginData plugindata) {
    this->plugindata = plugindata;
}

MMSPluginData MMSBackendPluginHandler::getPluginData() {
    return this->plugindata;
}

void MMSBackendPluginHandler::setSwitcherInterface(IMMSSwitcher *switcher) {
    this->switcher = switcher;
}

