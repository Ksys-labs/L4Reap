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

#ifndef IMMSSWITCHER_H_
#define IMMSSWITCHER_H_

#include "mmsgui/mmschildwindow.h"
#include "mmsconfig/mmsplugindata.h"

class MMSPluginManager;
class MMSInputManager;

class IMMSSwitcher {
    public:
        virtual ~IMMSSwitcher() {};

        virtual void show() = 0;
        virtual void hide() = 0;

		virtual void setWindowManager(IMMSWindowManager *wm) = 0;
		virtual void setPluginManager(MMSPluginManager *pm) = 0;
		virtual void setInputManager(MMSInputManager  *im) = 0;

		virtual MMSChildWindow* loadPreviewDialog(string filename, MMSTheme *theme = NULL, int id=-1) = 0;
        virtual MMSChildWindow* loadInfoBarDialog(string filename, MMSTheme *theme = NULL) = 0;

        virtual void setVolume(unsigned int volume, bool init = false) = 0;

        virtual IMMSSwitcher *newSwitcher(MMSPluginData *plugindata) = 0;

        virtual bool switchToPlugin() = 0;

        /* go back to the previous plugin */
        virtual bool revertToLastPlugin() = 0;

        virtual bool leavePlugin(bool show_switcher) = 0;

        /* refresh switchers main window */
        virtual void refresh() = 0;
        /**
         * load a generic childwindow to be shown by the plugin
         *
         * @param	application specific data
         *
         * @return	application specific data
         */
		virtual MMSChildWindow* loadChildWindow(string filename, MMSTheme *theme = NULL) = 0;

        /**
         * Generic callback for plugin->switcher communication.
         *
         * @param	application specific data
         *
         * @return	application specific data
         */
        virtual void* callback(void* data) = 0;

        /* refresh switchers main window */
        virtual MMSWidget *getMyButton() = 0;


};

#endif /*IMMSSWITCHER_H_*/
