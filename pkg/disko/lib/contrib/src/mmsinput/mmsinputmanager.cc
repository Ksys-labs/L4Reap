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

#include "mmsinput/mmsinputmanager.h"

#ifndef __LIS_DEBUG__
#undef MSG2OUT
#define MSG2OUT(ident, msg...)
#endif

MMSInputManager::MMSInputManager(string file, string name) {
	this->mapper = new MMSInputMapper(file, name);
	this->config = new MMSConfigData();
	this->buttonpress_window = NULL;
	this->button_pressed = false;
	clock_gettime(CLOCK_REALTIME,&this->lastinput);
}

MMSInputManager::~MMSInputManager() {
    this->threads.clear();
    this->subscriptions.clear();
    if(this->mapper) delete this->mapper;
    if(this->config) delete this->config;
}

void MMSInputManager::handleInput(MMSInputEvent *inputevent) {
	MMSWindow *window=NULL;

	this->mutex.lock();

	//lock mmsfb to ensure inputs are not interrupted by other threads
	mmsfb->lock();

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		/* keyboard inputs */

#ifdef __ENABLE_DEBUG__
		/* check crtl+c and exit */
		if((inputevent->key==MMSKEY_SMALL_C)&&(this->lastkey==MMSKEY_CONTROL))
			exit(1);
#endif

/*
#ifdef ROTATE_180
		switch (inputevent->key) {
		case MMSKEY_CURSOR_LEFT:
			inputevent->key = MMSKEY_CURSOR_RIGHT;
			break;
		case MMSKEY_CURSOR_RIGHT:
			inputevent->key = MMSKEY_CURSOR_LEFT;
			break;
		case MMSKEY_CURSOR_UP:
			inputevent->key = MMSKEY_CURSOR_DOWN;
			break;
		case MMSKEY_CURSOR_DOWN:
			inputevent->key = MMSKEY_CURSOR_UP;
			break;
		default:
			break;
		}
#endif
*/

		this->lastkey = inputevent->key;

		this->mapper->mapkey(inputevent);

#if __ENABLE_LOG__ || __ENABLE_DEBUG__
		string symbol = mmskeys[inputevent->key];
		TRACEOUT("MMSINPUT", "KEY PRESS %d (MMSKEY_%s)", this->lastkey, symbol.c_str());
		if(lastkey != inputevent->key) {
			symbol = mmskeys[inputevent->key];
			TRACEOUT("MMSINPUT", " >MAPPED TO %d (MMSKEY_%s)", inputevent->key, symbol.c_str());
		}
#endif

		if((inputevent->key==MMSKEY_POWER)||(inputevent->key==MMSKEY_POWER2)) {
			if(config->getShutdown() == true) {
				DEBUGMSG("MMSINPUTMANAGER", "executing: %s", config->getShutdownCmd().c_str());

				executeCmd(config->getShutdownCmd());
				sleep(30);
			}
			exit(0);
		}

		window = this->windowmanager->getToplevelWindow();

		if(window!=NULL) {
			if	((inputevent->key==MMSKEY_CURSOR_DOWN)||(inputevent->key==MMSKEY_CURSOR_UP)
				||(inputevent->key==MMSKEY_CURSOR_LEFT)||(inputevent->key==MMSKEY_CURSOR_RIGHT)) {
				/* ok execute input on window */
				window->handleInput(inputevent);
				memset(inputevent, 0, sizeof(MMSInputEvent));
				this->mutex.unlock();
				mmsfb->unlock();
				return;
			}
		}

		// have to call subscriptions?
		bool call_subscriptions = true;
		if (window) {
			bool modal = false;
			window->getModal(modal);
			if (modal)
				call_subscriptions = false;
		}

		if (call_subscriptions) {
			// go through subscriptions
			for(unsigned int i = 0; i < subscriptions.size();i++) {
				MMSKeySymbol key;
				if (subscriptions.at(i)->getKey(key)) {
					if (key == inputevent->key) {
						DEBUGMSG("MMSINPUTMANAGER", "found a subscription");
						// ok i found one execute
						subscriptions.at(i)->callback.emit(subscriptions.at(i));
						// stop it only one key per subscription
						DEBUGMSG("MMSINPUTMANAGER", "returning from handle input");
						mmsfb->unlock();
						this->mutex.unlock();
						return;
					}
				}
			}
		}

		if(window != NULL)
			window->handleInput(inputevent);
			memset(inputevent, 0, sizeof(MMSInputEvent));
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_KEYRELEASE) {
		/* keyboard inputs */
#if __ENABLE_LOG__ || __ENABLE_DEBUG__
		string symbol = mmskeys[inputevent->key];
		TRACEOUT("MMSINPUT", "KEY RELEASE %d (MMSKEY_%s)", this->lastkey, symbol.c_str());
#endif
		MMSKeySymbol beforemap = inputevent->key;
		this->mapper->mapkey(inputevent);
#if __ENABLE_LOG__ || __ENABLE_DEBUG__
		if(inputevent->key != beforemap) {
			symbol = mmskeys[inputevent->key];
			TRACEOUT("MMSINPUT", " >MAPPED TO %d (MMSKEY_%s)", inputevent->key, symbol.c_str());
		}
#endif

		window = this->windowmanager->getToplevelWindow();

		if(window != NULL)
			window->handleInput(inputevent);
			memset(inputevent, 0, sizeof(MMSInputEvent));
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED AT: %d,%d", inputevent->posx, inputevent->posy);
		MSG2OUT("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED AT: %d,%d", inputevent->posx, inputevent->posy);
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME,&ts);
		int64_t diff = timespecDiff(&ts, &this->lastinput);
		//printf("diff %ld\n",diff);
		/*if(diff < 130000000) {
			memset(inputevent, 0, sizeof(MMSInputEvent));
			return;
		}*/
		clock_gettime(CLOCK_REALTIME,&this->lastinput);

		this->button_pressed = true;
		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, true);

		this->oldx = inputevent->posx;
		this->oldy = inputevent->posy;
		window = this->windowmanager->getToplevelWindow();
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((inputevent->posx - rect.x < 0)||(inputevent->posy - rect.y < 0)
					||(inputevent->posx - rect.x - rect.w >= 0)||(inputevent->posy - rect.y - rect.h >= 0)) {
				/* pointer is not over the window */
				DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED, NOT OVER THE WINDOW");
				MSG2OUT("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED, NOT OVER THE WINDOW");

				this->mutex.unlock();
				memset(inputevent, 0, sizeof(MMSInputEvent));
				return;
			}
			if(inputevent->posx < 0 || inputevent->posy<0) {
				inputevent->absx = this->oldx;
				inputevent->absy = this->oldy;
			} else {
				this->oldx = inputevent->posx;
				this->oldy = inputevent->posy;
			}


			// save the pointer for release event
			this->buttonpress_window = window;

			inputevent->absx = inputevent->posx;
			inputevent->absy = inputevent->posy;
			inputevent->posx-= rect.x;
			inputevent->posy-= rect.y;
			inputevent->dx = 0;
			inputevent->dy = 0;

			window->handleInput(inputevent);
			memset(inputevent, 0, sizeof(MMSInputEvent));
		}
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON RELEASED AT: %d,%d", inputevent->posx, inputevent->posy);
//		MSG2OUT("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON RELEASED AT: %d,%d", inputevent->posx, inputevent->posy);
		this->button_pressed = false;

		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, false);

		window = this->windowmanager->getToplevelWindow();
		if (!window)
			window = this->buttonpress_window;
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((window == this->buttonpress_window)
				||   ((this->buttonpress_window)
					&&(inputevent->posx - rect.x >= 0)&&(inputevent->posy - rect.y >= 0)
					&& (inputevent->posx - rect.x - rect.w < 0)&&(inputevent->posy - rect.y - rect.h < 0))) {
				/* call windows handleInput with normalized coordinates */

				if(inputevent->posx < 0 || inputevent->posy<0) {
					inputevent->absx = this->oldx;
					inputevent->absy = this->oldy;
				}
				inputevent->absx = inputevent->posx;
				inputevent->absy = inputevent->posy;
				inputevent->posx-= rect.x;
				inputevent->posy-= rect.y;
				inputevent->dx = inputevent->absx - this->oldx;
				inputevent->dy = inputevent->absy - this->oldy;

				this->oldx = -1;
				this->oldy = -1;

				if (window->handleInput(inputevent)) {
					this->buttonpress_window = NULL;
					memset(inputevent, 0, sizeof(MMSInputEvent));
					mmsfb->unlock();
					this->mutex.unlock();
					return;
				}
			}
		}
		this->buttonpress_window = NULL;


		// have to call subscriptions?
		bool call_subscriptions = true;
		if (window) {
			bool modal = false;
			window->getModal(modal);
			if (modal)
				call_subscriptions = false;
		}

		if (call_subscriptions) {
			// go through subscriptions
			for(unsigned int i = 0; i < subscriptions.size();i++) {
				MMSFBRectangle pointer_area;
				if (subscriptions.at(i)->getPointerArea(pointer_area)) {
					if ((inputevent->posx >= pointer_area.x)&&(inputevent->posy >= pointer_area.y)
					  &&(inputevent->posx < pointer_area.x + pointer_area.w)&&(inputevent->posy < pointer_area.y + pointer_area.h)) {
						DEBUGMSG("MMSINPUTMANAGER", "found a subscription");
						// ok i found one execute
						subscriptions.at(i)->callback.emit(subscriptions.at(i));
						// stop it only one key per subscription
						DEBUGMSG("MMSINPUTMANAGER", "returning from handle input");
						memset(inputevent, 0, sizeof(MMSInputEvent));
						this->mutex.unlock();
						return;
					}
				}
			}
		}

	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_AXISMOTION) {

		/*this->oldx = inputevent->absx;
		this->oldy = inputevent->absy;
		memset(inputevent, 0, sizeof(MMSInputEvent));
		return;
		*/

		/* */
		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, this->button_pressed);


		window = this->windowmanager->getToplevelWindow();
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((inputevent->posx - rect.x < 0)||(inputevent->posy - rect.y < 0)
					||(inputevent->posx - rect.x - rect.w >= 0)||(inputevent->posy - rect.y - rect.h >= 0)) {
				/* pointer is not over the window */
				mmsfb->unlock();
				this->mutex.unlock();
				return;
			}

			inputevent->absx = inputevent->posx;
			inputevent->absy = inputevent->posy;
			inputevent->posx-=rect.x;
			inputevent->posy-=rect.y;
			if(this->button_pressed) {
				inputevent->dx = inputevent->absx - this->oldx;
				inputevent->dy = inputevent->absy - this->oldy;
			} else {
				inputevent->dx = 0;
				inputevent->dy = 0;
			}

			if(this->oldx == inputevent->absx && this->oldy == inputevent->absy) {

				memset(inputevent, 0, sizeof(MMSInputEvent));
				mmsfb->unlock();
				this->mutex.unlock();
				return;
			}

