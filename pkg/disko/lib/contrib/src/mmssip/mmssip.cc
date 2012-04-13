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

#ifdef PJSIP_AUTH_AUTO_SEND_NEXT
#undef PJSIP_AUTH_AUTO_SEND_NEXT
#endif
#define PJSIP_AUTH_AUTO_SEND_NEXT 1

#ifdef PJSIP_AUTH_HEADER_CACHING
#undef PJSIP_AUTH_HEADER_CACHING
#endif
#define PJSIP_AUTH_HEADER_CACHING 1


#include "mmstools/tools.h"
#include "mmstools/mmserror.h"
#include "mmssip/mmssip.h"

static MMSSip 			*thiz = NULL;
static bool   			registered = false;
static pjsua_player_id	ringtonePlayer = PJSUA_INVALID_ID;
static pjsua_player_id	busytonePlayer = PJSUA_INVALID_ID;
static pjsua_player_id	callingtonePlayer = PJSUA_INVALID_ID;

static void onIncomingCall(pjsua_acc_id, pjsua_call_id, pjsip_rx_data*);
static void onCallState(pjsua_call_id, pjsip_event*);
static void onCallMediaState(pjsua_call_id);
static void onRegistrationState(pjsua_acc_id);
static void onBuddyState(pjsua_buddy_id);

static void logCallback(int level, const char *data, int len) {
	DEBUGMSG("MMSSIP", data);
}

MMSSip::MMSSip(const string    &user,
		       const string    &passwd,
		       const string    &registrar,
		       const string    &realm,
		       const string    &stunserver,
		       const string    &nameserver,
		       const short int &localPort) :
    stunserver(stunserver),
    nameserver(nameserver),
    localPort(localPort),
    defaultAccount(-1) {

	/* only one instance of mmssip allowed */
	if(thiz) {
		DEBUGMSG("MMSSIP", "There's already an instance of MMSSIP running.");
		throw MMSError(0, "There's already an instance of MMSSIP running.");
	}

	thiz = this;

	pj_status_t status;

    /* create pjsua */
    status = pjsua_create();
    if(status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error initializing SIP stack (pjsua_create)");
		throw MMSError(0, "Error initializing SIP stack (pjsua_create)");
	}
    DEBUGMSG("MMSSIP", "SIP stack init #1");

    /* configure pjsua */
    pjsua_config         cfg;
    pjsua_logging_config logCfg;

    pjsua_config_default(&cfg);
    cfg.user_agent             = pj_str((char*)"Disko SIP stack");
    if(stunserver != "") {
    	DEBUGMSG("MMSSIP", "Using STUN server " + stunserver);
    	cfg.stun_host          = pj_str((char*)stunserver.c_str());
    }
    if(nameserver != "") {
    	DEBUGMSG("MMSSIP", "Using nameserver " + nameserver);
    	cfg.nameserver[0]      = pj_str((char*)nameserver.c_str());
    }
    cfg.cb.on_incoming_call    = &onIncomingCall;
    cfg.cb.on_call_media_state = &onCallMediaState;
    cfg.cb.on_call_state       = &onCallState;
    cfg.cb.on_reg_state        = &onRegistrationState;
    cfg.cb.on_buddy_state      = &onBuddyState;

    pjsua_logging_config_default(&logCfg);
    logCfg.level = 1;
#ifdef __ENABLE_LOG__
    logCfg.console_level = 4;
    logCfg.cb = logCallback;
#else
    logCfg.console_level = 0;
#endif

    status = pjsua_init(&cfg, &logCfg, NULL);
    if(status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error initializing SIP stack (pjsua_init)");
		throw MMSError(0, "Error initializing SIP stack (pjsua_init)");
	}

    DEBUGMSG("MMSSIP", "SIP stack init #2");

    /* add UDP transport */
    pjsua_transport_config transCfg;

    pjsua_transport_config_default(&transCfg);
    transCfg.port = localPort;
    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transCfg, NULL);
    if(status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error initializing SIP stack (pjsua_transport_create)");
		throw MMSError(0, "Error initializing SIP stack (pjsua_transport_create)");
	}

    DEBUGMSG("MMSSIP", "UDP transport created");

    /* start pjsua */
    status = pjsua_start();
    if(status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error starting SIP stack (pjsua_start)");
		throw MMSError(0, "Error starting SIP stack (pjsua_start)");
	}

    DEBUGMSG("MMSSIP", "SIP stack started");

    if(user != "") {
		if(!this->registerAccount(user, passwd, registrar, realm, true)) {
			DEBUGMSG("MMSSIP", "Error registering account");
			throw MMSError(0, "Error registering account");
		}
    }

    this->onCallSuccessfull    = new sigc::signal<void, int, int>;
    this->onCallIncoming       = new sigc::signal<void, int, string, int>;
    this->onCallDisconnected   = new sigc::signal<void, int, int>;
    this->onCalling            = new sigc::signal<void, int, int>;
    this->onBuddyStatus        = new sigc::signal<void, MMSSipBuddy>;
}

