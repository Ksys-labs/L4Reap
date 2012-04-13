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

#ifndef IMMSWINDOWMANAGER_H_
#define IMMSWINDOWMANAGER_H_

#include "mmsgui/mmswindow.h"
#include "mmsgui/theme/mmsthememanager.h"
#include "mmscore/mmstranslator.h"


class IMMSWindowManager {
    public:
        virtual ~IMMSWindowManager() {};

		virtual void reset() = 0;

        virtual MMSFBRectangle getVRect() = 0;

        virtual void addWindow(MMSWindow *window) = 0;
        virtual void removeWindow(MMSWindow *window) = 0;

        virtual bool lowerToBottom(MMSWindow *window) = 0;
		virtual bool raiseToTop(MMSWindow *window) = 0;

        virtual bool hideAllMainWindows(bool goback = false) = 0;
        virtual bool hideAllPopupWindows(bool except_modal = false) = 0;
        virtual bool hideAllRootWindows(bool willshown = false) = 0;

        virtual bool setToplevelWindow(MMSWindow *window) = 0;
        virtual MMSWindow *getToplevelWindow() = 0;
		virtual void removeWindowFromToplevel(MMSWindow *window) = 0;

        virtual void setBackgroundWindow(MMSWindow *window) = 0;
        virtual MMSWindow *getBackgroundWindow() = 0;

        virtual void setPointerPosition(int pointer_posx, int pointer_posy, bool pressed = false) = 0;

        virtual MMSTranslator *getTranslator() = 0;

        virtual MMSThemeManager *getThemeManager() = 0;

        virtual unsigned int printStack(char *buffer = NULL) = 0;
};

#endif /*IMMSWINDOWMANAGER_H_*/
