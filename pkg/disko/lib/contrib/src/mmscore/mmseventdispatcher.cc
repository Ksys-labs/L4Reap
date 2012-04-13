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

#include "mmscore/mmseventdispatcher.h"
#include "mmscore/mmseventthread.h"


MMSEventDispatcher::MMSEventDispatcher(MMSPluginManager *manager,MMSEventSignupManager *signupmanager) {
    this->manager = manager;
    this->signupmanager = signupmanager;
}

MMSEventDispatcher::~MMSEventDispatcher() {
}

void MMSEventDispatcher::raise(_IMMSEvent *event, int id) {
    MMSEventThread *thread;
    vector <MMSOSDPluginHandler *> osdHandlers;
    vector <MMSCentralPluginHandler *> centralHandlers;
    vector <MMSBackendPluginHandler *> backendHandlers;
    vector <MMSPluginData *> plugins;
    vector <sigc::signal<void, _IMMSEvent*> *> mysignals;
    IMMSEvent e(event);

    if (id > 0) {
    	DEBUGMSG("MMSEventdispatcher", "have a direct receiver");
    	try {
    		thread = new MMSEventThread(this->getManager()->getOSDPluginHandler(id), e);
	    } catch(MMSError &error) {
    		thread = new MMSEventThread(this->getManager()->getCentralPluginHandler(id), e);
	    }
        thread->start();
    } else {
    	DEBUGMSG("MMSEventdispatcher", "get receiver plugins");
        /* get all receiver plugins */
        try {
        	plugins = this->getSignupManager()->getReceiverPlugins(event);

        	DEBUGMSG("MMSEventdispatcher", "filter the osd handler");
	        /* get all osd handlers */
	        osdHandlers = getManager()->getOSDPluginHandlers(plugins);
	        for(unsigned int i=0; i<osdHandlers.size();i++) {
	        	DEBUGMSG("MMSEventdispatcher", "%s --> create new event thread for %s.", (osdHandlers.at(i))->getPluginData().getName().c_str(), event->getHeading().c_str());
	            /* start the threads */
	            thread = new MMSEventThread(osdHandlers.at(i), e);
	            thread->start();
	        }

	        DEBUGMSG("MMSEventdispatcher", "filter the central handler");
	        /* get all central handlers */
	        centralHandlers = getManager()->getCentralPluginHandlers(plugins);
	        for(unsigned int i=0; i<centralHandlers.size();i++) {
	        	DEBUGMSG("MMSEventdispatcher", "%s --> create new event thread for %s", (centralHandlers.at(i))->getPluginData().getName().c_str(), event->getHeading().c_str());
	            /* start the threads */
	            thread = new MMSEventThread(centralHandlers.at(i), e);
	            thread->start();
	        }

	        DEBUGMSG("MMSEventdispatcher", "filter the backend handler");
	        /* get all central handlers */
	        backendHandlers = getManager()->getBackendPluginHandlers(plugins);
	        for(unsigned int i=0; i<backendHandlers.size();i++) {
	            /* start the threads */
	        	DEBUGMSG("MMSEventdispatcher", "%s --> create new event thread for %s.", (backendHandlers.at(i))->getPluginData().getName().c_str(), event->getHeading().c_str());
	            thread = new MMSEventThread(backendHandlers.at(i), e);
	            thread->start();
	        }

        } catch (MMSEventSignupManagerError &err) {
        	DEBUGMSG("MMSEventdispatcher", "Error: %s", err.getMessage().c_str());
        	DEBUGMSG("MMSEventdispatcher", "try signal receivers");
        }

        // go for receiver signals
        try {
			mysignals = this->getSignupManager()->getReceiverSignals(event);
			for(vector <sigc::signal<void, _IMMSEvent*> *>::iterator it = mysignals.begin(); it != mysignals.end();it++) {
				(*it)->emit(event);
			}
        } catch (MMSEventSignupManagerError &err) {
        	DEBUGMSG("MMSEventdispatcher", "Error: %s", err.getMessage().c_str());
        	return;
        }

        //get rid of the allocated plugindata
        for(vector<MMSPluginData *>::iterator it = plugins.begin(); it != plugins.end(); ++it)
        	delete *it;
    }
}

MMSPluginManager *MMSEventDispatcher::getManager() {
    return this->manager;
}
MMSEventSignupManager *MMSEventDispatcher::getSignupManager() {
    return this->signupmanager;
}
