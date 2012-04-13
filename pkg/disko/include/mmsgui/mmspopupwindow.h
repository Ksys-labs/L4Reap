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

#ifndef MMSPOPUPWINDOW_H_
#define MMSPOPUPWINDOW_H_

#include "mmsgui/mmswindow.h"
#include "mmstools/mmstimer.h"

class MMSPopupWindow : public MMSWindow {

	private:
        string              className;
        MMSPopupWindowClass *popupWindowClass;
        MMSPopupWindowClass myPopupWindowClass;

        MMSTimer			*timer;
        sigc::connection 	timeOut_connection;

        bool create(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment,
                    MMSWINDOW_FLAGS flags, MMSTheme *theme, bool *own_surface, bool *backbuffer,
                    unsigned int duration);

	public:
		MMSPopupWindow(string className, string dx, string dy, string w, string h, MMSALIGNMENT alignment = MMSALIGNMENT_CENTER,
                       MMSWINDOW_FLAGS flags = MMSW_NONE, MMSTheme *theme = NULL, bool *own_surface = NULL,
                       bool *backbuffer = NULL, unsigned int duration = 0);
		MMSPopupWindow(string className, string w, string h, MMSALIGNMENT alignment = MMSALIGNMENT_CENTER,
                       MMSWINDOW_FLAGS flags = MMSW_NONE, MMSTheme *theme = NULL, bool *own_surface = NULL,
                       bool *backbuffer = NULL, unsigned int duration = 0);
        virtual ~MMSPopupWindow();

        void timeOut(void);
        virtual void afterShowAction(MMSPulser *pulser);
        virtual bool beforeHideAction(MMSPulser *pulser);

    public:
        /* theme access methods */
        unsigned int getDuration();

		/**
		 * Sets the duration in seconds to display the popupwindow when show()
		 * is called.
		 * 
		 * If the window is already shown, the current timeout is resetted.
		 * 
		 * @param	duration [in]	time in seconds to display the popupwindow
		 */
        void setDuration(unsigned int duration);

        void updateFromThemeClass(MMSPopupWindowClass *themeClass);
};

#endif /*MMSPOPUPWINDOW_H_*/
