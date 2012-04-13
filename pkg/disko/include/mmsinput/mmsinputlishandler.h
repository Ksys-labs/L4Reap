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

#ifndef MMSINPUTLISHANDLER_H_
#define MMSINPUTLISHANDLER_H_

#include "mmsgui/mmsguitools.h"
#include "mmsinput/mmsinputhandler.h"

//! maximum number of linux input devices
#define MMSINPUTLISHANDLER_MAX_DEVICES 	16

#define MMSINPUTLISHANDLER_EVENT_BUFFER_SIZE	100

typedef int MMSINPUTLISHANDLER_DEVTYPE;

#define MMSINPUTLISHANDLER_DEVTYPE_UNKNOWN		"UNKNOWN"
#define MMSINPUTLISHANDLER_DEVTYPE_KEYBOARD		"KEYBOARD"
#define MMSINPUTLISHANDLER_DEVTYPE_REMOTE		"REMOTE"
#define MMSINPUTLISHANDLER_DEVTYPE_TOUCHSCREEN	"TOUCHSCREEN"

typedef struct {
	float 			xFactor;		/**< multiplicate the x value to get the real value (touchscreen only) */
	float 			yFactor;		/**< multiplicate the y value to get the real value (touchscreen only) */
	bool  			swapX;			/**< swap x axis */
	bool  			swapY;			/**< swap y axis */
	bool			swapXY;			/**< swap x and y axis */
	MMSFBRectangle	rect;			/**< specifies resolution of touch controller */
	bool			haveBtnEvents;	/**< touch driver sends BTN_xxx events */
} MMSINPUTLISHANDLER_DEV_TOUCH;

typedef struct {
	string 	name;
	string	desc;
	string	type;

	MMSINPUTLISHANDLER_DEV_TOUCH touch;
} MMSINPUTLISHANDLER_DEV;


class MMSInputLISHandler : public MMSInputHandler {
	private:
		//! available input devices
		MMSINPUTLISHANDLER_DEV devices[MMSINPUTLISHANDLER_MAX_DEVICES];

		//! number of available input devices
		int	devcnt;


		//! event ring buffer
		MMSInputEvent 	ie_buffer[MMSINPUTLISHANDLER_EVENT_BUFFER_SIZE];

		//! current number of events in ring buffer
		unsigned char	ie_count;

		//! event ring buffer, read pos
		unsigned char 	ie_read_pos;

		//! event ring buffer, write pos
		unsigned char	ie_write_pos;

		class MMSInputLISThread	*listhread;

        //! lock
        MMSMutex lock;

        bool checkDevice();
        void getDevices();

		bool addEvent(MMSInputEvent *inputevent);

	public:
		MMSInputLISHandler(MMS_INPUT_DEVICE device);
		~MMSInputLISHandler();
		void grabEvents(MMSInputEvent *inputevent);

	// friends
	friend class MMSInputLISThread;
};

#endif /* MMSINPUTLISHANDLER_H_ */
