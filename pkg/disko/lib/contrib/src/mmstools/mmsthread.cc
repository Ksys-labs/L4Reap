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

#include "mmstools/mmsthread.h"
#include "mmstools/mmserror.h"
#include "mmstools/tools.h"
#include <map>

#ifdef __HAVE_DIRECTFB__
extern "C" {
#include <direct/debug.h>
#include <direct/thread.h>
#include <direct/trace.h>
}

D_DEBUG_DOMAIN( MMS_Thread, "MMS/Thread", "MMS Thread" );

#endif /* __HAVE_DIRECTFB__ */


typedef struct {
	void *udata;
	void (*handlerfunc)(void *);
} CLEANUP_STRUCT;


static std::map<pthread_t, CLEANUP_STRUCT *> cleanups;


MMSThread::MMSThread(string identity, int priority, bool autodetach) {
#ifdef __HAVE_DIRECTFB__
    D_DEBUG_AT( MMS_Thread, "MMSThread( %s )\n", identity.c_str() );

    direct_trace_print_stack(NULL);
#endif /* __HAVE_DIRECTFB__ */

    // save parameters
    this->identity = identity;
    this->priority = priority;

    // setup initial values
    this->starting = false;
    this->running = false;
    this->detached = false;
    this->autodetach = autodetach;
    setStacksize();
}


MMSThread::~MMSThread() {
}


void *MMSThread::runThread(void *thiz) {
	static_cast<MMSThread *>(thiz)->run();
	return NULL;
}


void MMSThread::run() {
	try {
#ifdef __HAVE_DIRECTFB__
        direct_thread_set_name( this->identity.c_str() );
#endif /* __HAVE_DIRECTFB__ */
        if(this->autodetach) {
        	this->detach();
        }

        // switch from starting state to running
        this->running = true;
        this->starting = false;

        // call real routine
		threadMain();

		// mark thread as stopped
        this->running = false;

	} catch(MMSError &error) {
        this->running = false;
        this->starting = false;
	    DEBUGMSG(this->identity.c_str(), "Abort due to: " + error.getMessage());
	}
}

bool MMSThread::start() {
	// safe start
	// we have two states: starting and running
	this->startlock.lock();
	if (isRunning()) {
		this->startlock.unlock();
		return false;
	}
	this->starting = true;
	this->startlock.unlock();

	// starting thread, setup priority and stacksize
    pthread_attr_init(&this->tattr);
    pthread_attr_getschedparam(&this->tattr, &this->param);
    this->param.sched_priority = this->priority;
    pthread_attr_setschedparam(&this->tattr, &this->param);
    pthread_attr_setstacksize(&this->tattr, this->stacksize);

    // create the new thread
    int rc;
	for (int i = 0; i < 3; i++) {
		// create: try or retry
		rc = pthread_create(&this->id, &this->tattr, this->runThread, static_cast<void *>(this));
		if (!rc) {
			// successfully created
			break;
		}
		usleep(50000);
	}

    // free attributes from "parent" thread
    pthread_attr_destroy(&this->tattr);

    if (rc) {
    	// failed to start the thread
    	this->starting = false;
    	return false;
    }

    // fine, thread is started
    // at this point it is also possible, that the new thread is finished again
    return true;
}


bool MMSThread::isRunning() {
	// check starting and running states
	if (this->starting) return true;
	return this->running;
}


void MMSThread::detach() {
	pthread_detach(this->id);
	this->detached = true;
}


bool MMSThread::cancel() {
	if (!isRunning()) {
		// thread is not running
		return false;
	}

	// try to cancel the thread
	int rc;
	for (int i = 0; i < 3; i++) {
		// cancel: try or retry
		rc = pthread_cancel(this->id);
		if (!rc) {
			// successfully canceled
			break;
		}
	}

	if (rc) {
		// could not cancel the thread
		return false;
	}

	// mark thread as stopped
	this->running = false;
	this->starting = false;
	return true;
}


void MMSThread::join() {
    if (!this->detached)
        pthread_join(this->id, NULL);
}


void MMSThread::setStacksize(size_t stacksize) {
	this->stacksize = stacksize;
}


void addGarbageHandler(void (*handlerfunc)(void *), void *data) {
	CLEANUP_STRUCT *item = new CLEANUP_STRUCT;
	std::map<pthread_t, CLEANUP_STRUCT *>::iterator it;
	pthread_t self = pthread_self();

	item->handlerfunc=handlerfunc;
	item->udata=data;

	it=cleanups.find(self);
	if(it!=cleanups.end()) {
		if(it->second)
			delete it->second;
		it->second = item;
	} else {
		cleanups.insert(std::make_pair(self,item));
	}
}

void callGarbageHandler() {
	std::map<pthread_t, CLEANUP_STRUCT *>::iterator it;
	pthread_t self = pthread_self();

	it = cleanups.find(self);
	if(it!=cleanups.end()) {
		// call the garbage handler
		it->second->handlerfunc(it->second->udata);

		// remove handler
		delete it->second;
		cleanups.erase(self);
	}
}

void clearGarbageHandler() {
	std::map<pthread_t, CLEANUP_STRUCT *>::iterator it;
	pthread_t self = pthread_self();

	it = cleanups.find(self);
	if(it!=cleanups.end()) {

		// remove handler
		delete it->second;
		cleanups.erase(self);
	}
}

