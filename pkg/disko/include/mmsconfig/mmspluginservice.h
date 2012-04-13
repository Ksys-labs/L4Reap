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

#ifndef MMSPLUGINSERVICE_H_
#define MMSPLUGINSERVICE_H_

#include "mmsconfig/mmsplugindata.h"
#include "mmsconfig/mmspropertydata.h"
#include "mmsconfig/mmsplugintypedata.h"
#include "mmstools/interfaces/immsdb.h"

typedef std::map<string, MMSPropertyData *> MMSPROPERTYDATA_MAP;

class MMSPluginService {
    private:
        IMMSDB 		*dbconn;

		const void setPluginProperties(MMSPluginData *plugin) const;
		const void setPluginProperties(vector<MMSPluginData*> &pluginList) const;

	public:
        MMSPluginService (DataSource *datasource);
        virtual ~MMSPluginService();

        void setDataSource(DataSource *datasource);
        DataSource *getDataSource();

        void setPlugin(MMSPluginData *data);
        void setPlugin(vector<MMSPluginData *> dataList);

        MMSPluginData *getPluginByID(int);
        MMSPluginData *getPluginByName(string name);
        vector<MMSPluginData*> getAllPlugins(const bool inactiveToo = false);
        vector<MMSPluginData*> getOSDPlugins(const bool inactiveToo = false);
        vector<MMSPluginData*> getCentralPlugins(const bool inactiveToo = false);
        vector<MMSPluginData*> getImportPlugins(const bool inactiveToo = false);
        vector<MMSPluginData*> getBackendPlugins(const bool inactiveToo = false);
        vector<MMSPluginData*> getPluginsByCategory(MMSPluginCategoryData *category, const bool inactiveToo = false);
        vector<MMSPluginData*> getPluginsByType(MMSPluginTypeData *type, const bool inactiveToo = false);
        MMSPluginCategoryData* getPluginCategoryByName(string name);
        vector<MMSPluginCategoryData*> getPluginCategories();
        MMSPluginTypeData*  getPluginTypeByName(string name);
        void getSystemProperties(std::map<string,MMSPropertyData*> &result);
};

#endif /*MMSPLUGINSERVICE_H_*/
