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

#ifndef MMSEVENTSIGNUP_H_
#define MMSEVENTSIGNUP_H_

#include "mmsbase/interfaces/immseventsignupmanager.h"
#include "mmsconfig/mmsplugindata.h"
#include "mmstools/mmserror.h"

MMS_CREATEERROR(MMSEventSignupError);

class MMSEventSignup : public IMMSEventSignup  {
    private:
        static IMMSEventSignupManager 	*manager;
        IMMSEventSignupManager 			*getManager();
        MMSPluginData          			data;
        vector<string *>       			subscriptions;
        bool 							plugindataset;
        sigc::signal<void,_IMMSEvent*>  *onSubscription;

    public:
        MMSEventSignup(MMSPluginData data);
        MMSEventSignup();
        virtual ~MMSEventSignup();
        void add(string);
        void executeSignup();
        MMSPluginData getPluginData();
        vector<string *> getSubScriptions();
        void setManager(IMMSEventSignupManager *manager);
        sigc::signal<void,_IMMSEvent*> *getSignal();
        bool isPlugin();

};

#endif /*MMSEVENTSIGNUP_H_*/
