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

#ifdef __HAVE_MMSSIP__

#ifndef MMSSIP_H_
#define MMSSIP_H_

#include <pjsua-lib/pjsua.h>
#include <iostream>
#include <sigc++/sigc++.h>
#include <map>

using namespace std;

typedef struct {
	string user;
	string passwd;
	string registrar;
	string realm;
	bool   autoanswer;
} MMSSipAccount;

typedef enum {
	BUDDY_ONLINE		= PJSUA_BUDDY_STATUS_ONLINE,
	BUDDY_OFFLINE		= PJSUA_BUDDY_STATUS_OFFLINE,
	BUDDY_UNKNOWN		= PJSUA_BUDDY_STATUS_UNKNOWN
} MMSSipBuddyStatus;

typedef struct {
	string 				name;
	string 				uri;
	MMSSipBuddyStatus	status;
} MMSSipBuddy;

typedef struct {
	pj_thread_desc	desc;
	pj_thread_t		*thread;
} MMSSipThread;

class MMSSip {
    private:
        string       stunserver,
                     nameserver;
        short int    localPort;

        vector<MMSSipThread>	threadInfo;

        map<int, MMSSipAccount>	accounts;
        int						defaultAccount;
        map<int, MMSSipBuddy> 	buddies;

    public:
        MMSSip(const string    &user,
    		   const string    &passwd,
    		   const string    &registrar,
    		   const string    &realm = "",
    		   const string    &stunserver = "",
    		   const string    &nameserver = "",
    		   const short int &localPort = 5060);

    	~MMSSip();

    	const bool registerAccount(const string &user,
								   const string &passwd,
								   const string &registrar,
								   const string &realm,
								   const bool defaultAccount = false,
								   const bool autoanswer = false);
    	const int call(const string &user, const string &domain = "");
    	void hangup(int id = PJSUA_INVALID_ID);
    	void answer(int id);
    	void addBuddy(const string &name, const string &uri);
        MMSSipBuddy	getBuddy(const int &id);
        bool setSpeakerVolume(const unsigned int percent);
        int  getSpeakerVolume();
        bool getAutoAnswer(const int accountId);
        void setAutoAnswer(const int accountId, const bool value = true);
        bool registerRingtone(const string &filename);
        bool registerBusytone(const string &filename);
        bool registerCallingtone(const string &filename);

        /**
         * sigc++-connector that will be emitted if a call is
         * successfull connected.
         *
         * Parameters are callid and call state.
         */
        sigc::signal<void, int, int>               *onCallSuccessfull;

        /**
         * sigc++-connector that will be emitted if a call is
         * incoming.
         *
         * Parameters are callid, calling user and call state.
         */
        sigc::signal<void, int, string, int>       *onCallIncoming;

        /**
         * sigc++-connector that will be emitted if a call is
         * disconnected.
         *
         * Parameters are callid and call state.
         */
        sigc::signal<void, int, int>               *onCallDisconnected;

        /**
         * sigc++-connector that will be emitted if you are trying
         * to call.
         *
         * Parameters are callid and call state.
         */
        sigc::signal<void, int, int>               *onCalling;

        /**
         * sigc++-connector that will be emitted if the status of
         * one of your buddies changed.
         *
         * Parameter is MMSSipBuddy structure.
         */
        sigc::signal<void, MMSSipBuddy>            *onBuddyStatus;
};

#endif /* MMSSIP_H_ */
#endif /* __HAVE_MMSSIP__ */
