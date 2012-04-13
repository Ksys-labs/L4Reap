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

#include "mmscore/mmsimportscheduler.h"

#define SCHEDULER_SLEEP_TIME 10 /* sleep time in seconds */

MMSImportScheduler::MMSImportScheduler(MMSPluginManager *pluginManager) {

    MMSConfigData *config = new MMSConfigData();
    DataSource    *source = new DataSource(config->getConfigDBDBMS(),
    		                               config->getConfigDBDatabase(),
    		                               config->getConfigDBAddress(),
    		                               config->getConfigDBPort(),
    		                               config->getConfigDBUser(),
    		                               config->getConfigDBPassword());
    delete config;

    /* run with the minimum priority */
//	this->setSchedulePriority(PRIORITY_MIN);

    /* save access to plugin manager */
    this->pluginManager = pluginManager;

    /* initialize the service objects */
    this->pluginService = new MMSPluginService(source);
    this->importPropertyService = new MMSImportPropertyService(source);
}

MMSImportScheduler::~MMSImportScheduler() {
    for(vector<IMPORT_PLUGINS*>::iterator it(importPlugins.begin()); it != importPlugins.end(); ++it) {
        delete(((IMPORT_PLUGINS*)(*it))->plugin);
        delete(((IMPORT_PLUGINS*)(*it))->importProperty);
        delete(((IMPORT_PLUGINS*)(*it))->pluginHandler);
        delete(*it);
    }
    delete this->pluginService;
    delete this->importPropertyService;
}

void MMSImportScheduler::getImportPlugins() {
    vector<MMSPluginData *> pluginList;

    DEBUGMSG("IMPORTSCHEDULER", "getImportPlugins()");
    /* get all import plugins */
    pluginList = this->pluginService->getImportPlugins();

    int base_time = time(NULL);

    /* go through the old importPlugins list -> delete entries */
    for(unsigned i=0; i<importPlugins.size(); i++) {
        /* check if entry is in pluginList */
        bool found=false;
        unsigned j=0;

        DEBUGMSG("IMPORTSCHEDULER", "Work with %s", importPlugins.at(i)->plugin->getName().c_str());
        while(j<pluginList.size()) {
            if (importPlugins.at(i)->plugin->getId() == pluginList.at(j)->getId()) {
                found=true;
                break;
            }
            j++;
        }

        if (!found) {

        	/* delete old entry from importPlugins */
        	DEBUGMSG("IMPORTSCHEDULER", "delete %s", importPlugins.at(i)->plugin->getName().c_str());
            importPlugins.erase(importPlugins.begin()+i);
        }
    }

    /* go through the new plugin list -> get import properties for each import plugin */
    for(unsigned i=0; i<pluginList.size(); i++) {
        /* check if entry is already in importPlugins */
        bool found=false;
        unsigned j=0;
        while(j<importPlugins.size()) {
            if (pluginList.at(i)->getId() == importPlugins.at(j)->plugin->getId()) {
                found=true;
                break;
            }
            j++;
        }

        if (!pluginList.at(i)->getActive()) {
            /* delete incactive plugin */
            continue;
        }

        if (!found) {
            /* have to create new importPlugins entry */
            IMPORT_PLUGINS *ip = new IMPORT_PLUGINS;

            /* fill plugin */
            ip->plugin = pluginList.at(i);

            /* fill import property */
            ip->importProperty = this->importPropertyService->getImportPropertyByPlugin(ip->plugin);

            /* get the handler */
            ip->pluginHandler = this->pluginManager->getImportPluginHandler(ip->plugin->getId());

            /* calculate the nextTime */
            ip->nextTime = base_time;
            if (!ip->importProperty->getOnStartUp()) {
                if (ip->importProperty->getTime())
                    ip->nextTime += ip->importProperty->getTime();
                else
                    ip->nextTime = 0;
            }
            base_time += SCHEDULER_SLEEP_TIME;

            /* add item */
            importPlugins.push_back(ip);
        }
        else {
            /* take the old entry and update it */
            /* update plugin */
            importPlugins.at(j)->plugin = pluginList.at(i);

            /* update import property */
            importPlugins.at(j)->importProperty = this->importPropertyService->getImportPropertyByPlugin(importPlugins.at(j)->plugin);

            /* get the handler */
            importPlugins.at(j)->pluginHandler = this->pluginManager->getImportPluginHandler(importPlugins.at(j)->plugin->getId());
        }
    }
}

void MMSImportScheduler::threadMain() {
    time_t curr_time;

    /* wait a little so other components can start faster */
    sleep(2);

    try {
	    while(1) {
    	        /* get all import plugins */
    	        getImportPlugins();

    	        /* get the current time */
    	        curr_time = time(NULL);

    	        /* through all my import plugins */
    	        for(unsigned int i=0; i<importPlugins.size(); i++) {
    	            /* check something */
    	            if (!importPlugins.at(i)->nextTime) continue;
    	            if (importPlugins.at(i)->nextTime > curr_time) continue;
    	            if (importPlugins.at(i)->importProperty->getInterval() <= 0)
    	                importPlugins.at(i)->nextTime = 0;
    	            else
    	                importPlugins.at(i)->nextTime = curr_time + importPlugins.at(i)->importProperty->getInterval();

    	            /* invoke execute import */
                    /* TODO: open a thread */
                    try {
    	                importPlugins.at(i)->pluginHandler->invokeExecute(NULL);
                    } catch(MMSError &error) {
                    	DEBUGMSG("IMPORTSCHEDULER", "Abort import due to: %s", error.getMessage().c_str());
                    }

    	        }

    	        sleep(SCHEDULER_SLEEP_TIME);
	    }
    } catch(MMSError &error) {
    	DEBUGMSG("IMPORTSCHEDULER", "Abort import due to: %s", error.getMessage().c_str());
    }

    /* self destruct */
    delete this;
}

void MMSImportScheduler::executeImport(int pluginID) {
	for(unsigned int i = 0;i<importPlugins.size();i++) {
		if(importPlugins.at(i)->plugin->getId()==pluginID) {
			importPlugins.at(i)->pluginHandler->invokeExecute();
			return;
		}
	}
}

