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

#include "mmscore/mmseventsignupmanager.h"
#include "mmstools/tools.h"
#include <string.h>

MMSEventSignupManager::MMSEventSignupManager() {
}

void MMSEventSignupManager::signup(IMMSEventSignup *signup) {
    this->signupmutex.lock();
    if(signup->isPlugin())
    	this->signups.push_back((MMSEventSignup *)signup);
    else
    	this->signals.push_back((MMSEventSignup *)signup);

    this->signupmutex.unlock();
}

vector<MMSPluginData *> MMSEventSignupManager::getReceiverPlugins(_IMMSEvent *event) {
    vector<MMSPluginData *> mydata;

    vector<string *> subscriptions;
    vector<string *>::iterator subsit;
    vector<MMSEventSignup *>::iterator signit;

    for(signit = this->signups.begin();signit != this->signups.end();signit++) {
    	subscriptions = (*signit)->getSubScriptions();
        for(subsit=subscriptions.begin();subsit!= subscriptions.end();subsit++){
      		 DEBUGMSG("MMSEventSignupManager", "compare %s to %s - size: %d", (*subsit)->c_str(), event->getHeading().c_str(), (*subsit)->size());
            /*compare heading of event with subsciptions */
            if(strncmp((*subsit)->c_str(),
                       event->getHeading().c_str(),
                       (*subsit)->size())==0) {

                /* add plugindata to return vector*/
                MMSPluginData *myplugin = new MMSPluginData;
                *myplugin = (*signit)->getPluginData();

                mydata.push_back(myplugin);
            }
        }
        subscriptions.clear();
    }
    if(mydata.empty())
        throw MMSEventSignupManagerError(0,"no subscriptions found");

    return mydata;
}

vector<sigc::signal<void, _IMMSEvent*> *> MMSEventSignupManager::getReceiverSignals(_IMMSEvent *event) {
	vector<sigc::signal<void, _IMMSEvent*> *> mysignals;
    vector<string *> subscriptions;
    vector<string *>::iterator subsit;
    vector<MMSEventSignup *>::iterator signupsit;

    for(signupsit= this->signals.begin();signupsit != this->signals.end();signupsit++) {
    	subscriptions = (*signupsit)->getSubScriptions();
        for(subsit = subscriptions.begin();subsit != subscriptions.end();subsit++) {
      		 DEBUGMSG("MMSEventSignupManager", "compare %s to %s - size: %d", (*subsit)->c_str(), event->getHeading().c_str(), (*subsit)->size());
            /*compare heading of event with subsciptions */
            if(strncmp((*subsit)->c_str(),
                       event->getHeading().c_str(),
                       (*subsit)->size())==0) {

                mysignals.push_back((*signupsit)->getSignal());
            }
        }
        subscriptions.clear();
    }
    if(mysignals.empty())
        throw MMSEventSignupManagerError(0,"no subscriptions found");

    return mysignals;

}
