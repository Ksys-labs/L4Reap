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

#ifndef MMSINPUTLISTHREAD_H_
#define MMSINPUTLISTHREAD_H_

#include "mmsinput/mmsinputlishandler.h"
#include <linux/keyboard.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <linux/vt.h>

#include <linux/input.h>

class MMSInputLISThread : public MMSThread {
	private:
		//! access to the input handler to buffer input events there
		MMSInputLISHandler	*handler;

		//! filedescriptor from which we read keyboard inputs (this should be the fd to the framebuffer console)
		int	kb_fd;

		// name of the device != keyboard
		MMSINPUTLISHANDLER_DEV device;

		//! filedescriptor of the device
		int dv_fd;

		//! shift key pressed
		bool shift_pressed;

		//! altgr key pressed
		bool altgr_pressed;

		//! is caps lock?
		bool is_caps_lock;

		//! button pressed?
		bool button_pressed;
		
		int lastX, lastY;
		struct input_event lastevent;

		bool openDevice();
		void closeDevice();

		MMSKeySymbol getSymbol(int code, unsigned short value);
		unsigned short readValue(unsigned char table, unsigned char index);
		MMSKeySymbol getKeyFromCode(bool pressed, unsigned char code);
		void updateLED();

		MMSKeySymbol translateKey(int code);
		bool translateEvent(struct input_event *linux_evt, MMSInputEvent *inputevent);

	public:
		MMSInputLISThread(MMSInputLISHandler *handler, int kb_fd);
		MMSInputLISThread(MMSInputLISHandler *handler, MMSINPUTLISHANDLER_DEV *device);
		~MMSInputLISThread();
		void threadMain();
};

#endif /* MMSINPUTLISTHREAD_H_ */