MMSSip::~MMSSip() {
	if(!pj_thread_is_registered()) {
		MMSSipThread tInfo;
		pj_bzero(tInfo.desc, sizeof(pj_thread_desc));
		if(pj_thread_register("MMSSIP", tInfo.desc, &tInfo.thread) == PJ_SUCCESS)
    		pjsua_destroy();
    }

	if(this->onCallSuccessfull) {
		this->onCallSuccessfull->clear();
		delete this->onCallSuccessfull;
	}
	if(this->onCallIncoming) {
		this->onCallIncoming->clear();
		delete this->onCallIncoming;
	}
	if(this->onCallDisconnected) {
		this->onCallDisconnected->clear();
		delete this->onCallDisconnected;
	}
	if(this->onCalling) {
		this->onCalling->clear();
		delete this->onCalling;
	}
	if(this->onBuddyStatus) {
		this->onBuddyStatus->clear();
		delete this->onBuddyStatus;
	}
	this->accounts.clear();
	this->buddies.clear();
}

/**
 * Register an account.
 *
 * @param	user		[in]	user name
 * @param	passwd		[in]	password
 * @param	registrar	[in]	registrar
 * @param	realm		[in]	realm
 * @param	defaultAcc	[in]	use this as default account for incoming/outgoing calls
 * @param	autoanswer	[in]	automatically answer incoming calls
 *
 * @note If you're using the autoanswer feature, all other active calls
 * will be disconnected, when an incoming call arrives.
 *
 * @return	true, if account was registered successfully
 */
const bool MMSSip::registerAccount(const string &user,
								   const string &passwd,
								   const string &registrar,
								   const string &realm,
								   const bool defaultAcc,
								   const bool autoanswer) {
	pj_status_t 		status;
    pjsua_acc_config 	accCfg;
    pjsua_acc_id		accID;
    char 				tmpid[256], tmpreg[256];

    DEBUGMSG("MMSSIP", "Registering account " + user + "@" + registrar +
    		"(default: " + (defaultAcc ? "true" : "false") +
    		 ", autoanswer: " + (autoanswer ? "true" : "false") + ")");

    snprintf(tmpid, sizeof(tmpid), "sip:%s@%s", user.c_str(), registrar.c_str());
    snprintf(tmpreg, sizeof(tmpreg), "sip:%s", realm.c_str());

    pjsua_acc_config_default(&accCfg);
    accCfg.reg_timeout  = 60;
    accCfg.id         = pj_str(tmpid);
    accCfg.reg_uri    = pj_str(tmpreg);
    if(defaultAcc) accCfg.priority += 1;
    accCfg.cred_count = 1;
    accCfg.cred_info[0].realm     = pj_str((char*)"*");
    accCfg.cred_info[0].scheme    = pj_str((char*)"Digest");
    accCfg.cred_info[0].username  = pj_str((char*)user.c_str());
    accCfg.cred_info[0].data_type = 0;
    accCfg.cred_info[0].data      = pj_str((char*)passwd.c_str());
    accCfg.publish_enabled        = PJ_FALSE;

    status = pjsua_acc_add(&accCfg, (defaultAcc ? PJ_TRUE : PJ_FALSE), &accID);
    if(status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error registering account sip:" + user + "@" + registrar + " (pjsua_acc_add)");
		return false;
	}

    DEBUGMSG("MMSSIP", "Account " + user + "@" + registrar + " has ID " + iToStr(accID));

 	MMSSipAccount acc = {user, passwd, registrar, realm, autoanswer};
    this->accounts[accID] = acc;
    if(defaultAcc)
    	this->defaultAccount = accID;

    return true;
}

/*
 * Calls a given user.
 *
 * @param	user	[in]	user id
 * @param	domain	[in]	domain for users sip account
 *
 * @note The SIP uri will be "sip:id@domain".
 *
 * @return call id
 *
 * @see MMSSip::hangup()
 */
