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

#include "mmsinput/mmsinputlishandler.h"
#include "mmsinput/mmsinputlisthread.h"
#include "mmsconfig/mmsconfigdata.h"
#include <cstring>
#include <typeinfo>
#include <sys/ioctl.h>

#include <linux/input.h>


#ifdef __HAVE_FBDEV__
MMSInputLISHandler::MMSInputLISHandler(MMS_INPUT_DEVICE device) :
	devcnt(0),
	ie_count(0),
	ie_read_pos(0),
	ie_write_pos(0) {

	getDevices();
	for (int i = 0; i < this->devcnt; i++) {
		if(this->devices[i].type != MMSINPUTLISHANDLER_DEVTYPE_UNKNOWN) {
			MMSInputLISThread *lt = new MMSInputLISThread(this, &this->devices[i]);
			if (lt) {
				lt->start();
			}
		}
	}
}
#else
MMSInputLISHandler::MMSInputLISHandler(MMS_INPUT_DEVICE device) {
	throw MMSError(0,(string)typeid(this).name() + " is empty. compile FBDEV support!");
}
#endif

MMSInputLISHandler::~MMSInputLISHandler() {
}


#define NUMBITS(x)          ((((x)-1)/(sizeof(long)*8))+1)
#define TSTBIT(bit, array)	((array[(bit)/(sizeof(long)*8)] >> ((bit)%(sizeof(long)*8))) & 1)

bool MMSInputLISHandler::checkDevice() {
    // point to the device
    MMSINPUTLISHANDLER_DEV *dev = &this->devices[this->devcnt];

    // open the device
    int  fd;
    if ((fd = open(dev->name.c_str(), O_RDWR)) < 0)
         return false;

	// try to grab the device
	if (ioctl(fd, EVIOCGRAB, 1)) {
		close(fd);
		return false;
	}

	// get description
	char devdesc[256];
	memset(devdesc, 0, sizeof(devdesc));
	ioctl(fd, EVIOCGNAME(sizeof(devdesc)-1), devdesc);
	dev->desc = devdesc;
	dev->type = MMSINPUTLISHANDLER_DEVTYPE_UNKNOWN;

	// try to find out the type of input device
	unsigned int  keys = 0;
	unsigned long ev_bit[NUMBITS(EV_MAX)];
	unsigned long key_bit[NUMBITS(KEY_MAX)];
	unsigned long abs_bit[NUMBITS(ABS_MAX)];

    // get event type of device
    ioctl(fd, EVIOCGBIT(0, sizeof(ev_bit)), ev_bit);
    if(TSTBIT(EV_KEY, ev_bit)) {
		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bit)), key_bit);
		for(int i = KEY_Q; i < KEY_M; i++)
			if (TSTBIT(i, key_bit))
				keys++;
		if(keys > 20 || TSTBIT(KEY_ENTER, key_bit)) {
			dev->type = MMSINPUTLISHANDLER_DEVTYPE_KEYBOARD;
		} else {
			for(int i = KEY_OK; i < KEY_MAX; i++) {
				if(TSTBIT(i, key_bit)) {
					dev->type = MMSINPUTLISHANDLER_DEVTYPE_REMOTE;
					break;
				}
			}
		}
    }

	/* check for touchscreen */
	if(dev->type == MMSINPUTLISHANDLER_DEVTYPE_UNKNOWN) {
		if(ioctl(fd, EVIOCGBIT(EV_ABS, sizeof (abs_bit)), abs_bit) != -1) {
			if(TSTBIT(ABS_X, abs_bit) && TSTBIT(ABS_Y, abs_bit)) {
				if(TSTBIT(EV_KEY, ev_bit) && (TSTBIT(BTN_LEFT, key_bit) || TSTBIT(BTN_TOUCH, key_bit))) {
					dev->touch.haveBtnEvents = true;
					dev->type = MMSINPUTLISHANDLER_DEVTYPE_TOUCHSCREEN;
				} else if(TSTBIT(ABS_PRESSURE, abs_bit)) {
					dev->touch.haveBtnEvents = false;
					dev->type = MMSINPUTLISHANDLER_DEVTYPE_TOUCHSCREEN;
				}
			}
		}

		if(dev->type == MMSINPUTLISHANDLER_DEVTYPE_TOUCHSCREEN) {
			struct input_absinfo abs;
			MMSConfigData config;
			MMSFBRectangle vRect = config.getVRect();

			/* use the graphicslayer resolution if vrect isn't set */
			if(vRect.w <= 0) {
				vRect = config.getGraphicsLayer().rect;
			}

			dev->touch.rect 	= config.getTouchRect();
			dev->touch.swapX 	= config.getTouchSwapX();
			dev->touch.swapY 	= config.getTouchSwapY();
			dev->touch.swapXY 	= config.getTouchSwapXY();

			if(dev->touch.rect.w) {
				dev->touch.xFactor = (float)vRect.w / (float)dev->touch.rect.w;
			} else if(ioctl(fd, EVIOCGABS(ABS_X), &abs) != -1) {
				if(dev->touch.swapXY) {
					dev->touch.yFactor =  (float)vRect.h / (abs.maximum - abs.minimum);
				} else {
					dev->touch.xFactor =  (float)vRect.w / (abs.maximum - abs.minimum);
				}
			} else {
				dev->touch.xFactor = 1.0;
			}
			if(dev->touch.rect.h) {
				dev->touch.yFactor = (float)vRect.h / (float)dev->touch.rect.h;
			} else if(ioctl(fd, EVIOCGABS(ABS_Y), &abs) != -1) {
				if(dev->touch.swapXY) {
					dev->touch.xFactor = (float)vRect.w / (abs.maximum - abs.minimum);
				} else {
					dev->touch.yFactor = (float)vRect.h / (abs.maximum - abs.minimum);
				}
			} else {
				dev->touch.yFactor = 1.0;
			}
		}
	}

	printf("Found %s, type=%s (%s)\n",
						dev->name.c_str(),
						dev->type.c_str(),
						dev->desc.c_str());

	// release device
	ioctl(fd, EVIOCGRAB, 0);
	close(fd);

	return true;
}

