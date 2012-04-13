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

#include "mmsgui/mmswindowaction.h"

MMSWindowAction::MMSWindowAction(class MMSWindow* destination) : MMSThread("MMSWindowAction") {
	this->window = destination;
	this->id = idFactory.getId();
	this->action = MMSWACTION_NONE;
	this->stopaction = false;
	this->cancelBroadcast.connect(sigc::mem_fun(this,&MMSWindowAction::onCancelBroadcast));
}

MMSWindowAction::~MMSWindowAction() {
}

int hideCnt = 0;

void MMSWindowAction::threadMain() {

    try {

    	this->stopaction = false;

    	switch(this->action) {
    		case MMSWACTION_SHOW:
                while (hideCnt) msleep(100);
//    			this->window->showAction(&(this->stopaction));
    			break;
    		case MMSWACTION_HIDE:
                hideCnt++;
//    			this->window->hideAction(&(this->stopaction));
                hideCnt--;
    			break;
    		default:
    			break;
    	}
    	this->stopaction = false;
    	this->action = MMSWACTION_NONE;

    } catch(MMSError &error) {
    	DEBUGMSG("MMSGUI", "Error: " + error.getMessage());
    }
}


void MMSWindowAction::cancelCleanup() {
	this->window->instantHide();
}

void MMSWindowAction::onCancelBroadcast(int type) {
	if (this->isRunning()) {
        if (this->action == MMSWACTION_HIDE) {
            while (this->isRunning()) msleep(100);
            return;
        }

		this->stopaction = true;
		int i = 0;
		while (this->stopaction) {
			msleep(100);
			i++;
			if (i > 20) this->cancel();
		}
		if (!this->stopaction) {
//			this->window->hideAction(&(this->stopaction));
        }
		else
			this->stopaction = false;
	}
}

void MMSWindowAction::setAction(MMSWACTION action) {
	this->action=action;
}

MMSWACTION MMSWindowAction::getAction() {
	return this->action;
}

sigc::signal<void,int> MMSWindowAction::cancelBroadcast;

