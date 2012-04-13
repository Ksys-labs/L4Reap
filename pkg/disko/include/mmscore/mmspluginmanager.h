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

#ifndef MMSPLUGINMANAGER_H_
#define MMSPLUGINMANAGER_H_

#include "mmsbase/mmsosdpluginhandler.h"
#include "mmsbase/mmscentralpluginhandler.h"
#include "mmsbase/mmsimportpluginhandler.h"
#include "mmsbase/mmsbackendpluginhandler.h"
#include "mmsconfig/mmspluginservice.h"

MMS_CREATEERROR(MMSPluginManagerError);

class MMSPluginManager {
    private:
    	DataSource							*source;
        MMSPluginService                    *service;
        vector<MMSOSDPluginHandler *>       osdPluginHandlers;
        vector<MMSCentralPluginHandler *>   centralPluginHandlers;
        vector<MMSImportPluginHandler *>    importPluginHandlers;
        vector<MMSBackendPluginHandler *>   backendPluginHandlers;
        MMSPluginData						*activeosdplugin;
        MMSPluginData						*activecentralplugin;
        IMMSSwitcher 					    *switcher;

        map<string, IMMSOSDPlugin*>			staticOSDPlugins;
        map<string, IMMSCentralPlugin*>		staticCentralPlugins;
        map<string, IMMSImportPlugin*>		staticImportPlugins;
        map<string, IMMSBackendPlugin*>		staticBackendPlugins;

    public:
        MMSPluginManager();
        ~MMSPluginManager();

        /**
         * Register an already instantiated OSD plugin object.
         *
         * This is necessary for statically linked plugins. You have to call
         * this method before loadOSDPlugins() otherwise your static OSD plugins
         * won't be loaded.
         *
         * @param	name	plugin name as used in plugindata
         * @param	plugin	instantiated plugin object
         */
        void registerStaticOSDPlugin(string, IMMSOSDPlugin*);

        /**
         * Register an already instantiated central plugin object.
         *
         * This is necessary for statically linked plugins. You have to call
         * this method before loadCentralPlugins() otherwise your static central plugins
         * won't be loaded.
         *
         * @param	name	plugin name as used in plugindata
         * @param	plugin	instantiated plugin object
         */
        void registerStaticCentralPlugin(string, IMMSCentralPlugin*);

        /**
         * Register an already instantiated import plugin object.
         *
         * This is necessary for statically linked plugins. You have to call
         * this method before loadImportPlugins() otherwise your static import plugins
         * won't be loaded.
         *
         * @param	name	plugin name as used in plugindata
         * @param	plugin	instantiated plugin object
         */
        void registerStaticImportPlugin(string, IMMSImportPlugin*);

        /**
         * Register an already instantiated backend plugin object.
         *
         * This is necessary for statically linked plugins. You have to call
         * this method before loadBackendPlugins() otherwise your static backend plugins
         * won't be loaded.
         *
         * @param	name	plugin name as used in plugindata
         * @param	plugin	instantiated plugin object
         */
        void registerStaticBackendPlugin(string, IMMSBackendPlugin*);

        void loadOSDPlugins();
        void loadCentralPlugins();
        void loadImportPlugins();
        void loadBackendPlugins();

        void initializeOSDPlugins();
        void initializeCentralPlugins();
        void initializeImportPlugins();
        void initializeBackendPlugins();

        void setActiceOSDPlugin(MMSPluginData *plugin);
        MMSPluginData *getActiveOSDPlugin();

        void setActiceCentralPlugin(MMSPluginData *plugin);
        MMSPluginData *getActiveCentralPlugin();

        void setSwitcher(IMMSSwitcher *switcher);

        vector<MMSOSDPluginHandler *> getOSDPluginHandlers(vector<MMSPluginData *> data);
        MMSOSDPluginHandler          *getOSDPluginHandler(int pluginid);

        vector<MMSCentralPluginHandler *> getCentralPluginHandlers(vector<MMSPluginData *> data);
        MMSCentralPluginHandler          *getCentralPluginHandler(int pluginid);

        vector<MMSImportPluginHandler *> getImportPluginHandlers(vector<MMSPluginData *> data);
        MMSImportPluginHandler          *getImportPluginHandler(int pluginid);

        vector<MMSBackendPluginHandler *> getBackendPluginHandlers(vector<MMSPluginData *> data);
        MMSBackendPluginHandler          *getBackendPluginHandler(int pluginid);
};
#endif /*MMSPLUGINMANAGER_H_*/