const int MMSSip::call(const string &user, const string &domain) {
	pj_status_t    status;
	pj_str_t       uri;
    pjsua_call_id  call;
    char           tmp[1024];

    if(!registered) {
    	DEBUGMSG("MMSSIP", "Cannot make a call (not registered)");
    	throw MMSError(0, "Cannot make a call (not registered)");
    }

    const char *cDomain;
    if((user.find("@") == string::npos) && this->defaultAccount >= 0) {
        cDomain = ((domain != "") ? domain.c_str() : this->accounts[defaultAccount].registrar.c_str());
        snprintf(tmp, 1024, "sip:%s@%s", user.c_str(), cDomain);
    }
    else
        snprintf(tmp, 1024, "sip:%s", user.c_str());

	if(!pj_thread_is_registered()) {
		MMSSipThread tInfo;
		pj_bzero(tInfo.desc, sizeof(pj_thread_desc));
		if(pj_thread_register("MMSSIP", tInfo.desc, &tInfo.thread) == PJ_SUCCESS) {
			this->threadInfo.push_back(tInfo);
		} else {
			DEBUGMSG("MMSSIP", "Error registering thread (pj_thread_register)");
			throw MMSError(0, "Error registering thread (pj_thread_register)");
		}
    }

	status = pjsua_verify_sip_url(tmp);
	if (status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Invalid callee info sip:" + user + "@" + cDomain);
		throw MMSError(0, "Invalid callee info sip:" + user + "@" + cDomain);
	}

    uri = pj_str(tmp);
	status = pjsua_call_make_call(this->defaultAccount, &uri, 0, NULL, NULL, &call);
	if (status != PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "Error calling sip:" + user + "@" + cDomain);
		char buf[1024];
		pj_strerror(status, buf, sizeof(buf));
		throw MMSError(0, buf);
	}

	return call;
}

void MMSSip::hangup(int id) {
    DEBUGMSG("MMSSIP", "calling pjsua_call_hangup (id=%d)", id);

	if(!pj_thread_is_registered()) {
		MMSSipThread tInfo;
		pj_bzero(tInfo.desc, sizeof(pj_thread_desc));
		if(pj_thread_register("MMSSIP", tInfo.desc, &tInfo.thread) == PJ_SUCCESS) {
			this->threadInfo.push_back(tInfo);
		} else {
			DEBUGMSG("MMSSIP", "Error registering thread (pj_thread_register)");
			throw MMSError(0, "Error registering thread (pj_thread_register)");
		}
    }

	if(id != PJSUA_INVALID_ID) {
		// this is just a workaround, because sending 603/Decline didn't
		// work if state < PJSIP_INV_STATE_CONNECTING
		pjsua_call_info ci;
		pjsua_call_get_info(id, &ci);
		if(ci.state < PJSIP_INV_STATE_CONNECTING) {
			DEBUGMSG("MMSSIP", "answering with code 480");
			pjsua_call_answer(id, 480, NULL, NULL);
		}
		else
			pjsua_call_hangup(id, 0, NULL, NULL);
	}
	else
		pjsua_call_hangup_all();
}

void MMSSip::answer(int id) {
    DEBUGMSG("MMSSIP", "calling pjsua_call_answer");

	if(!pj_thread_is_registered()) {
		MMSSipThread tInfo;
		pj_bzero(tInfo.desc, sizeof(pj_thread_desc));
		if(pj_thread_register("MMSSIP", tInfo.desc, &tInfo.thread) == PJ_SUCCESS) {
			this->threadInfo.push_back(tInfo);
		} else {
			DEBUGMSG("MMSSIP", "Error registering thread (pj_thread_register)");
			throw MMSError(0, "Error registering thread (pj_thread_register)");
		}
    }

    pjsua_call_answer(id, 200, NULL, NULL);
}

void MMSSip::addBuddy(const string &name, const string &uri) {
    pjsua_buddy_config buddyCfg;
    pjsua_buddy_id     buddyId;
    pjsua_buddy_info   buddyInfo;

    if(!registered) {
    	DEBUGMSG("MMSSIP", "Cannot add buddy (not registered)");
    	throw MMSError(0, "Cannot add buddy (not registered)");
    }

	if(!pj_thread_is_registered()) {
		MMSSipThread tInfo;
		pj_bzero(tInfo.desc, sizeof(pj_thread_desc));
		if(pj_thread_register("MMSSIP", tInfo.desc, &tInfo.thread) == PJ_SUCCESS) {
			this->threadInfo.push_back(tInfo);
		} else {
			DEBUGMSG("MMSSIP", "Error registering thread (pj_thread_register)");
			throw MMSError(0, "Error registering thread (pj_thread_register)");
		}
    }

    pjsua_buddy_config_default(&buddyCfg);
    char buri[80];
    sprintf(buri, "sip:%s", uri.c_str());
    buddyCfg.uri = pj_str(buri);
    buddyCfg.subscribe = true;
    if(pjsua_buddy_add(&buddyCfg, &buddyId) == PJ_SUCCESS) {
    	DEBUGMSG("MMSSIP", "successfully added buddy " + name);
    	MMSSipBuddy buddy = {name, uri, BUDDY_UNKNOWN};
    	buddies[buddyId] = buddy;
    	if(pjsua_buddy_get_info(buddyId, &buddyInfo) == PJ_SUCCESS) {
    		buddy.status = (MMSSipBuddyStatus)buddyInfo.status;
    	}
       	buddies[buddyId] = buddy;
       	onBuddyState(buddyId);
    }
    else
	    DEBUGMSG("MMSSIP", "failed to add buddy " + name);
}