void MMSInputLISHandler::getDevices() {
	for (int i = 0; i < MMSINPUTLISHANDLER_MAX_DEVICES; i++) {
		this->devices[this->devcnt].name = "/dev/input/event" + iToStr(i);
		if (checkDevice())
			this->devcnt++;
	}
}

bool MMSInputLISHandler::addEvent(MMSInputEvent *inputevent) {
	this->lock.lock();

	// block if buffer is full
	while(this->ie_count == (MMSINPUTLISHANDLER_EVENT_BUFFER_SIZE - 1)) {
		usleep(10000);
	}
	// add event
	this->ie_buffer[this->ie_write_pos] = *inputevent;

	// increase event counter
	this->ie_count++;

	// increase write position
	this->ie_write_pos++;
	if(this->ie_write_pos >= MMSINPUTLISHANDLER_EVENT_BUFFER_SIZE)
		this->ie_write_pos = 0;

	this->lock.unlock();
	return true;
}

void MMSInputLISHandler::grabEvents(MMSInputEvent *inputevent) {
#ifdef __HAVE_FBDEV__
	// block if buffer is empty
	while(this->ie_count == 0) {
		usleep(10000);
	}

	// there is at least one event in the buffer, return next event
	*inputevent = this->ie_buffer[this->ie_read_pos];

	// decrease event counter
	this->ie_count--;

	// increase read position
	this->ie_read_pos++;
	if (this->ie_read_pos >= MMSINPUTLISHANDLER_EVENT_BUFFER_SIZE)
		this->ie_read_pos = 0;
#else
	throw MMSError(0,(string)typeid(this).name() + " is empty. compile FBDEV support!");
#endif
}