#ifdef __L4_RE__
#else
			//printf("oldx = %d\n", this->oldx);
			fflush(stdout);
#endif
			this->oldx = inputevent->absx;
			this->oldy = inputevent->absy;


			window->handleInput(inputevent);
			memset(inputevent, 0, sizeof(MMSInputEvent));
		}
	}
	else
    if (inputevent->type == MMSINPUTEVENTTYPE_TSCALIBRATION) {
        window = this->windowmanager->getToplevelWindow();
        if (window) {
            window->handleInput(inputevent);
            memset(inputevent, 0, sizeof(MMSInputEvent));
        }
    }

	mmsfb->unlock();
	this->mutex.unlock();
}

void MMSInputManager::addDevice(MMS_INPUT_DEVICE device, int inputinterval) {
	MMSInputThread *thread = new MMSInputThread(this, device, inputinterval);

	this->threads.push_back(thread);

}

void MMSInputManager::setWindowManager(IMMSWindowManager *wm) {
	this->windowmanager = wm;

}

void MMSInputManager::startListen() {
	for(unsigned int i=0; i<this->threads.size();i++) {
		this->threads.at(i)->start();
	}
}

void MMSInputManager::stopListen() {
	for(unsigned int i=0; i<this->threads.size();i++) {
		this->threads.at(i)->cancel();
	}
}


void MMSInputManager::addSubscription(class MMSInputSubscription *sub)  {
	this->subscriptions.push_back(sub);
}
