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

#ifndef MMSBACKENDPLUGINHANDLER_H_
#define MMSBACKENDPLUGINHANDLER_H_

#include "mmstools/mmserror.h"
#include "mmsconfig/mmsplugindata.h"
#include "mmsbase/interfaces/immsbackendplugin.h"
#include "mmstools/mmsshlhandler.h"

MMS_CREATEERROR(MMSBackendPluginError);

class MMSBackendPluginHandler {
    private:
        bool 				loaded;
        bool				initialized;
        MMSPluginData 		plugindata;
        IMMSBackendPlugin 	*plugin;
        MMSShlHandler 		*handler;
        MMSMutex 			calllock;
        IMMSSwitcher 		*switcher;

    public:
        /**
         * Constructor for the plugin handler of a backend plugin.
         *
         * @param	plugindata	The plugindata object as usually read from the database.
         * @param	autoload	If true, the plugin will be loaded (no effect if _plugin is set)
         * @param	_plugin		Pointer to already instantiated plugin object. Usually used for static plugins.
         */
        MMSBackendPluginHandler(MMSPluginData plugindata, bool autoload = false, IMMSBackendPlugin *_plugin = NULL);
        ~MMSBackendPluginHandler();

        bool isLoaded();
        bool isInitialized();
        void load();
        void unload();
        MMSPluginData getPluginData();
        void setPluginData(MMSPluginData plugindata);
        void setSwitcherInterface(IMMSSwitcher *switcher);
        void invokeInitialize(void *data = NULL);
        void invokeOnEvent(IMMSEvent event);
        void invokeShutdown(void *data = NULL);

};

#endif /*MMSBACKENDPLUGINHANDLER_H_*/