MMSSipBuddy MMSSip::getBuddy(const int &id) {
	return this->buddies[id];
}

bool MMSSip::setSpeakerVolume(const unsigned int percent) {
	if(percent > 100) return false;
	if(!pjsua_conf_adjust_tx_level(0, (float)percent / 100) == PJ_SUCCESS) {
		DEBUGMSG("MMSSIP", "setting speaker volume failed");
		return false;
	}
	DEBUGMSG("MMSSIP", "setting speaker volume to %d%%", percent);

	return true;
}

int MMSSip::getSpeakerVolume() {
	unsigned int tx_level, rx_level;
	if(pjsua_conf_get_signal_level(0, &tx_level, &rx_level) == PJ_SUCCESS)
		return tx_level;

	return -1;
}

bool MMSSip::getAutoAnswer(int accountId) {
	try {
		MMSSipAccount acc = this->accounts[accountId];
		return acc.autoanswer;
	}
	catch(std::exception& e) {
		throw MMSError(0, e.what());
	}
}

void MMSSip::setAutoAnswer(int accountId, const bool value) {
	try {
		MMSSipAccount acc = this->accounts[accountId];
		acc.autoanswer = value;
	}
	catch(std::exception& e) {
		throw MMSError(0, e.what());
	}
}

/**
 * Register a .wav file as ringtone.
 *
 * @param	filename	[in]	wav-file to play
 *
 * @return	true, if successfull
 */
bool MMSSip::registerRingtone(const string &filename) {
	pj_str_t tmp;
	if(pjsua_player_create(pj_cstr(&tmp, filename.c_str()), 0, &ringtonePlayer) == PJ_SUCCESS)
		return true;

	return false;
}

/**
 * Register a .wav file as busy-tone.
 *
 * @note 	This file won't be looped.
 *
 * @param	filename	[in]	wav-file to play
 *
 * @return	true, if successfull
 */
bool MMSSip::registerBusytone(const string &filename) {
	pj_str_t tmp;
	if(pjsua_player_create(pj_cstr(&tmp, filename.c_str()), PJMEDIA_FILE_NO_LOOP, &busytonePlayer) == PJ_SUCCESS)
		return true;

	return false;
}

/**
 * Register a .wav file as calling-tone.
 *
 * @param	filename	[in]	wav-file to play
 *
 * @return	true, if successfull
 */
bool MMSSip::registerCallingtone(const string &filename) {
	pj_str_t tmp;
	if(pjsua_player_create(pj_cstr(&tmp, filename.c_str()), 0, &callingtonePlayer) == PJ_SUCCESS)
		return true;

	return false;
}

/* Callback called by the library upon receiving incoming call */
static void onIncomingCall(pjsua_acc_id  accId,
		                   pjsua_call_id callId,
                           pjsip_rx_data *rdata) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(rdata);

    /* if autoanswer is set, hangup all other calls */
    if(thiz->getAutoAnswer(accId)) {
    	DEBUGMSG("MMSSIP", "Incoming call on account %d which has autoanswer feature turned on", accId);
    	pjsua_call_id *ids;
    	unsigned int count;
    	if(pjsua_enum_calls(ids, &count) == PJ_SUCCESS) {
    		for(unsigned int i = 0; i < count; ++i)
    			pjsua_call_hangup(ids[i], 481, NULL, NULL);
    	}
    	thiz->answer(callId);
    } else {
		pjsua_call_get_info(callId, &ci);

		DEBUGMSG("MMSSIP", "Incoming call from %.*s (id=%d)", (int)ci.remote_info.slen, ci.remote_info.ptr, callId);

		if(thiz && thiz->onCallIncoming)
			thiz->onCallIncoming->emit(callId, ci.remote_info.ptr, ci.last_status);
    }
}

