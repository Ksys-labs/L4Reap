/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#include "mmsinput/mmsinputthread.h"
#include "mmsinput/mmsinputdfbhandler.h"
#include "mmsinput/mmsinputx11handler.h"
#include "mmsinput/mmsinputlishandler.h"
#include "mmsinput/mmsinputl4rehandler.h"
#include "mmsgui/fb/mmsfb.h"

extern MMSFB *mmsfb;


MMSInputThread::MMSInputThread(class MMSInputManager *manager, MMS_INPUT_DEVICE device, int inputinterval) {
	this->manager = manager;
	this->handler = NULL;
	this->device = device;
    this->inputinterval = inputinterval;
}

MMSInputThread::~MMSInputThread() {
	delete handler;
}

void MMSInputThread::threadMain() {
	MMSInputEvent			event, lastevent;
    time_t                  aclock;
    struct timeval          tv;
    double                  currtime, lasttime = 0;
    int                     lkcnt = 0, skip = 0;

    lastevent.type = MMSINPUTEVENTTYPE_NONE;

	if (this->handler == NULL) {
#ifndef __L4_RE__
		if (mmsfb->getBackend()==MMSFB_BE_DFB) {
			this->handler = new MMSInputDFBHandler(this->device);
		}
		else
		if (mmsfb->getBackend()==MMSFB_BE_FBDEV) {
			this->handler = new MMSInputLISHandler(this->device);
		}
		else
		if (mmsfb->getBackend()==MMSFB_BE_X11) {
			this->handler = new MMSInputX11Handler(this->device);
		}
#else
		if (mmsfb->getBackend()==MMSFB_BE_L4RE) {
			this->handler = new MMSInputL4REHandler(this->device);
		}
#endif


/*		if(config->getOutputType()!= MMS_OT_X11FB) {
			//create dfb input backend
			this->handler = new MMSInputDFBHandler(this->device);
		} else {
			#ifdef __HAVE_XLIB__
				if(mmsfb->getBackend()==MMSFB_BACKEND_X11) {
					this->handler = new MMSInputX11Handler(this->device);
				} else {
					//allthough we have xlib directfb is used;
					this->handler = new MMSInputDFBHandler(this->device);
				}
			#else
				//we have no xlib so its dfb x11
				this->handler = new MMSInputDFBHandler(this->device);
			#endif
		}
*/
	}

	while (1) {
		this->handler->grabEvents(&event);

		if (event.type == MMSINPUTEVENTTYPE_KEYPRESS) {
	        if (this->inputinterval > 0) {
	            /* here we stop to fast input devices */

	            /* get time in milliseconds */
	            time(&aclock);
	            gettimeofday(&tv, NULL);
	            currtime = ((double) aclock) * 1000 + ((double) tv.tv_usec) / 1000;

	            if (event.key == lastevent.key) {
	                /* check if input comes slow enough */
	                if (currtime < lasttime + ((!skip)?this->inputinterval:(this->inputinterval/2))) {
	                    /* to fast, ignore the input */
	                    lkcnt++;
	                    continue;
	                }
	            }
	            else
	                lkcnt = 0;

	            /* save the last event */
	            lastevent = event;

	            /* check if the last input is to old */
	            skip=0;
	            if (lkcnt > 0) {
	                if (currtime > lasttime + this->inputinterval*2)
	                    /* to old */
	                    lkcnt = 0;
	                else
	                    /* the next loop we want the input faster */
	                    skip = 1;
	            }

	            /* save current time */
	            lasttime = currtime;
	        }
		}

        /* to the input manager */
		this->manager->handleInput(&event);

		/* call garbage handler */
		callGarbageHandler();
	}
}

