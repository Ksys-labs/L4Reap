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

#include "mmsconfig/mmsconfig.h"
#include "mmscore/mmspluginmanager.h"
#include "mmsbase/interfaces/immsswitcher.h"

MMSPluginManager::MMSPluginManager() {
    MMSConfigData *config = new MMSConfigData();
    this->source          = new DataSource(config->getConfigDBDBMS(),
    		                               config->getConfigDBDatabase(),
    		                               config->getConfigDBAddress(),
    		                               config->getConfigDBPort(),
    		                               config->getConfigDBUser(),
    		                               config->getConfigDBPassword());
    delete config;

    this->service = new MMSPluginService(source);
}

MMSPluginManager::~MMSPluginManager() {
    if(this->source)  delete this->source;
    if(this->service) delete this->service;
	for(vector<MMSOSDPluginHandler*>::iterator it = this->osdPluginHandlers.begin(); it != this->osdPluginHandlers.end(); ++it)
		delete *it;
	for(vector<MMSCentralPluginHandler*>::iterator it = this->centralPluginHandlers.begin(); it != this->centralPluginHandlers.end(); ++it)
		delete *it;
	for(vector<MMSImportPluginHandler*>::iterator it = this->importPluginHandlers.begin(); it != this->importPluginHandlers.end(); ++it)
		delete *it;
	for(vector<MMSBackendPluginHandler*>::iterator it = this->backendPluginHandlers.begin(); it != this->backendPluginHandlers.end(); ++it)
		delete *it;
    this->osdPluginHandlers.clear();
    this->centralPluginHandlers.clear();
    this->importPluginHandlers.clear();
    this->backendPluginHandlers.clear();
}

void MMSPluginManager::registerStaticOSDPlugin(string name, IMMSOSDPlugin *plugin) {
	this->staticOSDPlugins[name] = plugin;
}

void MMSPluginManager::registerStaticCentralPlugin(string name, IMMSCentralPlugin *plugin) {
	this->staticCentralPlugins[name] = plugin;
}

void MMSPluginManager::registerStaticImportPlugin(string name, IMMSImportPlugin *plugin) {
	this->staticImportPlugins[name] = plugin;
}

void MMSPluginManager::registerStaticBackendPlugin(string name, IMMSBackendPlugin *plugin) {
	this->staticBackendPlugins[name] = plugin;
}

void MMSPluginManager::loadOSDPlugins() {
    vector<MMSPluginData *> data;

    if(!this->osdPluginHandlers.empty()) {
        throw MMSPluginManagerError(0,"OSD Plugins already loaded");
    }

    DEBUGMSG("MMSCore", "getOSDPlugins from service");
    data = this->service->getOSDPlugins();

    for(vector<MMSPluginData*>::iterator i = data.begin(); i !=  data.end(); ++i) {
        MMSOSDPluginHandler *myhandler;
        map<string, IMMSOSDPlugin*>::iterator iter = this->staticOSDPlugins.find((*i)->getName());
        if(iter != this->staticOSDPlugins.end())
        	myhandler = new MMSOSDPluginHandler(*(*i), true, iter->second);
        else
        	myhandler = new MMSOSDPluginHandler(*(*i), true);
        this->osdPluginHandlers.push_back(myhandler);
        DEBUGMSG("MMSCore", " %s", (*i)->getName().c_str());
    }
}

void MMSPluginManager::loadCentralPlugins() {
    vector<MMSPluginData *> data;

    if (!this->centralPluginHandlers.empty()) {
        throw MMSPluginManagerError(0,"Central Plugins already loaded");
    }

    data = this->service->getCentralPlugins();

    for(vector<MMSPluginData*>::iterator i = data.begin(); i !=  data.end(); ++i) {
        MMSCentralPluginHandler *myhandler;
        map<string, IMMSCentralPlugin*>::iterator iter = this->staticCentralPlugins.find((*i)->getName());
        if(iter != this->staticCentralPlugins.end())
        	myhandler = new MMSCentralPluginHandler(*(*i), true, iter->second);
        else
        	myhandler = new MMSCentralPluginHandler(*(*i), true);
        this->centralPluginHandlers.push_back(myhandler);
        DEBUGMSG("MMSCore", " %s", (*i)->getName().c_str());
    }
}