/* Callback called by the library when call's state has changed */
static void onCallState(pjsua_call_id callId, pjsip_event *e) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(callId, &ci);
    DEBUGMSG("MMSSIP", "Call %d state=%d (%.*s)", callId, ci.state, (int)ci.state_text.slen, ci.state_text.ptr);

    switch(ci.state) {
        case PJSIP_INV_STATE_NULL:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_NULL");
        	break;
        case PJSIP_INV_STATE_CALLING:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_CALLING");
            if(thiz && thiz->onCalling)
                thiz->onCalling->emit(callId, ci.last_status);
        	break;
        case PJSIP_INV_STATE_INCOMING:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_INCOMING");
        	if(ringtonePlayer != PJSUA_INVALID_ID &&
        		ci.role == PJSIP_ROLE_UAS &&
        	    ci.media_status == PJSUA_CALL_MEDIA_NONE)
        		pjsua_conf_connect(pjsua_player_get_conf_port(ringtonePlayer), 0);
        	break;
        case PJSIP_INV_STATE_EARLY:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_EARLY");
        	break;
        case PJSIP_INV_STATE_CONNECTING:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_CONNECTING");
        	if(callingtonePlayer != PJSUA_INVALID_ID &&
        		ci.role == PJSIP_ROLE_UAS &&
        	    ci.media_status == PJSUA_CALL_MEDIA_NONE)
        		pjsua_conf_connect(pjsua_player_get_conf_port(callingtonePlayer), 0);
        	break;
        case PJSIP_INV_STATE_CONFIRMED:
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_CONFIRMED");
        	if(callingtonePlayer != PJSUA_INVALID_ID)
        		pjsua_conf_disconnect(pjsua_player_get_conf_port(callingtonePlayer), 0);
            if(thiz && thiz->onCallSuccessfull)
                thiz->onCallSuccessfull->emit(callId, ci.last_status);
        	break;
        case PJSIP_INV_STATE_DISCONNECTED:
        	DEBUGMSG("MMSSIP", "lastStatusText: %s", ci.last_status_text);
        	DEBUGMSG("MMSSIP", "onCallState: PJSIP_INV_STATE_DISCONNECTED");
        	if(ringtonePlayer != PJSUA_INVALID_ID)
        		pjsua_conf_disconnect(pjsua_player_get_conf_port(ringtonePlayer), 0);
        	if((ci.last_status == 486) && (busytonePlayer != PJSUA_INVALID_ID))
        		pjsua_conf_connect(pjsua_player_get_conf_port(busytonePlayer), 0);
        	if(thiz && thiz->onCallDisconnected)
                thiz->onCallDisconnected->emit(callId, ci.last_status);
        	break;
        default:

        	break;
    }
}

/* Callback called by the library when call's media state has changed */
static void onCallMediaState(pjsua_call_id callId) {
    pjsua_call_info ci;

	if(ringtonePlayer != PJSUA_INVALID_ID)
		pjsua_conf_disconnect(pjsua_player_get_conf_port(ringtonePlayer), 0);

    pjsua_call_get_info(callId, &ci);

    if(ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}

static void onRegistrationState(pjsua_acc_id id) {
	pjsua_acc_info info;

	if(pjsua_acc_get_info(id, &info) == PJ_SUCCESS) {
		if(info.status == 200 && info.is_default) {
			registered = true;
		    DEBUGMSG("MMSSIP", (registered ? "registered" : "not registered"));
		}
	    DEBUGMSG("MMSSIP", "account: %s", info.acc_uri);
	    DEBUGMSG("MMSSIP", "status: %d", info.status);
	    DEBUGMSG("MMSSIP", "status_text: %s", info.status_text);
	    DEBUGMSG("MMSSIP", "online_status: %d", info.online_status);
	    DEBUGMSG("MMSSIP", "online_status_text: %s", info.online_status_text);

	    /* set online if registration successfull and online status 0 */
	    if(registered && !info.online_status) {
	    	if(pjsua_acc_set_online_status(id,	PJ_TRUE) == PJ_SUCCESS)
	    		DEBUGMSG("MMSSIP", "Setting online status successfull");
	    	else
	    		DEBUGMSG("MMSSIP", "Setting online status failed");
	    }
	}
}

static void onBuddyState(pjsua_buddy_id id) {
    pjsua_buddy_info info;

    if(pjsua_buddy_get_info(id, &info) == PJ_SUCCESS) {
        if(thiz && thiz->onBuddyStatus)
            thiz->onBuddyStatus->emit(thiz->getBuddy(id));
    }
}

#endif /* __HAVE_MMSSIP__ */