void MMSPluginManager::loadImportPlugins() {
    vector<MMSPluginData *> data;

    if (!this->importPluginHandlers.empty()) {
        throw MMSPluginManagerError(0,"Import Plugins already loaded");
    }

    data = this->service->getImportPlugins();

    for(vector<MMSPluginData*>::iterator i = data.begin(); i !=  data.end(); ++i) {
        MMSImportPluginHandler *myhandler;
        map<string, IMMSImportPlugin*>::iterator iter = this->staticImportPlugins.find((*i)->getName());
        if(iter != this->staticImportPlugins.end())
        	myhandler = new MMSImportPluginHandler(*(*i), true, iter->second);
        else
        	myhandler = new MMSImportPluginHandler(*(*i), true);
        this->importPluginHandlers.push_back(myhandler);
        DEBUGMSG("MMSCore", " %s", (*i)->getName().c_str());
    }
}

void MMSPluginManager::loadBackendPlugins() {
    vector<MMSPluginData *> data;

    if (!this->backendPluginHandlers.empty()) {
        throw MMSPluginManagerError(0,"Backend Plugins already loaded");
    }

    data = this->service->getBackendPlugins();

    for(vector<MMSPluginData*>::iterator i = data.begin(); i !=  data.end(); ++i) {
        MMSBackendPluginHandler *myhandler;
        map<string, IMMSBackendPlugin*>::iterator iter = this->staticBackendPlugins.find((*i)->getName());
        if(iter != this->staticBackendPlugins.end())
        	myhandler = new MMSBackendPluginHandler(*(*i), true, iter->second);
        else
        	myhandler = new MMSBackendPluginHandler(*(*i), true);
        this->backendPluginHandlers.push_back(myhandler);
        DEBUGMSG("MMSCore", " %s", (*i)->getName().c_str());
    }
}

void MMSPluginManager::initializeOSDPlugins() {
	for(vector<MMSOSDPluginHandler*>::iterator i = this->osdPluginHandlers.begin(); i != this->osdPluginHandlers.end(); ++i) {
        MMSPluginData pd = (*i)->getPluginData();
        (*i)->setSwitcherInterface(switcher->newSwitcher(&pd));
        (*i)->invokeInitialize();
    }
}

void MMSPluginManager::initializeCentralPlugins() {
	for(vector<MMSCentralPluginHandler*>::iterator i = this->centralPluginHandlers.begin(); i != this->centralPluginHandlers.end(); ++i) {
        MMSPluginData pd = (*i)->getPluginData();
        (*i)->setSwitcherInterface(switcher->newSwitcher(&pd));
        (*i)->invokeInitialize();
    }
}

void MMSPluginManager::initializeImportPlugins() {
	for(vector<MMSImportPluginHandler*>::iterator i = this->importPluginHandlers.begin(); i != this->importPluginHandlers.end(); ++i) {
        (*i)->invokeInitialize();
    }
}

void MMSPluginManager::initializeBackendPlugins() {
	for(vector<MMSBackendPluginHandler*>::iterator i = this->backendPluginHandlers.begin(); i != this->backendPluginHandlers.end(); ++i) {
        MMSPluginData pd = (*i)->getPluginData();
        (*i)->setSwitcherInterface(switcher->newSwitcher(&pd));
        (*i)->invokeInitialize();
    }
}

vector<MMSOSDPluginHandler *> MMSPluginManager::getOSDPluginHandlers(vector <MMSPluginData *> data) {
    vector<MMSOSDPluginHandler *> myhandlers;

    vector<MMSPluginData*>::iterator pdi;
    vector<MMSPluginData*>::iterator end = data.end();
    for(pdi = data.begin(); pdi != end; ++pdi) {
    	vector<MMSOSDPluginHandler*>::iterator oi;
    	vector<MMSOSDPluginHandler*>::iterator oi_end = this->osdPluginHandlers.end();
    	for(oi = this->osdPluginHandlers.begin(); oi != oi_end; ++oi) {
            if((*oi)->getPluginData().getId() == (*pdi)->getId()) {
                if((*oi)->getPluginData().getType()->getName() == PT_OSD_PLUGIN) {
                	myhandlers.push_back(*oi);
                } else {
                    throw MMSPluginManagerError(0,"handler for id " + iToStr((*pdi)->getId()) + " is not a osd plugin");
                }
            }
    	}
    }

    return myhandlers;
}

MMSOSDPluginHandler *MMSPluginManager::getOSDPluginHandler(int pluginid) {
	vector<MMSOSDPluginHandler*>::iterator i;
	vector<MMSOSDPluginHandler*>::iterator end = this->osdPluginHandlers.end();
	for(i = this->osdPluginHandlers.begin(); i != end; ++i) {
		if((*i)->getPluginData().getId() == pluginid) {
			if((*i)->getPluginData().getType()->getName() == PT_OSD_PLUGIN) {
				return (*i);
			} else {
                throw MMSPluginManagerError(0,"handler for id " + iToStr(pluginid) + " is not a osd plugin");
			}
		}
	}

    throw MMSPluginManagerError(0,"osd plugin handler for id " + iToStr(pluginid) + " was not found");
}

vector<MMSCentralPluginHandler *> MMSPluginManager::getCentralPluginHandlers(vector <MMSPluginData *> data) {
    vector<MMSCentralPluginHandler *> myhandlers;

    vector<MMSPluginData*>::iterator pdi;
    vector<MMSPluginData*>::iterator end = data.end();
    for(pdi = data.begin(); pdi != end; ++pdi) {
    	vector<MMSCentralPluginHandler*>::iterator ci;
    	vector<MMSCentralPluginHandler*>::iterator ci_end = this->centralPluginHandlers.end();
    	for(ci = this->centralPluginHandlers.begin(); ci != ci_end; ++ci) {
            if((*ci)->getPluginData().getId() == (*pdi)->getId()) {
                if((*ci)->getPluginData().getType()->getName() == PT_CENTRAL_PLUGIN) {
                	myhandlers.push_back(*ci);
                } else {
                    throw MMSPluginManagerError(0,"handler for id " + iToStr((*pdi)->getId()) + " is not a central plugin");
                }
            }
    	}
    }

    return myhandlers;
}

MMSCentralPluginHandler *MMSPluginManager::getCentralPluginHandler(int pluginid) {
	vector<MMSCentralPluginHandler*>::iterator i;
	vector<MMSCentralPluginHandler*>::iterator end = this->centralPluginHandlers.end();
	for(i = this->centralPluginHandlers.begin(); i != end; ++i) {
		if((*i)->getPluginData().getId() == pluginid) {
			if((*i)->getPluginData().getType()->getName() == PT_CENTRAL_PLUGIN) {
				return (*i);
			} else {
                throw MMSPluginManagerError(0,"handler for id " + iToStr(pluginid) + " is not a central plugin");
			}
		}
	}

    throw MMSPluginManagerError(0,"central plugin handler for id " + iToStr(pluginid) + " was not found");
}

vector<MMSImportPluginHandler *> MMSPluginManager::getImportPluginHandlers(vector <MMSPluginData *> data) {
    vector<MMSImportPluginHandler *> myhandlers;

    vector<MMSPluginData*>::iterator pdi;
    vector<MMSPluginData*>::iterator end = data.end();
    for(pdi = data.begin(); pdi != end; ++pdi) {
    	vector<MMSImportPluginHandler*>::iterator ii;
    	vector<MMSImportPluginHandler*>::iterator ii_end = this->importPluginHandlers.end();
    	for(ii = this->importPluginHandlers.begin(); ii != ii_end; ++ii) {
            if((*ii)->getPluginData().getId() == (*pdi)->getId()) {
                if((*ii)->getPluginData().getType()->getName() == PT_IMPORT_PLUGIN) {
                	myhandlers.push_back(*ii);
                } else {
                    throw MMSPluginManagerError(0,"handler for id " + iToStr((*pdi)->getId()) + " is not an import plugin");
                }
            }
    	}
    }

    return myhandlers;
}

MMSImportPluginHandler *MMSPluginManager::getImportPluginHandler(int pluginid) {
	vector<MMSImportPluginHandler*>::iterator i;
	vector<MMSImportPluginHandler*>::iterator end = this->importPluginHandlers.end();
	for(i = this->importPluginHandlers.begin(); i != end; ++i) {
		if((*i)->getPluginData().getId() == pluginid) {
			if((*i)->getPluginData().getType()->getName() == PT_IMPORT_PLUGIN) {
				return (*i);
			} else {
                throw MMSPluginManagerError(0,"handler for id " + iToStr(pluginid) + " is not an import plugin");
			}
		}
	}

    throw MMSPluginManagerError(0,"import plugin handler for id " + iToStr(pluginid) + " was not found");
}

vector<MMSBackendPluginHandler *> MMSPluginManager::getBackendPluginHandlers(vector <MMSPluginData *> data) {
    vector<MMSBackendPluginHandler *> myhandlers;

    vector<MMSPluginData*>::iterator pdi;
    vector<MMSPluginData*>::iterator end = data.end();
    for(pdi = data.begin(); pdi != end; ++pdi) {
    	vector<MMSBackendPluginHandler*>::iterator bi;
    	vector<MMSBackendPluginHandler*>::iterator bi_end = this->backendPluginHandlers.end();
    	for(bi = this->backendPluginHandlers.begin(); bi != bi_end; ++bi) {
            if((*bi)->getPluginData().getId() == (*pdi)->getId()) {
                if((*bi)->getPluginData().getType()->getName() == PT_BACKEND_PLUGIN) {
                	myhandlers.push_back(*bi);
                } else {
                    throw MMSPluginManagerError(0,"handler for id " + iToStr((*pdi)->getId()) + " is not a backend plugin");
                }
            }
    	}
    }

    return myhandlers;
}

MMSBackendPluginHandler *MMSPluginManager::getBackendPluginHandler(int pluginid) {
	vector<MMSBackendPluginHandler*>::iterator i;
	vector<MMSBackendPluginHandler*>::iterator end = this->backendPluginHandlers.end();
	for(i = this->backendPluginHandlers.begin(); i != end; ++i) {
		if((*i)->getPluginData().getId() == pluginid) {
			if((*i)->getPluginData().getType()->getName() == PT_BACKEND_PLUGIN) {
				return (*i);
			} else {
                throw MMSPluginManagerError(0,"handler for id " + iToStr(pluginid) + " is not a backend plugin");
			}
		}
	}

    throw MMSPluginManagerError(0,"backend plugin handler for id " + iToStr(pluginid) + " was not found");
}


void MMSPluginManager::setActiceOSDPlugin(MMSPluginData *plugin) {
	this->activeosdplugin = plugin;
}

MMSPluginData *MMSPluginManager::getActiveOSDPlugin() {
	return this->activeosdplugin;
}

void MMSPluginManager::setActiceCentralPlugin(MMSPluginData *plugin) {
	this->activecentralplugin = plugin;
}

MMSPluginData *MMSPluginManager::getActiveCentralPlugin() {
	return this->activecentralplugin;
}

void MMSPluginManager::setSwitcher(IMMSSwitcher *switcher) {
	this->switcher = switcher;
}
